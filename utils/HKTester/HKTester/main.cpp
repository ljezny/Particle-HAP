//
//  main.cpp
//  HKTester
//
//  Created by Lukas Jezny on 03/02/2019.
//  Copyright © 2019 Lukas Jezny. All rights reserved.
//

#include <iostream>
#include <unistd.h>

#include "Particle_Compat/particle_compat.h"

#include "../../../src/HKConfig.h"
#include "../../../src/HKServer.h"

#import <Foundation/Foundation.h>

#include <stdio.h>


#include "../../../examples/relayswitch/RelaySwitchAccessory.h"

NSNetService *nsService;

HAPAccessoryDescriptor *acc = new RelaySwitchAccessory(0,false);


void progress(Progress_t progress) {
    Serial.printf("PROGRESS: %d\n",progress);
    delay(2000); //simulate slowness of photon
}
HKServer *server = NULL;
int main(int argc, const char * argv[]) {
   
    NSString *hapName = @"HKTester1";
    
    int passcodeNumber = 52312643;
    NSString *passscode = [NSString stringWithFormat:@"%d%d%d-%d%d-%d%d%d", (passcodeNumber / 10000000) % 10,(passcodeNumber / 1000000) % 10,(passcodeNumber / 100000) % 10,(passcodeNumber / 10000) % 10,(passcodeNumber / 1000) % 10,(passcodeNumber / 100) % 10,(passcodeNumber / 10) % 10,passcodeNumber % 10];
    
    server = new HKServer(acc->getDeviceType(), [hapName cString],[passscode cString],progress);
    
    nsService = [[NSNetService alloc] initWithDomain:@"" type:@"_hap._tcp." name:hapName port:TCP_SERVER_PORT];

    NSDictionary *txtDict = @{
        @"pv": [@"1.0" dataUsingEncoding:NSUTF8StringEncoding], // state
        @"id": [[NSString stringWithCString:server->getDeviceIdentity().c_str()] dataUsingEncoding:NSUTF8StringEncoding], // identifier
        @"c#": [@"1" dataUsingEncoding:NSUTF8StringEncoding], // version
        @"s#": [@"1" dataUsingEncoding:NSUTF8StringEncoding], // state
        @"sf": [@"1" dataUsingEncoding:NSUTF8StringEncoding], // discoverable
        @"ff": [@"0" dataUsingEncoding:NSUTF8StringEncoding], // mfi compliant
        @"md": [hapName dataUsingEncoding:NSUTF8StringEncoding], // name
        @"ci": [NSString stringWithFormat:@"%d",acc->getDeviceType()] // category identifier
    };
    
    NSData *txtData = [NSNetService dataFromTXTRecordDictionary:txtDict]; //NetService.data(fromTXTRecord: record)
    [nsService setTXTRecordData:txtData];
    
    
    [nsService startMonitoring];
    [nsService publish];
    
    //acc->descriptors.push_back(new WindowsShutterAccessory(14678913,14678916,sizeof(int)));
    //acc->descriptors.push_back(new WindowsShutterAccessory(4102033,4102036,2 * sizeof(int)));
    //acc->descriptors.push_back(new WindowsShutterAccessory(4102034,4102040,3 * sizeof(int)));
    //acc->descriptors.push_back(new MotionSensorAccessory());
    acc->initAccessorySet();
    
    server->start();
    
    for(;;) {
        server->handle();
        acc->handle();
        usleep(10);
    }
    return 0;
}
