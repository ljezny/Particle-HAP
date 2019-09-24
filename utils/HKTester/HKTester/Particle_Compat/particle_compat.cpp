//
//  particle_compat.cpp
//  HKTester
//
//  Created by Lukas Jezny on 03/02/2019.
//  Copyright © 2019 Lukas Jezny. All rights reserved.
//
#include "../../../../src/HKPersistor.h"

#include <stdio.h>

#include "particle_compat.h"

EthernetClass Ethernet;
UDP udp;
EEPROMClass EEPROM;
SerialLink Serial;
RGBClass RGB;
ParticleClass Particle;

HKStorage EEPROMClass::get(int address, HKStorage storage){
    char file[256];
    strcat(strcpy(file, getenv("HOME")), "/particle_hap_storage");
    
    FILE* f = fopen(file, "r");
    if(f){
        fread(&storage, sizeof(HKStorage), 1, f);
        fclose(f);
    } else {
        memset(&storage, 0xFF, sizeof(HKStorage)); //simulate empty eeprom
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
void delayMicroseconds(int micros) {
    usleep(micros);
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
long int micros() {
    return timestamp() - start;
}
int random(int max) {
    return random() % max;
}
int analogRead(int pin) {
    return random() % 4096;
}
int digitalRead(int pin){
    return 1;
}
void digitalWrite(int pin, int value){
    
}
void analogWrite(int pin, int value){
    
}
void pinMode(int pin, int mode){
    
}
