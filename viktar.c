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

#define HEADER_SIZE strlen(viktar_header_t)
#define FOOTER_SIZE strlen(viktar_footer_t)

void display_help(void);

int 
main(int argc, char *argv[])
{
    int verbose = FALSE;
    // viktar_action_t action = ACTION_NONE;
    char *filename = NULL;
    int iarch = STDIN_FILENO;
    char buf[100] = {'\0'};
    viktar_header_t md;

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
                fprintf(stderr, "verbose enabled\n");
                break;

            case 'x': // Extract members from viktar file
                //action = ACTION_EXTRACT;
                break;

            case 'c': // Create viktar file
                break;

            case 't': // Short table of contents

                break;

            case 'T': // Long table of contents
                break;

            case 'f': // Sets viktar input file
                filename = optarg;
                break;
            
            case 'V': // Validate archive member with CRC values
                break;

            default: // Invalid options
                fprintf(stderr, "unknown option... Exiting.\n");
                return EXIT_FAILURE;
                break;
            }
        }
    }

    // If filename is set with -f, assigning the file descriptor to iarch
    if (filename != NULL) {
        iarch = open(filename, O_RDONLY);
    }

    // Reading the file
    fprintf(stderr, "reading archive file: \"%s\"\n", filename);
    read(iarch, buf, strlen(VIKTAR_TAG));

    // Validates the tag
    if(strncmp(buf, VIKTAR_TAG, strlen(VIKTAR_TAG) != 0)) {
        // Not a valid viktar file
        fprintf(stderr, "not a viktar file: \"%s\"\n", filename);
        exit(EXIT_FAILURE);
    }

    // Processing the archive file metadata
    printf("Contents of viktar file: \"%s\"\n", filename != NULL ? filename : "stdin");

    while (read(iarch, &md, sizeof(viktar_header_t)) > 0) {
        // Printing archive member name
    }


    if (optind < argc) {
        for (int i = optind; i < argc; i++) {
            // Verbose output
            if (verbose == TRUE) {
            }
        }
    }
    return EXIT_SUCCESS;
}