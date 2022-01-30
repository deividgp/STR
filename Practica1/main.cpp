#include "mbed.h"
#include "Grove_LCD_RGB_Backlight.h"
#include <cstdio>

AnalogIn light(A0);
AnalogIn pot(A1);
AnalogIn button(A2);
PwmOut led(D6);
PwmOut buzzer(D3);
Grove_LCD_RGB_Backlight display(D14, D15);
Mutex stdio_mutex;
Semaphore one_slot(1);

float lightVal;
float ledVal;
float avgValue = 0;
bool avgThread = false;

void buzzerEmitter(){
    uint64_t now = Kernel::get_ms_count();
    uint64_t later;
    do{
        buzzer.write(1);
        later = Kernel::get_ms_count();
    }while(later - now < 500);
    buzzer.write(0);
}

void buzzerThread(){
    Thread *threadBuzzer = new Thread(osPriorityNormal, 1024, NULL, NULL);
    threadBuzzer->start(callback(buzzerEmitter));
    threadBuzzer->join();
    delete threadBuzzer;
}

void lightReader(){
    float lightValAux = light.read();
    if(lightValAux < 0){
        buzzerThread();
        return;
    }
    lightVal = lightValAux;
    ledVal = 1.0-lightVal;
    led.write(ledVal);
}

void potReader(){
    float potVal = pot.read();
    if(potVal < 0){
        buzzerThread();
    }else if(potVal > ledVal){
        ledVal = potVal;
        led.write(ledVal);
    }
}

void valuesDisplay(){
    one_slot.acquire();
    char buffer [100];

    if(avgValue == 0){
        sprintf(buffer, "Llum:%.2f | Led:%.2f", lightVal*100.0, ledVal*100.0);
        display.print(buffer);
    }else{
        sprintf(buffer, "Mitjana: %.2f", avgValue);
        uint64_t now = Kernel::get_ms_count();
        uint64_t later;
        display.clear();
        do{
            display.locate(0, 0);
            display.print(buffer);
            later = Kernel::get_ms_count();
        }while(later - now < 5000);
    }
    one_slot.release();
}

void displayThread(){
    Thread *threadDisplay = new Thread(osPriorityHigh, 1024, NULL, NULL);
    threadDisplay->start(callback(valuesDisplay));
    threadDisplay->join();
    delete threadDisplay;
}

void buttonPress(){
    stdio_mutex.lock();
    avgThread = true;
    float numberLightValues = 0;
    float totalLightValue = 0;
    uint64_t now = Kernel::get_ms_count();
    uint64_t later;
    do{
        numberLightValues++;
        totalLightValue = totalLightValue + light.read();
        later = Kernel::get_ms_count();
    }while(later - now < 10000);
    avgValue = totalLightValue/numberLightValues;
    
    stdio_mutex.unlock();
    displayThread();
    avgThread = false;
    avgValue = 0;
}

void buttonReader(){
    if(avgThread == true){
        return;
    }

    float buttonVal = button.read();

    if(buttonVal < 0){
        buzzerThread();
    }else if(buttonVal >= 0.75){
        Thread *threadAvg = new Thread(osPriorityNormal, 1024, NULL, NULL);
        threadAvg->start(callback(buttonPress));
        threadAvg->join();
        delete threadAvg;
    }
}

int main()
{
    while (true) {
        Thread *threadLight = new Thread(osPriorityHigh, 1024, NULL, NULL);
        threadLight->start(callback(lightReader));
        Thread *threadPot = new Thread(osPriorityHigh, 1024, NULL, NULL);
        threadPot->start(callback(potReader));
        Thread *threadButton = new Thread(osPriorityHigh, 1024, NULL, NULL);
        threadButton->start(callback(buttonReader));

        threadLight->join();
        delete threadLight;
        threadPot->join();
        delete threadPot;
        threadButton->join();
        delete threadButton;
        printf("Intensitat de la llum: %f | Intensitat del led: %f\n", lightVal*100.0, ledVal*100.0);
        
        displayThread();
    }
}