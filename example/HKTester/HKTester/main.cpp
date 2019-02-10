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


NSNetService *nsService;



int main(int argc, const char * argv[]) {
    NSString *hapName = @"HKTester1";
    NSString *passscode = @"523-12-643";
    HKServer server = HKServer(HAP_ACCESSORY_TYPE, [hapName cString],[passscode cString]);
    
    nsService = [[NSNetService alloc] initWithDomain:@"" type:@"_hap._tcp." name:@"HKTester" port:TCP_SERVER_PORT];

    NSDictionary *txtDict = @{
        @"pv": [@"1.0" dataUsingEncoding:NSUTF8StringEncoding], // state
        @"id": [[NSString stringWithCString:server.getDeviceIdentity()] dataUsingEncoding:NSUTF8StringEncoding], // identifier
        @"c#": [@"1" dataUsingEncoding:NSUTF8StringEncoding], // version
        @"s#": [@"1" dataUsingEncoding:NSUTF8StringEncoding], // state
        @"sf": [@"1" dataUsingEncoding:NSUTF8StringEncoding], // discoverable
        @"ff": [@"0" dataUsingEncoding:NSUTF8StringEncoding], // mfi compliant
        @"md": [hapName dataUsingEncoding:NSUTF8StringEncoding], // name
        @"ci": [@"5" dataUsingEncoding:NSUTF8StringEncoding] // category identifier
    };
    
    NSData *txtData = [NSNetService dataFromTXTRecordDictionary:txtDict]; //NetService.data(fromTXTRecord: record)
    [nsService setTXTRecordData:txtData];
    
    
    [nsService startMonitoring];
    [nsService publish];
    
    initAccessorySet();
    
    server.setup();
    for(;;) {
        server.handle();
        usleep(10000);
    }
    return 0;
}
