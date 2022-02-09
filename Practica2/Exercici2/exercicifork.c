#include <sys/time.h>
#include <time.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>

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

    res = start_periodic_timer(1000000, 50000);
    if (res < 0) {
        perror("Start Periodic Timer");

        return -1;
    }

    int contB=0, contC = 0;

    while(1) {
        wait_next_activation();

        child = fork();

        if (child < 0) {
            perror("Fork");
            return -1;
        }

        if (child == 0) {
            task1();
            return 0;
        }
        
        //Mod 2 pq se ejecuta cada 100ms
        if (contB%2 == 0){
            child = fork();
        
            if (child < 0) {
                perror("Fork");
                return -1;
            }

            if (child == 0) {
                task2();
                return 0;
            }
            
            contB=0;
        }
        //Mod 3 pq se ejecuta cada 150ms
        if (contC%3 == 0){
            child = fork();
        
            if (child < 0) {
                perror("Fork");
                return -1;
            }

            if (child == 0) {
                task3();
                return 0;
            }
            
            contC=0;
        }
        contB++;
        contC++;
    }

    return 0;
}