//
//  particle_compat.h
//  HKTester
//
//  Created by Lukas Jezny on 03/02/2019.
//  Copyright © 2019 Lukas Jezny. All rights reserved.
//

#ifndef particle_compat_h
#define particle_compat_h

#include <unistd.h>
#include <sys/time.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/un.h>
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
//LEDStatus RGB_STATUS_YELLOW(RGB_COLOR_YELLOW, LED_PATTERN_FADE, LED_PRIORITY_IMPORTANT);
class LEDStatus {
public:
    LEDStatus(int x, int y, int z){
        
    }
    void setActive(bool active) {
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
        
        int set = 1;
        setsockopt(socket, SOL_SOCKET, SO_NOSIGPIPE, (void *)&set, sizeof(int));
    }
    
    void stop(){
        
    }
    bool connected(){
        return true;
    }
    int status() {
        return 1;
    }
    int available(){
        read_buffer_offset = 0;
        int l = recv(socket,read_buffer,4096,0);
        if(l>0){
            read_buffer_offset += l;
            return l;
        }
        return 0;
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
        int result = read_buffer_offset;
        read_buffer_offset = 0;
        sleep(1);
        return result;
    }
    int write(unsigned char *buffer, size_t len) {
        send(socket, buffer, len, 0);
        return len;
    }
    int getWriteError() {
        return 0;
    }
};

class TCPServer {
private:
    int server_socket;
    struct sockaddr_in servaddr, cliaddr;
    
public:
    int port;
    TCPServer(int port){
        this->port = port;
        if ( (server_socket = socket(PF_INET, SOCK_STREAM, 0)) < 0 ) {
            perror("socket creation failed");
        }
        int opt_val = 1;
        setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof opt_val);
        setsockopt(server_socket, SOL_SOCKET, SO_REUSEPORT, &opt_val, sizeof opt_val);
        
        memset(&servaddr, 0, sizeof(servaddr));
        
        // Filling server information
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(port);
        servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
        
        
        
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
    int get(int address, int value){
        return 0;
    }
    void put(int address, int value){
        
    }
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

class Logger{
public:
    Logger(const char *name) {
        
    }
    int info(const char *format, ...){
        va_list argptr;
        va_start(argptr, format);
        vfprintf(stdout, format, argptr);
        va_end(argptr);
        return 0;
    }
    int warn(const char *format, ...){
        va_list argptr;
        va_start(argptr, format);
        vfprintf(stdout, format, argptr);
        va_end(argptr);
        return 0;
    }
};

class ParticleClass {
public:
    void variable(const char *format, ...){
       
    }
    
    void publish(const char *format, ...){
        
    }
   
};

//typedef std::function<void()> wiring_interrupt_handler_t;

void delay(int ms);
void delayMicroseconds(int micros);
long int millis();
long int micros();
int random(int max);
int analogRead(int pin);
int digitalRead(int pin);
void digitalWrite(int pin, int value);
void analogWrite(int pin, int value);
void pinMode(int pin, int mode);

extern EthernetClass Ethernet;
extern UDP udp;
extern EEPROMClass EEPROM;
extern SerialLink Serial;
extern RGBClass RGB;
extern ParticleClass Particle;

#define INT 0
#define PUBLIC 0
#define PRIVATE 0

#define A0 0
#define D0 0
#define D1 1
#define D2 2
#define D4 4
#define D5 5
#define D6 6
#define LOW 0
#define HIGH 1
#define OUTPUT 0
#define INPUT 1
#define CHANGE 0
#define RGB_COLOR_YELLOW 0
#define LED_PATTERN_FADE 0
#define LED_PRIORITY_IMPORTANT 0

#endif /* particle_compat_h */
