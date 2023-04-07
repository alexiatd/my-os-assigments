#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_PATH 400

struct section_header
{
    char section_name[18];
    int sect_type;
    int sect_offset;
    int sect_size;
};

const char *program_name;

void parse_path(char *filepath)
{
    int fd = open(filepath, O_RDONLY);
    struct section_header section_headers;
    // lseek(fd, 0, SEEK_SET);

    char magic[2];
    int version = 0, nr_section = 0, header_size = 0;

    if (fd < 0)
    {
        return;
    }

    read(fd, &magic, 1);
    magic[1] = '\0';

    if (strcmp(magic, "W") != 0)
    {
        printf("ERROR\nwrong magic");
        // close(fd);

        return;
    }

    read(fd, &header_size, 2);

    read(fd, &version, 2);

    if (version < 74 || version > 103)
    {
        printf("ERROR\nwrong version");
        // close(fd);

        return;
    }

    read(fd, &nr_section, 1);
    // printf("%d\n",nr_section);
    if (nr_section < 4 || nr_section > 10)
    {
        printf("ERROR\nwrong sect_nr");
        // close(fd);

        return;
    }

    for (int i = 0; i < nr_section; i++)
    {
        section_headers.sect_type = 0;
        section_headers.sect_offset = 0;
        section_headers.sect_size = 0;
        read(fd, &section_headers.section_name, 17);

        read(fd, &section_headers.sect_type, 4);

        if (section_headers.sect_type != 46 && section_headers.sect_type != 54 && section_headers.sect_type != 32 && section_headers.sect_type != 89 && section_headers.sect_type != 56)
        {
            printf("ERROR\nwrong sect_types");
            // close(fd);

            return;
        }

        read(fd, &section_headers.sect_offset, 4);

        read(fd, &section_headers.sect_size, 4);
    }

    int offset = 1 + 2 + 2 + 1;

    lseek(fd, offset, SEEK_SET);

    // lseek(fd,0,SEEK_SET);

    printf("SUCCESS\n");
    // read(fd, &magic, 1);
    // read(fd, &header_size, 2);
    // read(fd, &version, 2);

    printf("version=%d\n", version);
    // read(fd, &nr_section, 1);

    printf("nr_sections=%d\n", nr_section);

    for (int i = 1; i <= nr_section; i++)
    {
        section_headers.sect_type = 0;
        section_headers.sect_offset = 0;
        section_headers.sect_size = 0;

        printf("section%d:", i);
        read(fd, &section_headers.section_name, 17);
        section_headers.section_name[17] = '\0';

        printf(" %s", section_headers.section_name);

        read(fd, &section_headers.sect_type, 4);
        printf(" %d", section_headers.sect_type);

        read(fd, &section_headers.sect_offset, 4);

        read(fd, &section_headers.sect_size, 4);
        printf(" %d", section_headers.sect_size);

        printf("\n");
    }

    close(fd);
}

void extract(char *directory, int section, int line)
{

    int fd = open(directory, O_RDONLY);
    lseek(fd, 0, SEEK_SET);
    struct section_header section_headers;
    char magic[2];
    int version = 0, nr_section = 0, header_size = 0;

    if (fd < 0)
    {
        close(fd);
        printf("ERROR\ninvalid file\n");
        return;
    }

    read(fd, &magic, 1);
    magic[1] = '\0';

    if (strcmp(magic, "W") != 0)
    {
        close(fd);

        return;
    }

    read(fd, &header_size, 2);

    read(fd, &version, 2);
    // printf("version=%d\n", version);

    if (version < 74 || version > 103)
        return;

    read(fd, &nr_section, 1);
    if (nr_section < 4 || nr_section > 10)
    {
        close(fd);

        return;
    }

    if (section > nr_section)
    {

        printf("ERROR\ninvalid section\n");
        close(fd);

        return;
    }

    for (int i = 1; i <= nr_section; i++)
    {

        section_headers.sect_type = 0;
        section_headers.sect_offset = 0;
        section_headers.sect_size = 0;
        read(fd, &section_headers.section_name, 17);
        section_headers.section_name[17] = '\0';

        read(fd, &section_headers.sect_type, 4);

        if (section_headers.sect_type != 46 && section_headers.sect_type != 54 && section_headers.sect_type != 32 && section_headers.sect_type != 89 && section_headers.sect_type != 56)
        {
            close(fd);

            return;
        }

        read(fd, &section_headers.sect_offset, 4);

        read(fd, &section_headers.sect_size, 4);
    }


    printf("SUCCESS\n");

    int offset = 1 + 2 + 2 + 1;

    lseek(fd, offset, SEEK_SET);

    for (int i = 1; i <= section; i++)
    {

        section_headers.sect_type = 0;
        section_headers.sect_offset = 0;
        section_headers.sect_size = 0;
        read(fd, &section_headers.section_name, 17);
        section_headers.section_name[17] = '\0';

        read(fd, &section_headers.sect_type, 4);

        read(fd, &section_headers.sect_offset, 4);

        read(fd, &section_headers.sect_size, 4);
    }

    char buff[2000000];
    lseek(fd, section_headers.sect_offset, SEEK_SET);

    read(fd, &buff, section_headers.sect_size);

    int nr = 1, k = 0;
    for (int i = section_headers.sect_size - 1; i >= 0; i--)
    {
        if (line == nr)
        {
            printf("%c", buff[i]);
            k = 1;
        }
        if (buff[i] == 0x0A && buff[i - 1] == 0x0D)
            nr++;


        if (nr > line && k == 0)
        {
            printf("ERROR\ninvalid line\n");
            close(fd);
            return;
        }
    }
    if (nr < line)
    {
        printf("ERROR\ninvalid line\n");
        close(fd);
        return;
    }
    //free(buff);
    close(fd);
}

void find(char *directory, int c)
{
    DIR *dir;
    struct dirent *dirEntry;
    struct stat inode;
    char *final = malloc(MAX_PATH * sizeof(char));

    dir = opendir(directory);
    if (dir == NULL)
    {
        printf("ERROR\ninvalid directory path\n");
        free(final);
        return;
    }
    if (c == 1)
    {
        c = 0;
        printf("SUCCESS\n");
    }
    while ((dirEntry = readdir(dir)) != 0)
    {
        if (strcmp(dirEntry->d_name, ".") != 0 && strcmp(dirEntry->d_name, "..") != 0)
        {
            snprintf(final, MAX_PATH, "%s/%s", directory, dirEntry->d_name);

            stat(final, &inode);
            if (S_ISREG(inode.st_mode))
            {

                int fd = open(final, O_RDONLY);
                lseek(fd, 0, SEEK_SET);
                struct section_header section_headers;
                char magic[2];
                int version = 0, nr_section = 0, header_size = 0, k = 0;

                if (fd < 0)
                {
                    free(final);
                    close(fd);
                    return;
                }

                read(fd, &magic, 1);
                magic[1] = '\0';

                if (strcmp(magic, "W") != 0)
                {
                    free(final);
                    close(fd);

                    return;
                }

                read(fd, &header_size, 2);

                read(fd, &version, 2);
                // printf("version=%d\n", version);

                if (version < 74 || version > 103)
                {
                    close(fd);
                    free(final);
                    return;
                }

                read(fd, &nr_section, 1);
                if (nr_section < 4 || nr_section > 10)
                {
                    free(final);
                    close(fd);
                    return;
                }
                // printf("nr_sections=%d\n", nr_section);

                for (int i = 0; i < nr_section && k == 0; i++)
                {
                    section_headers.sect_type = 0;
                    section_headers.sect_offset = 0;
                    section_headers.sect_size = 0;
                    read(fd, &section_headers.section_name, 17);
                    section_headers.section_name[17] = '\0';

                    read(fd, &section_headers.sect_type, 4);

                    if (section_headers.sect_type == 56)
                        k++;

                    read(fd, &section_headers.sect_offset, 4);

                    read(fd, &section_headers.sect_size, 4);
                }

                close(fd);
                if (k != 0)
                    printf("%s\n", final);
            }
            if (S_ISDIR(inode.st_mode))
            {
                // printf("DIR \n");
                find(final, 0);
            }
        }
    }
    free(final);
}

int search_directory(char *directory, int option, int value, int recursiv, int k)
{
    DIR *dir;
    struct dirent *dirEntry;
    struct stat inode;
    char *final = malloc(MAX_PATH * sizeof(char));
    int size;

    dir = opendir(directory);
    if (dir == NULL)
    {
        // printf("ERROR\ninvalid directory path\n");
        return -1;
    }
    if (k == 1)
    {
        k = 0;
        printf("SUCCESS\n");
    }

    while ((dirEntry = readdir(dir)) != 0)
    {

        if (strcmp(dirEntry->d_name, ".") != 0 && strcmp(dirEntry->d_name, "..") != 0)
        {
            snprintf(final, MAX_PATH, "%s/%s", directory, dirEntry->d_name);
            stat(final, &inode);
            if (S_ISDIR(inode.st_mode) && recursiv)
                search_directory(final, option, value, recursiv, 0);

            if (option == 1)
            {
                size = inode.st_size;

                if (size > value && S_ISREG(inode.st_mode))
                {

                    printf("%s\n", final);
                }
            }
            else if (option == 2)
            {
                if (S_ISREG(inode.st_mode) || S_ISDIR(inode.st_mode))
                    if ((inode.st_mode & S_IXUSR))
                        printf("%s\n", final);
            }
            else if (option == 0)
                printf("%s\n", final);
        }
    }

    closedir(dir);
    free(final);
    return 1;
}

int main(int argc, char **argv)
{
    int test, value, c = 0, recursive = 0;
    program_name = argv[0];
    char *dir;

    if (argc >= 2)
    {
        if (strcmp(argv[1], "variant") == 0)
        {
            printf("99110\n");
        }
        else if (strcmp(argv[1], "list") == 0)
        {
            for (int i = 2; i < argc; i++)
            {

                if (strcmp(argv[i], "recursive") == 0)
                    recursive = 1;
                if (strncmp(argv[i], "size_greater=", 12) == 0)
                {
                    c = 1;

                    value = atoi(strtok(argv[i] + 13, "="));
                }
                else if (strcmp(argv[i], "has_perm_execute") == 0)
                {
                    c = 2;
                    // printf("DA \n");
                }
                else
                {
                    if (strncmp(argv[i], "path=", 4) == 0)
                        dir = strtok(argv[i] + 5, "=");
                }
            }
            test = search_directory(dir, c, value, recursive, 1);
            if (test)
            {
                test = 1;
            }
        }
        else if (strcmp(argv[1], "parse") == 0)
        {
            if (strncmp(argv[2], "path=", 4) == 0)
            {
                dir = strtok(argv[2] + 5, "=");

                parse_path(dir);
            }
        }
        else if (strcmp(argv[1], "findall") == 0)
        {
            if (strncmp(argv[2], "path=", 4) == 0)
            {
                dir = strtok(argv[2] + 5, "=");
                find(dir, 1);
            }
        }
        else if (strcmp(argv[1], "extract") == 0)
        {
            int section, line;
            if (strncmp(argv[2], "path=", 4) == 0)
            {
                dir = strtok(argv[2] + 5, "=");
                if (strncmp(argv[3], "section=", 7) == 0)
                    section = atoi(strtok(argv[3] + 8, "="));
                if (strncmp(argv[4], "line=", 4) == 0)
                    line = atoi(strtok(argv[4] + 5, "="));
                //  printf("%d\n",line);

                extract(dir, section, line);
            }
        }
    }

    return 0;
}