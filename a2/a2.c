#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "a2_helper.h"

sem_t sem;
const char *program_name;

void P(sem_t *sem)
{
    sem_wait(sem);
}

void V(sem_t *sem)
{
    sem_post(sem);
}

void thread_function(void *arg)
{
    int th_id = arg;

    P(&sem);

    if (th_id == 3)
    {
        info(BEGIN, 3, 3);

        info(BEGIN, 3, 2);

        info(END, 3, 2);

        info(END, 3, 3);
    }
    else
    {
        info(BEGIN, 3, th_id);

        info(END, 3, th_id);
    }

    V(&sem);
}

int main()
{
    pthread_t t[1000];

    int n, p2, p3, p4, p5, p6, p7, p8, p9;

    long long i;
    init();

    info(BEGIN, 1, 0);
    p2 = fork();

    if (p2 == 0)
    {
        info(BEGIN, 2, 0);

        p3 = fork();

        if (p3 == 0) // p2

        {
            info(BEGIN, 3, 0);

            if (sem_init(&sem, n, 0) < 0)
            {
                perror("Error creating the semaphore");
                exit(2);
            }

            for (i = 1; i <= 4; i++)
                if (pthread_create(&t[i], NULL, (void *(*)(void *))thread_function, (void *)i) != 0)
                {
                    perror("Cannot create threads");
                    exit(1);
                }

            for (i = 1; i <= 4; i++)
                pthread_join(t[i], NULL);

            p4 = fork();

            if (p4 == 0) // p3
            {
                info(BEGIN, 4, 0);
                p5 = fork();

                if (p5 == 0) // p4
                {
                    info(BEGIN, 5, 0);
                    p6 = fork();

                    if (p6 == 0) // p5
                    {
                        info(BEGIN, 6, 0);
                        waitpid(p6, NULL, 0);

                        info(END, 6, 0);
                        return;
                    }
                    else if (p6 > 0)
                    {
                        waitpid(p6, NULL, 0);

                        p9 = fork();

                        if (p9 == 0) // p9
                        {

                            info(BEGIN, 9, 0);

                            info(END, 9, 0);
                            return;
                        }
                        waitpid(p9, NULL, 0);
                    }

                    waitpid(p5, NULL, 0);

                    info(END, 5, 0);
                    return;
                }
                else if (p5 > 0)
                {
                    p7 = fork();

                    if (p7 == 0) // p7
                    {

                        info(BEGIN, 7, 0);

                        info(END, 7, 0);
                        return;
                    }
                    waitpid(p7, NULL, 0);
                }

                waitpid(p4, NULL, 0);

                info(END, 4, 0);
                return;
            }
            waitpid(p3, NULL, 0);

            info(END, 3, 0);
            return;
        }

        waitpid(p2, NULL, 0);

        info(END, 2, 0);
        return;
    }
    else if (p2 > 0)
    {
        p8 = fork();
        if (p8 == 0)
        {

            info(BEGIN, 8, 0);
            waitpid(p8, NULL, 0);

            info(END, 8, 0);
            return;
        }
    }
    waitpid(p8, NULL, 0);
    waitpid(p2, NULL, 0);

    info(END, 1, 0);
    return 0;
}
