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


class TCPClient {
private:
    int socket = 0;
    uint8_t read_buffer[4096];
    size_t read_buffer_offset = 0;
public:
    TCPClient(){
        socket = -1;
    }
    
    TCPClient(int s){
        socket = s;
        struct timeval read_timeout;
        read_timeout.tv_sec = 0;
        read_timeout.tv_usec = 10;
        setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, &read_timeout, sizeof read_timeout);
    }
    
    void stop(){
        
    }
    bool connected(){
        return true;
    }
    int status() {
        return 1;
    }
    bool available(){
        read_buffer_offset = 0;
        int l = recv(socket,read_buffer,4096,0);
        if(l>0){
            read_buffer_offset += l;
        }
        return read_buffer_offset > 0;
    }
    
    operator bool()
    {
        return socket != -1;
    }
    
    char* remoteIP() {
        return "1.1.1.1";
    }
    //int len = client.read(tempBuffer,tempBufferSize);
    int read(unsigned char *buffer, size_t size) {
        memcpy(buffer, read_buffer, read_buffer_offset);
        return read_buffer_offset;
    }
    void write(unsigned char *buffer, size_t len) {
        send(socket, buffer, len, 0);
    }
    
};

class TCPServer {
private:
    int server_socket;
    struct sockaddr_in servaddr, cliaddr;
public:
    TCPServer(int port){
        if ( (server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
            perror("socket creation failed");
        }
        
        memset(&servaddr, 0, sizeof(servaddr));
        //memset(&cliaddr, 0, sizeof(cliaddr));
        
        // Filling server information
        servaddr.sin_family    = AF_INET; // IPv4
        servaddr.sin_addr.s_addr = INADDR_ANY;
        servaddr.sin_port = htons(port);
        
        bind(server_socket, (const struct sockaddr *)&servaddr, sizeof(servaddr));
        if ((listen(server_socket, 0)) != 0) {
            perror("Listen failed...\n");
        }
        
        fcntl(server_socket, F_SETFL, O_NONBLOCK);
    }
    TCPClient available(){
        struct sockaddr cli;
        socklen_t len;
        int conn = accept(server_socket,(struct sockaddr *) &cli, &len);
        
        return TCPClient(conn);
    }
    void begin(){
        
    }
    void stop(){
        
    }
    
};

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
