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

void EEPROMClass::get(int address, HKStorage storage){
    
}
void EEPROMClass::put(int address, HKStorage storage){
    
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
