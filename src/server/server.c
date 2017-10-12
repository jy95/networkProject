#include <stdio.h>  /* FILE */
#include <stdlib.h> // malloc , etc

int main(int argc, char *argv[]) {
    // just un début pour le compilateur
    if ( argv ) {
        // blabla
    }
    fprintf(stdout, "Nombre d'arguments : %d\n", argc);
    return EXIT_SUCCESS;
}