#include <getopt.h>
#include "common.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

option_t *get_option_arg(int argc, char *argv[]) {
    int opt;
    const char *filename = NULL;
    while ((opt = getopt(argc, argv, "f:")) != -1) {
        switch (opt) {
            case 'f':
                filename = optarg;
                if ( filename == NULL ) {
                    fprintf(stderr, "L'option -%c requiert un argument\n", optopt);
                    return NULL;
                }
                break;
            default:
                fprintf(stderr, "erreur\n");
                return NULL;
        }
    }

    if ( optind >= argc ) {
        fprintf(stderr, "Pas de domaine\n");
        return NULL;
    }
    char *domaine = argv[optind];

    optind++;
    if ( optind >= argc ) {
        fprintf(stderr, "Pas de port\n");
        return NULL;
    }

    char *err = NULL;
    int port = strtod(argv[optind], &err);

    if ( *err != 0 ) {
        fprintf(stderr, "Port is not a number\n");
        return NULL;
    }

    option_t *option_arg = calloc(1, sizeof(struct option));
    if ( option_arg == NULL ) {
        fprintf(stderr, "Erreur allocation de memoire\n");
        return NULL;
    }
    option_arg->domaine = domaine;
    option_arg->filename = filename;
    option_arg->port = port;

    return option_arg;
}
