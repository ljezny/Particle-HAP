//
//  particle_compat.h
//  HKTester
//
//  Created by Lukas Jezny on 03/02/2019.
//  Copyright © 2019 Lukas Jezny. All rights reserved.
//

#ifndef particle_compat_h
#define particle_compat_h

#include <sys/socket.h>

#include <unistd.h>
#include <sys/time.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdarg.h>
#include <fcntl.h>
using namespace std;

struct HKStorage;

class UDP {
private:
public:
    void begin(int port){
    }
    
    int beginPacket(const uint8_t host[4], uint16_t port){
        return 0;
    }
    size_t write(const uint8_t *buffer, size_t size){
        return 0;
    }
    size_t read(const uint8_t *buffer, size_t size){
        return 0;
    }
    
    void setBuffer(size_t size, const uint8_t *buffer){
    }
    
    int parsePacket() {
        return 0;
    }
    
    void flush() {
    }
    int remotePort() {
        return 0;
    }
    int remoteIP() {
        return 0;
    }
    int endPacket() {
        return 0;
    }
    
    void stop(){
    }
    
};


typedef int sock_result_t;
typedef int network_interface_t;
int socket_handle_invalid();
int socket_handle_valid(int socket);
int socket_create_tcp_server(int port, int nif);
int socket_accept(int server_socket);
int socket_close(int socket);
int socket_receive(int socket, uint8_t *buffer,size_t size, int timeout);
int socket_send(int socket,uint8_t *buffer,size_t size);

class RGBClass {
public:
    void control(bool value) {
        
    }
    void color(unsigned char r, unsigned char g, unsigned char b) {
        
    }
};

class SerialLink {
public:
    int printf(const char *format, ...){
        va_list argptr;
        va_start(argptr, format);
        vfprintf(stdout, format, argptr);
        va_end(argptr);
        return 0;
    }
    int println(const char *value) {
        fprintf(stdout, value);
        fprintf(stdout, "\n");
        //vfprintf(stdout, value);
        return 0;
    }
};

class EEPROMClass {
public:
    HKStorage get(int address, HKStorage storage);
    void put(int address, HKStorage storage);
    
};


class IPAddress {
private:
    unsigned char buffer[4];
public:
    IPAddress() {
    }
    
    void load(unsigned char *address) {
        memcpy(buffer,address,4);
    }
    unsigned char const& operator[](int index) const
    {
        return buffer[index];
    }
};

class EthernetClass {
public:
    IPAddress localIP(){
        //int socket_local4(int s,char ip[4],uint16 *port)
        char hostbuffer[256];
        char *IPbuffer;
        struct hostent *host_entry;
        int hostname;
        
        // To retrieve hostname
        hostname = gethostname(hostbuffer, sizeof(hostbuffer));
        
        // To retrieve host information
        host_entry = gethostbyname(hostbuffer);
        
        // To convert an Internet network
        // address into ASCII string
       // IPbuffer = inet_ntoa(*((struct in_addr*)
         //                      host_entry->h_addr_list[0]));
        unsigned char* address =( unsigned char*) host_entry->h_addr_list[0];
        IPAddress a = IPAddress();
        a.load(address);;
        return a;
    }
};


void delay(int ms);
long int millis();

extern EthernetClass Ethernet;
extern UDP udp;
extern EEPROMClass EEPROM;
extern SerialLink Serial;
extern RGBClass RGB;

#endif /* particle_compat_h */
