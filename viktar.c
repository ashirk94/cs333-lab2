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

int 
main(int argc, char *argv[])
{
    int verbose = FALSE;
    //viktar_action_t action = ACTION_NONE;
    // const char *archive_file = NULL;
    // const char **files_to_extract = NULL;
    // int num_files = 0;

    // Processing command line options with getopt
    {
        int opt = -1;

        while ((opt = getopt(argc, argv, OPTIONS)) != -1) {
            switch (opt) {
            case 'h': // Help info
                printf("viktar <options> [archive-file] [member [...]]");
                printf("\t-x\tExtract members from viktar file\n");
                printf("\t-c\tCreate a viktar style archive file\n");
                printf("\t-t\tShort table of contents\n");
                printf("\t-T\tLong table of contents\n");
                printf("\t-t\tLong table of contents\n");
                printf("\t-f filename\tSpecify the name of the viktar file on which to operate\n");
                printf("\t-V\tValidate the content of the archive member with the CRC values stored in the archive file\n");
                printf("\t-h\tShow help text and exit\n");
                printf("\t-v\tVerbose mode\n");
                return EXIT_SUCCESS;
                break;
                
            case 'v': // Verbose mode
                verbose = TRUE;
                fprintf(stderr, "verbose enabled\n");
                break;

            case 'x':
                //action = ACTION_EXTRACT;
                break;

            case 'c':
                break;

            case 't':
                break;

            case 'T':
                break;

            case 'f':
                //archive_file = optarg;
                break;
            
            case 'V':
                break;

            default: // Invalid options
                fprintf(stderr, "unknown option... Exiting.\n");
                return EXIT_FAILURE;
                break;
            }
        }
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

// // Extracts files from the viktar archive
// int extract_files(const char *archive_file, const char **files_to_extract, int num_files) {
//     int archive_fd;
//     if (archive_file) {
//         // Open the archive file for reading
//         archive_fd = open(archive_file, O_RDONLY);
//         if (archive_fd == -1) {
//             perror("Error opening archive file");
//             return EXIT_FAILURE;
//         }
//     } else {
//         // Read from stdin
//         archive_fd = STDIN_FILENO;
//     }

//     // Loop through each file in the viktar archive
//     for (int i = 0; i < num_files; i++) {
//         // Extract files based on their names provided in files_to_extract
        
//         // Get file status
//         struct stat st;
//         if (stat(files_to_extract[i], &st) == -1) {
//             perror("Error getting file status");
//             return 1;
//         }

//         // Restore timestamps
//         struct utimbuf utime_buf;
//         utime_buf.actime = st.st_atime;
//         utime_buf.modtime = st.st_mtime;
//         if (utime(files_to_extract[i], &utime_buf) == -1) {
//             perror("Error restoring timestamps");
//             return EXIT_FAILURE;
//         }

//         // Restore permissions
//         if (chmod(files_to_extract[i], st.st_mode) == -1) {
//             perror("Error restoring file permissions");
//             return EXIT_FAILURE;
//         }

//         // Validate CRC values
//         uLong crc32_data = crc32(0L, Z_NULL, 0);
//         unsigned char buffer[1024]; // Adjust buffer size as needed
//         ssize_t bytes_read;
//         int file_fd = open(files_to_extract[i], O_RDONLY);
//         if (file_fd == -1) {
//             perror("Error opening file for CRC validation");
//             return EXIT_FAILURE;
//         }
//         while ((bytes_read = read(file_fd, buffer, sizeof(buffer))) > 0) {
//             crc32_data = crc32(crc32_data, buffer, (uInt)bytes_read);
//         }
//         close(file_fd);

//         // Compare CRC values and issue warning if they do not match
//         // Extract the file anyway

//         // Overwrite existing files if necessary
//     }

//     if (archive_file) {
//         close(archive_fd);
//     }

//     return 0;
// }