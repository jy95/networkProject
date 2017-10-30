#include "client.h"

int isSendingWindowFull(window_util_t *windowUtil, uint8_t FirstSeqNumInWindow) {

    // on obtient la taille
    int windowSize = get_window(windowUtil);

    // si la fenêtre du client est à 0, c'est bloquant
    if ( windowSize == 0 )
        return 1;

    // on commence
    uint8_t index = FirstSeqNumInWindow;
    // par défaut, y a pas de place
    int result = 1;
    // savoir si on doit continuer
    int shouldContinue = 1;
    // un counter pour savoir combien d'éléments on doit parcourir
    int count = 0;

    while (count < windowSize && shouldContinue == 1) {

        // technique en C pour checker si cela est null
        if ((windowUtil->storedPackets)[index] ) {
            // on a trouvé un emplacement libre
            result = 0;
            // on arrête la boucle
            shouldContinue--;
        }
        index++;
        count++;
    }
    return result;
}

/**
 * Permet de savoir si le numéro de sequence est valide
 * @param seqnum le numéro à vérfier
 * @param start le numéro de séquence du début de la window
 * @param count le nombre de message envoyés
 * @return 1 si c'est le cas , 0 sinon
 */
unsigned int isInSlidingWindowOfClient(uint8_t seqnum, uint8_t start, int count) {

    // à partir de quel numéro on commence
    uint8_t index = start;
    int counter = 0; // pour savoir
    int shouldStop = 0; // pour savoir si on doit s'arrêter dans le while
    int result = 0; // par défaut c'est hors séquence

    while (counter < count && shouldStop == 0) {

        // on a trouvé notre numéro de séquence
        if ( index == (seqnum -1) ) {
            shouldStop = 1;
            result = 1;
        }
        index++; //
        counter++; // on incrémente pour checker le suivant
    }
    return result;
}

