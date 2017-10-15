//
// Created by jy95 on 15/10/2017.
//

#ifndef PROJECT_COMMON_H
#define PROJECT_COMMON_H

typedef struct option_arg { //Regroupe trois type d'argument possibles
    const char *filename; //le nom de fichier avec l'option -f
    const char *domaine; // le nom de domain ou l'adresse IPV6
    int port; //Le port de destination
} option_t;

// traitter les arguments en ligne de commande
option_t *get_option_arg(int argc, char *argv[]);

#endif //PROJECT_COMMON_H
