// Lab 2 viktar.c
// Alan Shirk
// alans@pdx.edu

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

#define BUFFER_SIZE 1000

void display_help(void);
void run_toc_short(const char *filename);
void run_toc_long(const char *filename);
void create_archive(const char *filename, int file_count, char *file_list[]);
void extract_archive(const char *filename);
void validate_archive(const char *filename);

int 
main(int argc, char *argv[]) {
    int verbose = FALSE;
    viktar_action_t action = ACTION_NONE;
    char *filename = NULL;

    // Processing command line options with getopt
    int opt = -1;
    while ((opt = getopt(argc, argv, OPTIONS)) != -1) {
        switch (opt) {
            case 'h': // Help info
                display_help();
                return EXIT_SUCCESS;
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
                printf("invalid option -- '%s'\n", optarg);
                printf("oopsie - unrecognized command line option \"(null)\"\n");
                break;
        }
    }

    // Verbose output
    if (verbose == TRUE) {
        fprintf(stderr, "verbose enabled\n");
    }

    // Switch for the viktar actions
    switch (action) {
        case ACTION_TOC_SHORT:
            run_toc_short(filename);
            break;
        case ACTION_TOC_LONG:
            run_toc_long(filename);
            break;
        case ACTION_CREATE:
            create_archive(filename, argc - optind, &argv[optind]);
            break;
        case ACTION_EXTRACT:
            extract_archive(filename);
            break;
        case ACTION_VALIDATE:
            validate_archive(filename);
            break;
        default:
            printf("no action supplied\n");
            printf("exiting without doing ANYTHING...\n");
            return EXIT_FAILURE;
            break;
    }

    return EXIT_SUCCESS;
}

// Shows the help text
void
display_help(void) {
    printf("help text\n");
    printf("\t%s\n", "viktar");
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
}

// Short table of contents
void 
run_toc_short(const char *filename) {
    int iarch = STDIN_FILENO;
    char buf[BUFFER_SIZE] = {'\0'};
    viktar_header_t header;

    if (filename == NULL) {
        fprintf(stderr, "reading archive from stdin\n");
    } else {
        iarch = open(filename, O_RDONLY);
        if (iarch < 0) {
            perror("cannot open archive file");
            exit(EXIT_FAILURE);
        }
        fprintf(stderr, "reading archive file: \"%s\"\n", filename);
        read(iarch, buf, strlen(VIKTAR_TAG));
    }

    if (strncmp(buf, VIKTAR_TAG, strlen(VIKTAR_TAG)) != 0) {
        fprintf(stderr, "not a viktar file: \"%s\"\n", filename != NULL ? filename : "stdin");
        exit(EXIT_FAILURE);
    }

    printf("Contents of viktar file: \"%s\"\n", filename != NULL ? filename : "stdin");

    while (read(iarch, &header, sizeof(viktar_header_t)) > 0) {
        memset(buf, 0, BUFFER_SIZE);
        strncpy(buf, header.viktar_name, VIKTAR_MAX_FILE_NAME_LEN);
        printf("\tfile name: %s\n", buf);
        lseek(iarch, header.st_size + sizeof(viktar_footer_t), SEEK_CUR);
    }

    if (filename != NULL) {
        close(iarch);
    }
}

// Long table of contents
void 
run_toc_long(const char *filename) {
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
    uint32_t crc_header;
    uint32_t crc_data;
    ssize_t bytes_read;
    size_t data_remaining;
    size_t to_read;
    char mode_str[11];

    if (filename == NULL) {
        fprintf(stderr, "reading archive from stdin\n");
    } else {
        iarch = open(filename, O_RDONLY);
        if (iarch < 0) {
            perror("cannot open archive file");
            exit(EXIT_FAILURE);
        }
        fprintf(stderr, "reading archive file: \"%s\"\n", filename);
        read(iarch, buf, strlen(VIKTAR_TAG));
    }

    if (strncmp(buf, VIKTAR_TAG, strlen(VIKTAR_TAG)) != 0) {
        fprintf(stderr, "not a viktar file: \"%s\"\n", filename != NULL ? filename : "stdin");
        exit(EXIT_FAILURE);
    }

    printf("Contents of viktar file: \"%s\"\n", filename != NULL ? filename : "stdin");

    while (read(iarch, &header, sizeof(viktar_header_t)) > 0) {
        memset(buf, 0, BUFFER_SIZE);
        strncpy(buf, header.viktar_name, VIKTAR_MAX_FILE_NAME_LEN);

        pw = getpwuid(header.st_uid);
        gr = getgrgid(header.st_gid);
        mtime = localtime(&header.st_mtim.tv_sec);
        atime = localtime(&header.st_atim.tv_sec);
        strftime(mtime_date, sizeof(mtime_date), "%Y-%m-%d %H:%M:%S %Z", mtime);
        strftime(atime_date, sizeof(atime_date), "%Y-%m-%d %H:%M:%S %Z", atime);

        printf("\tfile name: %s\n", buf);

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

        printf("\t\tmode:         %s\n", mode_str);
        printf("\t\tuser:         %s\n", pw != NULL ? pw->pw_name : "unknown");
        printf("\t\tgroup:        %s\n", gr != NULL ? gr->gr_name : "unknown");
        printf("\t\tsize:         %ld\n", header.st_size);
        printf("\t\tmtime:        %s\n", mtime_date);
        printf("\t\tatime:        %s\n", atime_date);

        crc_header = crc32(0L, Z_NULL, 0);
        crc_header = crc32(crc_header, (const Bytef *)&header, sizeof(viktar_header_t));
        printf("\t\tcrc32 header: 0x%08x\n", crc_header);

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

    if (filename != NULL) {
        close(iarch);
    }
}

// Creates viktar archive
void 
create_archive(const char *filename, int file_count, char *file_list[]) {
    int oarch;
    int ifd;
    struct stat st;
    viktar_header_t header;
    uint32_t crc_header;
    uint32_t crc_data;
    size_t data_remaining;
    size_t to_read;
    ssize_t bytes_read;
    unsigned char crc_buf[BUFFER_SIZE];
    viktar_footer_t footer;

    // Opening the archive file for writing
    if (filename != NULL) {
        oarch = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        if (oarch < 0) {
            perror("cannot open/create archive file");
            exit(EXIT_FAILURE);
        }
    } else {
        oarch = STDOUT_FILENO;
    }

    // Writing the tag to the viktar file
    if (write(oarch, VIKTAR_TAG, strlen(VIKTAR_TAG)) != (size_t)strlen(VIKTAR_TAG)) {
        perror("write error");
        if (filename != NULL) close(oarch);
        exit(EXIT_FAILURE);
    }

    // Loops through each file to add to the viktar archive
    for (int i = 0; i < file_count; i++) {
        char *member_filename = file_list[i];
        if (stat(member_filename, &st) < 0) {
            perror("stat error");
            if (filename != NULL) close(oarch);
            exit(EXIT_FAILURE);
        }

        ifd = open(member_filename, O_RDONLY);
        if (ifd < 0) {
            perror("cannot open member file");
            if (filename != NULL) close(oarch);
            exit(EXIT_FAILURE);
        }

        memset(&header, 0, sizeof(viktar_header_t));
        strncpy(header.viktar_name, member_filename, VIKTAR_MAX_FILE_NAME_LEN);
        header.st_size = st.st_size;
        header.st_mode = st.st_mode;
        header.st_uid = st.st_uid;
        header.st_gid = st.st_gid;
        header.st_atim = st.st_atim;
        header.st_mtim = st.st_mtim;

        crc_header = crc32(0L, Z_NULL, 0);
        crc_header = crc32(crc_header, (const Bytef *)&header, sizeof(viktar_header_t));

        if (write(oarch, &header, sizeof(viktar_header_t)) != sizeof(viktar_header_t)) {
            perror("write header error");
            close(ifd);
            if (filename != NULL) close(oarch);
            exit(EXIT_FAILURE);
        }

        crc_data = crc32(0L, Z_NULL, 0);
        data_remaining = st.st_size;

        while (data_remaining > 0) {
            to_read = (data_remaining > BUFFER_SIZE) ? BUFFER_SIZE : data_remaining;
            bytes_read = read(ifd, crc_buf, to_read);
            if (bytes_read < 0) {
                perror("read error");
                close(ifd);
                if (filename != NULL) close(oarch);
                exit(EXIT_FAILURE);
            }
            if (write(oarch, crc_buf, bytes_read) != bytes_read) {
                perror("write data error");
                close(ifd);
                if (filename != NULL) close(oarch);
                exit(EXIT_FAILURE);
            }
            crc_data = crc32(crc_data, crc_buf, bytes_read);
            data_remaining -= bytes_read;
        }

        footer.crc32_header = crc_header;
        footer.crc32_data = crc_data;

        if (write(oarch, &footer, sizeof(viktar_footer_t)) != sizeof(viktar_footer_t)) {
            perror("write footer error");
            close(ifd);
            if (filename != NULL) close(oarch);
            exit(EXIT_FAILURE);
        }

        close(ifd);
    }

    if (filename != NULL) {
        close(oarch);
    }
}

// Extracts from a viktar archive
void 
extract_archive(const char *filename) {
    int iarch;
    int ofd;
    viktar_header_t header;
    viktar_footer_t footer;
    uint32_t crc_header;
    uint32_t crc_data;
    size_t data_remaining;
    size_t to_read;
    ssize_t bytes_read;
    unsigned char crc_buf[BUFFER_SIZE];
    char buf[sizeof(VIKTAR_TAG)];
    struct timespec times[2];

    // Opening the archive file for reading
    if (filename != NULL) {
        iarch = open(filename, O_RDONLY);
        if (iarch < 0) {
            perror("cannot open archive file");
            exit(EXIT_FAILURE);
        }
    } else {
        fprintf(stderr, "reading archive from stdin\n");
        iarch = STDIN_FILENO;
    }

    // Validates the tag
    if (read(iarch, buf, strlen(VIKTAR_TAG)) != strlen(VIKTAR_TAG)) {
        perror("read error");
        if (filename != NULL) close(iarch);
        exit(EXIT_FAILURE);
    }
    buf[strlen(VIKTAR_TAG)] = '\0';

    if (strcmp(buf, VIKTAR_TAG) != 0) {
        fprintf(stderr, "not a viktar file: \"%s\"\n", filename != NULL ? filename : "stdin");
        if (filename != NULL) close(iarch);
        exit(EXIT_FAILURE);
    }

    // Processes each file in the archive
    while (read(iarch, &header, sizeof(viktar_header_t)) == sizeof(viktar_header_t)) {
        // Calculates and validates CRC for header
        crc_header = crc32(0L, Z_NULL, 0);
        crc_header = crc32(crc_header, (const Bytef *)&header, sizeof(viktar_header_t));

        // Creates and opens the output file
        ofd = open(header.viktar_name, O_WRONLY | O_CREAT | O_TRUNC, header.st_mode & 0777);
        if (ofd < 0) {
            perror("cannot create output file");
            if (filename != NULL) close(iarch);
            exit(EXIT_FAILURE);
        }

        // Reads and writes file data
        crc_data = crc32(0L, Z_NULL, 0);
        data_remaining = header.st_size;

        while (data_remaining > 0) {
            to_read = (data_remaining > BUFFER_SIZE) ? BUFFER_SIZE : data_remaining;
            bytes_read = read(iarch, crc_buf, to_read);
            if (bytes_read < 0) {
                perror("read error");
                close(ofd);
                if (filename != NULL) close(iarch);
                exit(EXIT_FAILURE);
            }
            if (write(ofd, crc_buf, bytes_read) != bytes_read) {
                perror("write error");
                close(ofd);
                if (filename != NULL) close(iarch);
                exit(EXIT_FAILURE);
            }
            crc_data = crc32(crc_data, crc_buf, bytes_read);
            data_remaining -= bytes_read;
        }

        // Reads and validates the footer
        if (read(iarch, &footer, sizeof(viktar_footer_t)) != sizeof(viktar_footer_t)) {
            perror("read footer error");
            close(ofd);
            if (filename != NULL) close(iarch);
            exit(EXIT_FAILURE);
        }

        if (footer.crc32_header != crc_header) {
            fprintf(stderr, "*** CRC32 failure header: %s  in file: 0x%08x  extract: 0x%08x ***\n", header.viktar_name, footer.crc32_header, crc_header);
        }
        if (footer.crc32_data != crc_data) {
            fprintf(stderr, "*** CRC32 failure data: %s  in file: 0x%08x  extract: 0x%08x ***\n", header.viktar_name, footer.crc32_data, crc_data);
        }

        // Restore file timestamps using futimens
        times[0] = header.st_atim;
        times[1] = header.st_mtim;
        if (futimens(ofd, times) < 0) {
            perror("futimens error");
            close(ofd);
            if (filename != NULL) close(iarch);
            exit(EXIT_FAILURE);
        }

        // Restores file permissions using fchmod
        if (fchmod(ofd, header.st_mode & 0777) < 0) {
            perror("fchmod error");
            close(ofd);
            if (filename != NULL) close(iarch);
            exit(EXIT_FAILURE);
        }

        // Closes the output file
        close(ofd);
    }

    // Closes the archive file
    if (filename != NULL) {
        close(iarch);
    }
}

// Validates viktar files
void 
validate_archive(const char *filename) {
    int iarch;
    viktar_header_t header;
    viktar_footer_t footer;
    uint32_t crc_header;
    uint32_t crc_data;
    size_t data_remaining;
    size_t to_read;
    ssize_t bytes_read;
    unsigned char crc_buf[BUFFER_SIZE];
    char buf[sizeof(VIKTAR_TAG)];
    int member_count = 0;

    // Opens the viktar file for reading
    if (filename != NULL) {
        iarch = open(filename, O_RDONLY);
        if (iarch < 0) {
            perror("cannot open archive file");
            exit(EXIT_FAILURE);
        }
    } else {
        iarch = STDIN_FILENO;
    }

    // Validates the tag
    if (read(iarch, buf, strlen(VIKTAR_TAG)) != strlen(VIKTAR_TAG)) {
        perror("read error");
        if (filename != NULL) close(iarch);
        exit(EXIT_FAILURE);
    }
    buf[strlen(VIKTAR_TAG)] = '\0';

    if (strcmp(buf, VIKTAR_TAG) != 0) {
        fprintf(stderr, "not a viktar file: \"%s\"\n", filename != NULL ? filename : "stdin");
        if (filename != NULL) close(iarch);
        exit(EXIT_FAILURE);
    }

    // Processes each file in the archive
    while (read(iarch, &header, sizeof(viktar_header_t)) == sizeof(viktar_header_t)) {
        member_count++;

        // Calculates CRC for header
        crc_header = crc32(0L, Z_NULL, 0);
        crc_header = crc32(crc_header, (const Bytef *)&header, sizeof(viktar_header_t));

        // Reads file data and calculates CRC
        crc_data = crc32(0L, Z_NULL, 0);
        data_remaining = header.st_size;

        while (data_remaining > 0) {
            to_read = (data_remaining > BUFFER_SIZE) ? BUFFER_SIZE : data_remaining;
            bytes_read = read(iarch, crc_buf, to_read);
            if (bytes_read < 0) {
                perror("read error");
                if (filename != NULL) close(iarch);
                exit(EXIT_FAILURE);
            }
            crc_data = crc32(crc_data, crc_buf, bytes_read);
            data_remaining -= bytes_read;
        }

        // Reads the footer
        if (read(iarch, &footer, sizeof(viktar_footer_t)) != sizeof(viktar_footer_t)) {
            perror("read footer error");
            if (filename != NULL) close(iarch);
            exit(EXIT_FAILURE);
        }

        printf("Validation for data member %d:\n", member_count);

        // Validates CRC for header
        if (footer.crc32_header == crc_header) {
            printf("\tHeader crc does match:     0x%08x   0x%08x for member %s\n",
                   footer.crc32_header, crc_header, header.viktar_name);
        } else {
            printf("\tHeader crc does not match: 0x%08x   0x%08x for member %d\n",
                   crc_header, footer.crc32_header, member_count);
        }

        // Validates CRC for data
        if (footer.crc32_data == crc_data) {
            printf("\tData crc does match:       0x%08x   0x%08x for member %s\n",
                   footer.crc32_data, crc_data, header.viktar_name);
        } else {
            printf("\tData crc does not match:   0x%08x   0x%08x for member %d\n",
                   crc_data, footer.crc32_data, member_count);
        }
    }

    // Closes the archive file
    if (filename != NULL) {
        close(iarch);
    }
}
