#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

struct periodic_task {
  struct timespec r;
  int period;
};

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

void wait_next_activation(struct periodic_task *t)
{
    clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &t->r, NULL);
    timespec_add_us(&t->r, t->period);
}

struct periodic_task *start_periodic_timer(uint64_t offs, int t)
{
    struct periodic_task *p;

    p = malloc(sizeof(struct periodic_task));
    if (p == NULL) {
        return NULL;
    }

    clock_gettime(CLOCK_REALTIME, &p->r);
    timespec_add_us(&p->r, offs);
    p->period = t;

    return p;
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
  printf("\tT: %Lu\n", t - previous);
  previous = t;
}

int main(int argc, char *argv[])
{
    int res, dummy;
    struct periodic_task *t;

    res = fork();
    if (res ==0) {
      t = start_periodic_timer(2000000, 50000);
      while (1) {
        wait_next_activation(t);
        task1();
      }
    }

    res = fork();
    if (res == 0) {
      t = start_periodic_timer(2000000, 100000);
      while (1) {
        wait_next_activation(t);
        task2();
      }
    }

    res = fork();
    if (res == 0) {
      t = start_periodic_timer(2000000, 150000);
      while (1) {
        wait_next_activation(t);
        task3();
      }
    }

    wait(&dummy);
    wait(&dummy);
    wait(&dummy);

    return 0;
}