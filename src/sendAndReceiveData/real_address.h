//
// Created by jy95 on 05/10/2017.
//

#ifndef NETWORKPROJECT_REAL_ADDRESS_H
#define NETWORKPROJECT_REAL_ADDRESS_H

#include <netinet/in.h> /* * sockaddr_in6 */
#include <sys/types.h> /* sockaddr_in6 */

typedef struct option_arg { //Regroupe trois type d'argument possibles
    const char* filename; //le nom de fichier avec l'option -f
    const char* domaine; // le nom de domain ou l'adresse IPV6
    const char* port; //Le port de destination
} option_t;

/* Resolve the resource name to an usable IPv6 address
 * @address: The name to resolve
 * @rval: Where the resulting IPv6 address descriptor should be stored
 * @return: NULL if it succeeded, or a pointer towards
 *          a string describing the error if any.
 *          (const char* means the caller cannot modify or free the return value,
 *           so do not use malloc!)
 */
const char * real_address(const char *address, struct sockaddr_in6 *rval);


#endif //NETWORKPROJECT_REAL_ADDRESS_H
