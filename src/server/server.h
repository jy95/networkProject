//
// Created by jy95 on 15/10/2017.
//

#ifndef PROJECT_SERVER_H
#define PROJECT_SERVER_H

#define BUFFER_MAX_SIZE 31
#define MAX_WINDOW_SIZE 32
#define MAX_PACKET_RECEIVED_SIZE_FOR_SERVER 530// la taille max

#include <arpa/inet.h>
#include <memory.h>
#include <stdio.h>  /* FILE */
#include <stdlib.h> // malloc , etc
#include <time.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "../client/client.h"
#include "../sendAndReceiveData/real_address.h"
#include "../sendAndReceiveData/create_socket.h"
#include "../sendAndReceiveData/read_write_loop.h"
#include "../paquet/packet_interface.h"
//#include <netinet6/in6.h> // pas compatible ici

#endif //PROJECT_SERVER_H
