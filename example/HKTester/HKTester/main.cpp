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



HKServer server = HKServer();

int main(int argc, const char * argv[]) {
    server.setup();
    
    for(;;) {
        server.handle();
        usleep(10000);
    }
    
    return 0;
}
