// Lab 2 viktar.c
// Alan Shirk - alans

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <zlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

#include "viktar.h"

#define BUFFER_SIZE 100

uint32_t calculate_crc32(const void *buf, size_t len);

int 
main(int argc, char *argv[])
{
    int verbose = FALSE;
    viktar_action_t action = ACTION_NONE;
    char *filename = NULL;
    int iarch = STDIN_FILENO;
    char buf[BUFFER_SIZE] = {'\0'};
    unsigned char crc_buf[BUFFER_SIZE] = {'\0'};
    viktar_header_t header;
    struct passwd *pw;
    struct group *gr;
    struct tm *mtime;
    struct tm *atime;
    char mtime_date[40];
    char atime_date[40];
    char mode_str[11];
    uint32_t crc_header;
    uint32_t crc_data;
    ssize_t bytes_read;
    size_t data_remaining;
    size_t to_read;

    // Processing command line options with getopt
    {
        int opt = -1;

        while ((opt = getopt(argc, argv, OPTIONS)) != -1) {
            switch (opt) {
            case 'h': // Help info
                printf("help text\n");
                printf("\t%s\n", argv[0]);
                printf("\tOptions: %s\n", OPTIONS);
                printf("\t\t-x\t\textract file/files from archive\n");
                printf("\t\t-c\t\tcreate an archive file\n");
                printf("\t\t-t\t\tdisplay a short table of contents of the archive file\n");
                printf("\t\t-T\t\tdisplay a long table of contents of the archive file\n");
                printf("\t\tOnly one of xctTV can be specified\n");
                printf("\t\t-f filename\tuse filename as the archive file\n");
                printf("\t\t-V\t\tvalidate the crc values in the viktar file\n");
                printf("\t\t-v\t\tgive verbose diagnostic messages\n");
                printf("\t\t-h\t\tdisplay this AMAZING help message\n");
                return EXIT_SUCCESS;
                break;
                
            case 'v': // Verbose mode
                verbose = TRUE;
                break;

            case 'x': // Extract members from viktar file
                action = ACTION_EXTRACT;
                break;

            case 'c': // Create viktar file
                action = ACTION_CREATE;
                break;

            case 't': // Short table of contents
                action = ACTION_TOC_SHORT;
                break;

            case 'T': // Long table of contents
                action = ACTION_TOC_LONG;
                break;

            case 'f': // Sets viktar input file
                filename = optarg;
                break;
            
            case 'V': // Validate archive member with CRC values
                action = ACTION_VALIDATE;
                break;

            default: // Invalid options
                printf("invalid option -- '%s'", optarg);
                printf("oopsie - unrecognized command line option \"(null)\"\n");
                printf("no action supplied\n");
                printf("exiting without doing ANYTHING...\n");
                return EXIT_FAILURE;
                break;
            }
        }
    }
    // Verbose output
    if (verbose == TRUE) {
        fprintf(stderr, "verbose enabled\n");
    }

    // Short table of contents
    if (action == ACTION_TOC_SHORT) {

        if (filename == NULL) {
            fprintf(stderr, "reading archive from stdin\n");
        }     
        else {
            // If filename is set with -f, assigning the file descriptor to iarch
            iarch = open(filename, O_RDONLY);
            fprintf(stderr, "reading archive file: \"%s\"\n", filename);
            // Reading the file
            read(iarch, buf, strlen(VIKTAR_TAG));
        }

        // Validates the tag
        if(strncmp(buf, VIKTAR_TAG, strlen(VIKTAR_TAG)) != 0) {
            // Not a valid viktar file
            fprintf(stderr, "not a viktar file: \"%s\"\n", filename != NULL ? filename : "stdin");
            exit(EXIT_FAILURE);
        }

        // Processing the archive file metadata
        printf("Contents of viktar file: \"%s\"\n", filename != NULL ? filename : "stdin");

        while (read(iarch, &header, sizeof(viktar_header_t)) > 0) {
            // Printing archive member name
            memset(buf, 0, 100);
            strncpy(buf, header.viktar_name, VIKTAR_MAX_FILE_NAME_LEN);
            printf("\tfile name: %s\n", buf);
            lseek(iarch, header.st_size + sizeof(viktar_footer_t), SEEK_CUR);
        }

        // Closing the file
        if (filename != NULL) {
            close(iarch);
        }

    }

    // Long table of contents
    if (action == ACTION_TOC_LONG) {
        if (filename == NULL) {
            fprintf(stderr, "reading archive from stdin\n");
        }     
        else {
            // If filename is set with -f, assigning the file descriptor to iarch
            iarch = open(filename, O_RDONLY);
            fprintf(stderr, "reading archive file: \"%s\"\n", filename);
            // Reading the file
            read(iarch, buf, strlen(VIKTAR_TAG));
        }

        // Validates the tag
        if(strncmp(buf, VIKTAR_TAG, strlen(VIKTAR_TAG)) != 0) {
            // Not a valid viktar file
            fprintf(stderr, "not a viktar file: \"%s\"\n", filename != NULL ? filename : "stdin");
            exit(EXIT_FAILURE);
        }

        // Processing the archive file metadata
        printf("Contents of viktar file: \"%s\"\n", filename != NULL ? filename : "stdin");

    while (read(iarch, &header, sizeof(viktar_header_t)) > 0) {
        memset(buf, 0, 100);
        strncpy(buf, header.viktar_name, VIKTAR_MAX_FILE_NAME_LEN);

        // Updates user and group information for each header
        pw = getpwuid(header.st_uid);
        gr = getgrgid(header.st_gid);
        mtime = localtime(&header.st_mtim.tv_sec);
        atime = localtime(&header.st_atim.tv_sec);
        strftime(mtime_date, sizeof(mtime_date), "%Y-%m-%d %H:%M:%S %Z", mtime);
        strftime(atime_date, sizeof(atime_date), "%Y-%m-%d %H:%M:%S %Z", atime);

        // Creates the mode string
        mode_str[0] = (S_ISDIR(header.st_mode)) ? 'd' : '-';
        mode_str[1] = (header.st_mode & S_IRUSR) ? 'r' : '-';
        mode_str[2] = (header.st_mode & S_IWUSR) ? 'w' : '-';
        mode_str[3] = (header.st_mode & S_IXUSR) ? 'x' : '-';
        mode_str[4] = (header.st_mode & S_IRGRP) ? 'r' : '-';
        mode_str[5] = (header.st_mode & S_IWGRP) ? 'w' : '-';
        mode_str[6] = (header.st_mode & S_IXGRP) ? 'x' : '-';
        mode_str[7] = (header.st_mode & S_IROTH) ? 'r' : '-';
        mode_str[8] = (header.st_mode & S_IWOTH) ? 'w' : '-';
        mode_str[9] = (header.st_mode & S_IXOTH) ? 'x' : '-';
        mode_str[10] = '\0';

        printf("\tfile name: %s\n", buf);
        printf("\t\tmode:         %s\n", mode_str);
        printf("\t\tuser:         %s\n", pw != NULL ? pw->pw_name : "unknown");
        printf("\t\tgroup:        %s\n", gr != NULL ? gr->gr_name : "unknown");
        printf("\t\tsize:         %ld\n", header.st_size);
        printf("\t\tmtime:        %s\n", mtime_date);
        printf("\t\tatime:        %s\n", atime_date);

        // Calculates CRC header
        crc_header = calculate_crc32(&header, sizeof(viktar_header_t));
        printf("\t\tcrc32 header: 0x%08x\n", crc_header);

        // Reads data and calculates CRC for data
        crc_data = crc32(0L, Z_NULL, 0);
        data_remaining = header.st_size;

        while (data_remaining > 0) {
            to_read = (data_remaining > BUFFER_SIZE) ? BUFFER_SIZE : data_remaining;
            bytes_read = read(iarch, crc_buf, to_read);
            if (bytes_read < 0) {
                perror("read error");
                close(iarch);
                exit(EXIT_FAILURE);
            }
            crc_data = crc32(crc_data, crc_buf, bytes_read);
            data_remaining -= bytes_read;
        }

        printf("\t\tcrc32 data:   0x%08x\n", crc_data);

        lseek(iarch, sizeof(viktar_footer_t), SEEK_CUR);
    }

        // Closing the file
        if (filename != NULL) {
            close(iarch);
        }

    }
    



    return EXIT_SUCCESS;
}

// Function to calculate CRC32
uint32_t calculate_crc32(const void *buf, size_t len) {
    return crc32(0L, Z_NULL, 0) ^ crc32(0L, buf, len);
}