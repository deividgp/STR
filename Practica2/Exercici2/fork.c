#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>

static int counter;

static void body(const char *s)
{
    int i;

    for (i = 0; i < 5; i++) {
        printf("%s: %d\n", s, counter++);
    }
}

int main(int argc, char *argv[])
{
    pid_t child;
    int res;

    counter = 10;

    child = fork();
    if (child < 0) {
        perror("Fork");

        return -1;
    }

    if (child == 0) {
        body("Child");

	return 0;
    }
    body("Father");

    wait(&res);

    return 0;
}
