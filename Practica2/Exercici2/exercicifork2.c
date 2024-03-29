#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <stdint.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <ctype.h>

static sigset_t sigset;
static void wait_next_activation(void)
{
    int dummy;

    sigwait(&sigset, &dummy);
}
int start_periodic_timer(uint64_t offs, int period)
{
    struct itimerval t;

    t.it_value.tv_sec = offs / 1000000;
    t.it_value.tv_usec = offs % 1000000;
    t.it_interval.tv_sec = period / 1000000;
    t.it_interval.tv_usec = period % 1000000;

    sigemptyset(&sigset);
    sigaddset(&sigset, SIGALRM);
    sigprocmask(SIG_BLOCK, &sigset, NULL);

    return setitimer(ITIMER_REAL, &t, NULL);
}

void task1(void)
{
  int i,j;
 
  for (i=0; i<3; i++) {
    for (j=0; j<1000; j++) ;
    printf("1");
    fflush(stdout);
  }
}

void task2(void)
{
  int i,j;

  for (i=0; i<5; i++) {
    for (j=0; j<10000; j++) ;
    printf("2");
    fflush(stdout);
  }
}

void task3(void)
{
  static uint64_t previous;
  uint64_t t;
  struct timeval tv;

  if (previous == 0) {
    gettimeofday(&tv, NULL);
    previous = tv.tv_sec * 1000ULL + tv.tv_usec / 1000ULL;
  }

  gettimeofday(&tv, NULL);
  t = tv.tv_sec * 1000ULL + tv.tv_usec / 1000ULL;
  printf("\tT: %lu\n", t - previous);
  previous = t;
}

int main(int argc, char *argv[])
{
    int res;
    pid_t child;

    res = start_periodic_timer(1000000, 20000);
    if (res < 0) {
        perror("Start Periodic Timer");

        return -1;
    }

    int compt1=0, compt2=0, compt3 = 0;

    while(1) {
        wait_next_activation();

        if (compt1%3 == 0){
            child = fork();
        
            if (child < 0) {
                perror("Fork");
                return -1;
            }

            if (child == 0) {
                task1();
                return 0;
            }
            
            compt1=0;
        }
        
        if (compt2%4 == 0){
            child = fork();
        
            if (child < 0) {
                perror("Fork");
                return -1;
            }

            if (child == 0) {
                task2();
                return 0;
            }
            
            compt2=0;
        }

        if (compt3%6 == 0){
            child = fork();
        
            if (child < 0) {
                perror("Fork");
                return -1;
            }

            if (child == 0) {
                task3();
                return 0;
            }
            
            compt3=0;
        }
        compt1++;
        compt2++;
        compt3++;
    }

    return 0;
}