#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <semaphore.h>
#include <pthread.h>
#include <string.h>
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
    int req, res;
    ssize_t byts;
    program_name = argv[0];
    char *buf, *buf1;
    char text[4],shm_ptr;

    if (mkfifo(RESP_PIPE_NAME, 0666))
    {
        printf("ERROR \ncannot create the response pipe");
        exit(1);
    }

    req = open(REQ_PIPE_NAME, O_RDONLY);

    if (req == -1)
    {
        printf("ERROR \ncannot open the request pipe");
        exit(2);
    }

    res = open(RESP_PIPE_NAME, O_WRONLY);

    if (res == -1)
    {
        printf("ERROR \ncannot open the response pipe");
        exit(3);
    }
    char mess[6] = "HELLO$";

    if (write(res, mess, strlen(mess)) == -1)
    {
        printf("ERROR write response on RESP_PIPE_99110");
        exit(4);
    }
    printf("SUCCESS");

    int size = 0, i=0;
    char nr;
    unsigned int n = 99110;

    while (1)
    {
        //  while(read(req, buf, 1)){
        //     if(strcmp(buf,'$')==0)

        byts = read(req, buf, 250);

/*
        buf1=strtok(buf,"$");
        i=0;
        while(buf1!=0){
            strcpy(text[i],buf1);
            i++;
            buf1=strtok(NULL,"$");
        }
*/
        //  buf[6] = '\0';

        if (strncmp(buf, "PING$", 5) == 0)
        {
            // printf("gfgf");
            write(res, "PING$", 5);
            write(res, "PONG$", 5);

            // sprintf(nr, "%x", n);
            write(res, &n, sizeof(unsigned int));
        }
        else if (strncmp(buf, "CREATE_SHM$",11) == 0)
        {
            
            //if (strncmp(buf, "3262703", 7) == 0)
               int smh_fd = shm_open("/iEPooM", O_CREAT | O_RDWR, 0664);
            shm_ptr = mmap(NULL, 3262703, PROT_READ | PROT_WRITE, MAP_SHARED, smh_fd, 0);
            if (smh_fd < 0 && ftruncate(smh_fd, 3262703) < 0)
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
        else if (strncmp(buf, "EXIT$", 5) == 0)
        {
            exit(1);
        }
    }

    close(req);
    close(res);
    unlink(RESP_PIPE_NAME);

    return 0;
}