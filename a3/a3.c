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
    // printf("SUCCESS");

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
           // char * request;
            
            read(req, request_number_field,4);
            unsigned int number_shm = *(unsigned int*)request_number_field;

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
            free(request_number_field);
        }
        else if (strncmp(request_msg, "WRITE_TO_SHM", 12) == 0)
        {
           unsigned int offset, val;
        
            char * request_number_field=malloc(4);
            read(req, request_number_field,4);
            offset = *(unsigned int*)request_number_field;
            read(req, request_number_field,4);
            val = *(unsigned int*)request_number_field;

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
            free(request_number_field);
        }
        else if(strncmp(request_msg, "MAP_FILE", 8) == 0)
        {
            char * file_name = malloc(250);
            read(req,c,1);
            int i = 0;
            while(c[0] != '$')
            {
                file_name[i] = c[0];
                i++;
                read(req,c,1);
            }
            file_name[i] ='\0';

            write(res, "MAP_FILE$", 9);
            int fd = open(file_name,O_RDONLY);
            if(fd < 0)
            {
                write(res,"ERROR$",6);
            }
            else {
                file_size = lseek(fd,0,SEEK_END);
                //lseek(fd,0,SEEK_SET);
                mapped_file = (char*)mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
                if(mapped_file == MAP_FAILED)
                {
                    write(res,"ERROR$",6);
                }
                else {
                    write(res,"SUCCESS$",8);
                }

            }
            close(fd);
            
        }
        else if(strncmp(request_msg,"READ_FROM_FILE_OFFSET",21) == 0)
        {
            unsigned int offset, nr_bytes;
        
            char * request_number_field=malloc(4);
            read(req, request_number_field,4);
            offset = *(unsigned int*)request_number_field;
            read(req, request_number_field,4);
            nr_bytes = *(unsigned int*)request_number_field;

            write(res,"READ_FROM_FILE_OFFSET$",22);
            if(offset + nr_bytes >= file_size)
            {
               write(res,"ERROR$",6);
            }
            else if (mapped_file == NULL)
            {
                write(res,"ERROR$",6);
            }
            else if(smh == NULL)
            {
                write(res,"ERROR$",6);
            }
            else {
                int i = 0;
                while(i < nr_bytes )
                {
                    smh[i] = mapped_file[offset+i];
                    i++;
                }
                write(res,"SUCCESS$",8);

            }


        }
         else if(strncmp(request_msg,"READ_FROM_FILE_SECTION",22)==0)
        {
            unsigned int offset, nr_bytes, section_no;
        
            char * request_number_field=malloc(4);
            read(req, request_number_field,4);
            section_no = *(unsigned int*)request_number_field;

            read(req, request_number_field,4);
            offset = *(unsigned int*)request_number_field;

            read(req, request_number_field,4);
            nr_bytes = *(unsigned int*)request_number_field;

           //write(res,&section_no,4);
           // printf("%u %u %u %u\n",section_no,offset,nr_bytes,mapped_file[5]);
            
            write(res,"READ_FROM_FILE_SECTION$",23);
            //write(res,"ERROR$",6);
            //printf("%d",section_no <= mapped_file[5]);
            //char* new_s=malloc(2);
          //  new_s[0] = mapped_file[5];
            //new_s[1] ="$";
            //write(res,new_s,2);
            
            if(mapped_file == NULL)
            {
                  write(res,"ERROR$",6);
            }
            else if(smh == NULL)
            {
                  write(res,"ERROR$",6);
            }
            else if(section_no <= mapped_file[5])
            {  
                unsigned int sect_offset,sect_size;
                unsigned int section_add_offset =  6 + (section_no - 1) * 29 + 21;
                unsigned int section_add_size =  6 + (section_no - 1) * 29 + 25;
                sect_offset = *((unsigned int*)(mapped_file + section_add_offset));
                sect_size = *((unsigned int*)(mapped_file + section_add_size));

                if(offset + nr_bytes >= sect_size)
                {
                  write(res,"ERROR$",6);
                }   
                else {
                int i=0;
                while(i < nr_bytes)
                {
                    smh[i] = mapped_file[sect_offset + offset + i];
                    i++;
                }

                 write(res,"SUCCESS$",8);

                }
            }else {
                write(res,"ERROR$",6);
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
