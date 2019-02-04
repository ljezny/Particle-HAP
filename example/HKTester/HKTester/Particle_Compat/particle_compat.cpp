//
//  particle_compat.cpp
//  HKTester
//
//  Created by Lukas Jezny on 03/02/2019.
//  Copyright © 2019 Lukas Jezny. All rights reserved.
//
#include "../../../../src/homekit/HKPersistor.h"

#include <stdio.h>

#include "particle_compat.h"

EthernetClass Ethernet;
UDP udp;
EEPROMClass EEPROM;
SerialLink Serial;
RGBClass RGB;

HKStorage EEPROMClass::get(int address, HKStorage storage){
    char file[256];
    strcat(strcpy(file, getenv("HOME")), "/particle_hap_storage");
    
    FILE* f = fopen(file, "r");
    if(f){
        fread(&storage, sizeof(HKStorage), 1, f);
        fclose(f);
    }
    return storage;
}
void EEPROMClass::put(int address, HKStorage storage){
    char file[256];
    strcat(strcpy(file, getenv("HOME")), "/particle_hap_storage");
    
    FILE* f = fopen(file, "w");
    if(f){
        fwrite(&storage, sizeof(HKStorage), 1, f);
        fclose(f);
    }
}

void delay(int ms) {
    usleep(100 * ms);
}

long int timestamp() {
    struct timeval tp;
    gettimeofday(&tp, NULL);
    long int ms = tp.tv_sec * 1000 + tp.tv_usec / 1000;
    return ms;
}

long int start = timestamp();

long int millis() {
    return timestamp() - start;
}

int socket_handle_invalid() {
    return -1;
}

int socket_handle_valid(int socket) {
    return socket_handle_invalid() != socket;
}

int socket_create_tcp_server(int port, int nif) {
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    
    int server_socket = socket_handle_invalid();
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    servaddr.sin_family    = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(port);
    bind(server_socket, (const struct sockaddr *)&servaddr, sizeof(servaddr));
    if ((listen(server_socket, 0)) != 0) {
        perror("Listen failed...\n");
    }
    
    fcntl(server_socket, F_SETFL, O_NONBLOCK);
    return server_socket;
}

int socket_accept(int server_socket){
    struct sockaddr cli;
    socklen_t len;
    int s = accept(server_socket,(struct sockaddr *) &cli, &len);
    if(s != -1){
        struct timeval read_timeout;
        read_timeout.tv_sec = 0;
        read_timeout.tv_usec = 10;
        setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &read_timeout, sizeof read_timeout);
    }
    

    
    return s;
}

int socket_close(int socket){
    return shutdown(socket,2);
}

int socket_receive(int socket, uint8_t *buffer,size_t size, int timeout) {
    int l = recv(socket,buffer,size,0);
    if(l>0){
        return l;
    }
    return 0;
}

int socket_send(int socket,uint8_t *buffer,size_t size){
    return send(socket, buffer, size, 0);
}
