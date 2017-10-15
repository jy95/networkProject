//
// Created by jy95 on 14/10/2017.
//

#ifndef PROJECT_CLIENT_H
#define PROJECT_CLIENT_H

#define MAX_LATENCE_TIME    2000 // le temps maximal
#define EXPECTED_LATENCE_RATE  10   // un pourcentage arbitraire pour évaluer délai/latence/le temps de traitement
#define MAX_PAYLOAD_SIZE    512  // la taille MAX du payload

typedef struct option_arg { //Regroupe trois type d'argument possibles
    const char *filename; //le nom de fichier avec l'option -f
    const char *domaine; // le nom de domain ou l'adresse IPV6
    int port; //Le port de destination
} option_t;

// traitter les arguments en ligne de commande
option_t *get_option_arg(int argc, char *argv[]);

#endif //PROJECT_CLIENT_H
