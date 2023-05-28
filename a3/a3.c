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
    // ssize_t byts;
    program_name = argv[0];
    //char *buf = malloc(501);
    char * mapped_file;
    int file_size;
   

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

    unsigned int n = 99110;
    //int smh_fd;
    char* smh = NULL;
    char* request_msg=malloc(250);
    char* c=malloc(2);
    while (1)
    {
        int i=0;
        ///read(req, buf, 500);
        read(req,c,1);
        while(c[0] != '$')
        {
            request_msg[i] = c[0];
            i++;
            read(req,c,1);
        }
        request_msg[i] ='\0';
       
       // buf[strlen(buf)]='\0';
        if (strncmp(request_msg, "PING", 4) == 0)
        {
            write(res, "PING$", 5);
            write(res, "PONG$", 5);

            write(res, &n, sizeof(unsigned int));
        }
        else if (strncmp(request_msg, "CREATE_SHM", 10) == 0)
        {
            char * request_number_field=malloc(4);
            
            read(req, request_number_field,4);
            
            int smh_fd = shm_open("/iEPooM", O_CREAT | O_RDWR, 0664);
            int tr = ftruncate(smh_fd, number_shm);
            smh = (char*)mmap(NULL, number_shm, PROT_READ | PROT_WRITE, MAP_SHARED, smh_fd, 0);
            if (smh_fd < 0 && tr < 0)
            {
                write(smh_fd, "CREATE_SHM$", 11);
                write(smh_fd, "ERROR$", 7);
               
            }
            else
            {
                write(res, "CREATE_SHM$", 11);
                write(res, "SUCCESS$", 8);
            }
        }
        else if (strncmp(request_msg, "WRITE_TO_SHM", 12) == 0)
        {
           unsigned int offset, val;
        
            read(req, offset,4);
            read(req, val,4);
           
            unsigned int valueoff = offset + sizeof(val);
            if (offset >= 0 && valueoff < 3262703)
            {
                    memcpy(smh + offset, &val, sizeof(unsigned int));
                    write(res, "WRITE_TO_SHM$", 13);
                    write(res, "SUCCESS$", 8);
            }
            else
            {
                write(res, "WRITE_TO_SHM$", 13);
                write(res, "ERROR$", 7);
                // exit(5);
            }
        }
        
        else if (strncmp(request_msg, "EXIT", 4) == 0)
        {
            close(req);
            close(res);
            unlink(RESP_PIPE_NAME);
            exit(1);
        }
    }

    return 0;
}
