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

#include "../../../src/homekit/HKConfig.h"
#include "../../../src/homekit/HKServer.h"

#import <Foundation/Foundation.h>

#include <stdio.h>


#include "../../../src/LightBulbAccessory.h"
#include "../../../src/WindowsShutterAccessory.h"

NSNetService *nsService;

//HAPAccessoryDescriptor *acc = new WindowsShutterAccessory();
HAPAccessoryDescriptor *acc = new LightBulbAccessory();

void progress(Progress_t progress) {
    Serial.printf("PROGRESS: %d\n",progress);
}

int main(int argc, const char * argv[]) {
    NSString *hapName = @"HKTester1";
    
    int passcodeNumber = 52312643;
    NSString *passscode = [NSString stringWithFormat:@"%d%d%d-%d%d-%d%d%d", (passcodeNumber / 10000000) % 10,(passcodeNumber / 1000000) % 10,(passcodeNumber / 100000) % 10,(passcodeNumber / 10000) % 10,(passcodeNumber / 1000) % 10,(passcodeNumber / 100) % 10,(passcodeNumber / 10) % 10,passcodeNumber % 10];
    
    HKServer server = HKServer(acc->getDeviceType(), [hapName cString],[passscode cString],progress);
    
    nsService = [[NSNetService alloc] initWithDomain:@"" type:@"_hap._tcp." name:@"HKTester" port:TCP_SERVER_PORT];

    NSDictionary *txtDict = @{
        @"pv": [@"1.0" dataUsingEncoding:NSUTF8StringEncoding], // state
        @"id": [[NSString stringWithCString:server.getDeviceIdentity()] dataUsingEncoding:NSUTF8StringEncoding], // identifier
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
    
    acc->initAccessorySet();
    
    server.setup();
    for(;;) {
        server.handle();
        acc->handle();
        usleep(10000);
    }
    return 0;
}
