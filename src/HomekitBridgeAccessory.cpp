//
//  HomekitBridgeAccessory.cpp
//  HKTester
//
//  Created by Lukas Jezny on 02/03/2019.
//  Copyright © 2019 Lukas Jezny. All rights reserved.
//

#include "HomekitBridgeAccessory.h"

void HomekitBridgeAccessory::handle() {
    for(int i = 0; i < descriptors.size(); i++) {
        descriptors.at(i)->handle();
    }
}

void HomekitBridgeAccessory::initAccessorySet(){
    for(int i = 0; i < descriptors.size(); i++) {
        descriptors.at(i)->initAccessorySet();
    }
}
