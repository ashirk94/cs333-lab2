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

void display_help(void);

int 
main(int argc, char *argv[])
{
    int verbose = FALSE;
    viktar_action_t action = ACTION_NONE;
    char *filename = NULL;
    int iarch = STDIN_FILENO;
    char buf[100] = {'\0'};
    viktar_header_t header;
    viktar_footer_t footer;
    struct passwd *pw;
    struct group *gr;
    struct tm *mtime;
    struct tm *atime;
    char mtime_date[20];
    char atime_date[20];

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
        // Getting metadata
        pw = getpwuid(header.st_uid);
        gr = getgrgid(header.st_gid);
        mtime = localtime(&header.st_mtim.tv_sec);
        atime = localtime(&header.st_atim.tv_sec);
        strftime(mtime_date, sizeof(mtime_date), "%Y-%m-%d %H:%M:%S %Z", mtime);
        strftime(atime_date, sizeof(atime_date), "%Y-%m-%d %H:%M:%S %Z", atime);

        // Processing the archive file metadata
        printf("Contents of viktar file: \"%s\"\n", filename != NULL ? filename : "stdin");

        while (read(iarch, &header, sizeof(viktar_header_t)) > 0) {
            // Printing archive member name
            memset(buf, 0, 100);
            strncpy(buf, header.viktar_name, VIKTAR_MAX_FILE_NAME_LEN);
            printf("\tfile name: %s\n", buf);
            printf("\t\tmode:         %04o\n", header.st_mode & 07777);
            printf("\t\tuser:         %s\n", pw->pw_name);
            printf("\t\tgroup:        %s\n", gr->gr_name);
            printf("\t\tsize:         %ld\n", header.st_size);
            printf("\t\tmtime:        %s\n", mtime_date);
            printf("\t\tatime:        %s\n", atime_date);

            // Read the footer and print the CRC values
            if (read(iarch, &footer, sizeof(viktar_footer_t)) > 0) {
                printf("\t\tcrc32 header: 0x%08x\n", footer.crc32_header);
                printf("\t\tcrc32 data:   0x%08x\n", footer.crc32_data);
            }

            lseek(iarch, header.st_size, SEEK_CUR);
        }

        // Closing the file
        if (filename != NULL) {
            close(iarch);
        }

    }



    return EXIT_SUCCESS;
}