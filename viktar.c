// Lab 2 viktar.c
// Alan Shirk - alans

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <zlib.h>

int 
main(int argc, char *argv[])
{
    int verbose = FALSE;

    // Processing command line options with getopt
    {
        int opt = -1;

        while ((opt = getopt(argc, argv, OPTIONS)) != -1) {
            switch (opt) {
            case 'h': // Help info
                return EXIT_SUCCESS;
                break;
                
            case 'v': // Verbose mode
                verbose_level++;
                fprintf(stderr, "verbose enabled\n");
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
    return EXIT_SUCCESS;
}