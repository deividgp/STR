/*********************************************************************/
/* Sistemes de temps real						*/
/* Fitxer: periodic-3.c						*/
/* Exemple sobre POSIX							*/
/* Cadena de complilació: gcc -Wall <file.c> -lrt -o <file>		*/
/* David Gamez (dgamezalari@lear.com)					*/
/*********************************************************************/

#include <sys/time.h>
#include <signal.h>
#include <time.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>


static sigset_t sigset;
static void wait_next_activation(void)
{
    int dummy;

    sigwait(&sigset, &dummy);    /* suspenem l'execució fins que rebem un 'set' de la senyal */  
}
int start_periodic_timer(uint64_t offs, int period)
{
    struct itimerspec t;
    struct sigevent sigev;
    timer_t timer;
    const int signal = SIGALRM;
    int res;

    t.it_value.tv_sec = offs / 1000000;
    t.it_value.tv_nsec = (offs % 1000000) * 1000;
    t.it_interval.tv_sec = period / 1000000;
    t.it_interval.tv_nsec = (period % 1000000) * 1000;

    sigemptyset(&sigset);       /* Inicia la senyal del conjunt  'sigset' */
    sigaddset(&sigset, signal);	
    sigprocmask(SIG_BLOCK, &sigset, NULL);

    memset(&sigev, 0, sizeof(struct sigevent));
    sigev.sigev_notify = SIGEV_SIGNAL;
    sigev.sigev_signo = signal;
    res = timer_create(CLOCK_MONOTONIC, &sigev, &timer);
    if (res < 0) {
        perror("Timer Create");

	exit(-1);
    }
    return timer_settime(timer, 0 /*TIMER_ABSTIME*/, &t, NULL);
}

static void job_body(void)
{
    static int cnt;
    static uint64_t start;
    uint64_t t;
    struct timeval tv;

    if (start == 0) {
        gettimeofday(&tv, NULL);
	start = tv.tv_sec * 1000ULL + tv.tv_usec / 1000ULL;
    }
        
    gettimeofday(&tv, NULL);
    t = tv.tv_sec * 1000ULL + tv.tv_usec / 1000ULL;
    if (cnt && (cnt % 100) == 0) {
        printf("Avg time: %f\n", (double)(t - start) / (double)cnt);
    }
    cnt++;
}

int main(int argc, char *argv[])
{
    int res;

    res = start_periodic_timer(2000000, 5000);
    if (res < 0) {
        perror("Start Periodic Timer");

        return -1;
    }

    while(1) {
        wait_next_activation();
        job_body();
    }

    return 0;
}

