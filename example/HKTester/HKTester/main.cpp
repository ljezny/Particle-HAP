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

#include <pthread.h>
#include <stdio.h>
#include "../../../src/homekit/Accessory.h"

NSNetService *nsService;

HKServer server = HKServer();

void *handle(void *vargp) {
    server.setup();
    for(;;) {
        server.handle();
        usleep(10000);
    }
    
}

int main(int argc, const char * argv[]) {
    
    nsService = [[NSNetService alloc] initWithDomain:@"" type:@"_hap._tcp." name:@hapName port:TCP_SERVER_PORT];
    
    NSDictionary *txtDict = @{
        @"pv": [@"1.0" dataUsingEncoding:NSUTF8StringEncoding], // state
        @"id": [@deviceIdentity dataUsingEncoding:NSUTF8StringEncoding], // identifier
        @"c#": [@"1" dataUsingEncoding:NSUTF8StringEncoding], // version
        @"s#": [@"1" dataUsingEncoding:NSUTF8StringEncoding], // state
        @"sf": [@"1" dataUsingEncoding:NSUTF8StringEncoding], // discoverable
        @"ff": [@"0" dataUsingEncoding:NSUTF8StringEncoding], // mfi compliant
        @"md": [@hapName dataUsingEncoding:NSUTF8StringEncoding], // name
        @"ci": [@"5" dataUsingEncoding:NSUTF8StringEncoding] // category identifier
    };
    
    NSData *txtData = [NSNetService dataFromTXTRecordDictionary:txtDict]; //NetService.data(fromTXTRecord: record)
    [nsService setTXTRecordData:txtData];
    
    
    [nsService startMonitoring];
    [nsService publish];
    
    initAccessorySet();
    
    pthread_t workthread;
   
    pthread_create(&workthread,NULL, &handle, NULL);
    pthread_join(workthread, NULL);
    return 0;
}
