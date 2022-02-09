/*********************************************************************/
/* Sistemes de temps real						*/
/* Fitxer: periodic-4.c						*/
/* Exemple sobre POSIX							*/
/* Cadena de complilació: gcc -Wall <file.c> -lrt -o <file>		*/
/* David Gamez (dgamezalari@lear.com)					*/
/*********************************************************************/
#include <sys/time.h>
#include <time.h>
#include <stdint.h>
#include <stdio.h>

static struct timespec r;
static int period;

#define NSEC_PER_SEC 1000000000ULL
static inline void timespec_add_us(struct timespec *t, uint64_t d)
{
    d *= 1000;
    d += t->tv_nsec;
    while (d >= NSEC_PER_SEC) {
        d -= NSEC_PER_SEC;
	t->tv_sec += 1;
    }
    t->tv_nsec = d;
}

static void wait_next_activation(void)
{
    clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &r, NULL);
    timespec_add_us(&r, period);
}
int start_periodic_timer(uint64_t offs, int t)
{
    clock_gettime(CLOCK_REALTIME, &r);
    timespec_add_us(&r, offs);
    period = t;

    return 0;
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
