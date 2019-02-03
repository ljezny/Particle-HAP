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
using namespace std;

struct HKStorage;

class UDP {
private:
    int server_socket;
    struct sockaddr_in servaddr, cliaddr;
    uint8_t int_buffer[4096];
    size_t buffer_offset = 0;
public:
    void begin(int port){
        // Creating socket file descriptor
        if ( (server_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
            perror("socket creation failed");
        }
        
        memset(&servaddr, 0, sizeof(servaddr));
        memset(&cliaddr, 0, sizeof(cliaddr));
        
        // Filling server information
        servaddr.sin_family    = AF_INET; // IPv4
        servaddr.sin_addr.s_addr = INADDR_ANY;
        servaddr.sin_port = htons(port);
        //int    bind(int, const struct sockaddr *, socklen_t) __DARWIN_ALIAS(bind);
        // Bind the socket with the server address
        bind(server_socket, (const struct sockaddr *)&servaddr, sizeof(servaddr));
        

    }
    
    int beginPacket(const uint8_t host[4], uint16_t port){
        cliaddr.sin_family = AF_INET;
        
        cliaddr.sin_port = htons(port);
        cliaddr.sin_addr.s_addr = INADDR_ALLMDNS_GROUP;
        buffer_offset = 0;
        return 0;
    }
    size_t write(const uint8_t *buffer, size_t size){
        memcpy(int_buffer + buffer_offset, buffer, size);
        buffer_offset += size;
        return size;
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
        size_t len = sendto(server_socket, int_buffer, buffer_offset,
               0, (const struct sockaddr *) &cliaddr,
               sizeof(cliaddr));
        return len;
    }
    
    void stop(){
        
    }
    
};


class TCPClient {
private:
    int socket = 0;
public:
    void stop(){
        
    }
    bool connected(){
        return true;
    }
    
    bool available(){
        return false;
    }
    
    operator bool()
    {
        return socket != 0;
    }
    
    char* remoteIP() {
        return "1.1.1.1";
    }
    //int len = client.read(tempBuffer,tempBufferSize);
    int read(unsigned char *buffer, size_t size) {
        return 0;
    }
    void write(unsigned char *buffer, size_t len) {

    }
    
};

class TCPServer {
public:
    TCPServer(int port){
        
    }
    TCPClient available(){
        return TCPClient();
    }
    void begin(){
        
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
        return 0;
    }
    int println(const char *value) {
        return 0;
    }
};

class EEPROMClass {
public:
    void get(int address, HKStorage storage);
    void put(int address, HKStorage storage);
    
};


class IPAddress {
private:
    unsigned char buffer[4];
public:
    IPAddress() {
    }
    
    IPAddress(unsigned char address[4]) {
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
        return IPAddress();
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
