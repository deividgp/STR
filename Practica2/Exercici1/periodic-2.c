/*********************************************************************/
/* Sistemes de temps real						*/
/* Fitxer: periodic-2.c						*/
/* Exemple sobre POSIX							*/
/* Cadena de complilació: gcc <fitxer.c> -o fitxer			*/
/* David Gamez (dgamezalari@lear.com)					*/
/*********************************************************************/

#include <sys/time.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>

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
