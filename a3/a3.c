#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/mman.h>

const char *program_name;

#define REQ_PIPE_NAME "REQ_PIPE_99110"
#define RESP_PIPE_NAME "RESP_PIPE_99110"

struct section_header
{
    char section_name[18];
    int sect_type;
    int sect_offset;
    int sect_size;
};

int main(int argc, char **argv)
{
    int n, fd[2], smh_fd;
    int req,res;
    // ssize_t byts;
    program_name = argv[0];
    // char *buf[10];
    // char mess[250];
    // void *shm_ptr;

    if(mkfifo(RESP_PIPE_NAME, 0600)){
        printf("ERROR \ncannot create the response pipe");
        exit(1);
    }


    req = open(REQ_PIPE_NAME, O_RDONLY);

    if (req ==-1)
    {
        printf("ERROR \ncannot open the request pipe");
        exit(2);
    }

    res = open(RESP_PIPE_NAME, O_WRONLY);

    if (res ==-1)
    {
        printf("ERROR \ncannot open the response pipe");
        exit(3);
    }
    char mess[5]="HELLO$";

    if (write(res, mess, strlen(mess)) == -1)
    {
        printf("ERROR write response on RESP_PIPE_99110");
        exit(4);
    }
    printf("SUCCESS");
    /*
        for (;;)
        {
            byts = read(fd[0], mess, sizeof(mess));

            if (strcmp(byts, "PING$") == 0)
            {
                write(fd[0], "PING$PONG$99110", byts);
            }
            else if (strcmp(byts, "CREATE_SHM$3262703$") == 0)
            {
                smh_fd = shm_open("/iEPooM", O_CREAT | O_RDWR, 0664);
                //shm_ptr = mmap(NULL, 3000000, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
                if (smh_fd < 0 && ftruncate(smh_fd, 3000000) < 0 )
                {
                    write(smh_fd, "CREATE_SHM$", byts);
                    write(smh_fd, "ERROR$", byts);
                    exit(2);
                }
                else
                {
                    write(smh_fd, "CREATE_SHM$", byts);
                    write(smh_fd, "SUCCESS$", byts);
                    exit(3);
                }
            }
        }
    */
    close(req);
    close(res);
    unlink(res);

    return 0;
}