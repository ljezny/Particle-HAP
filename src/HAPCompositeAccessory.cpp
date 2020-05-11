//
//  CompositeAccessory.cpp
//  HKTester
//
//  Created by Lukas Jezny on 02/03/2019.
//  Copyright © 2019 Lukas Jezny. All rights reserved.
//

#include "HAPCompositeAccessory.h"



void HAPCompositeAccessory::identity(bool oldValue, bool newValue, HKConnection *sender) {
}

bool HAPCompositeAccessory::handle() {
    bool result = false;
    for(uint i = 0; i < descriptors.size(); i++) {
        //process only one accessory per "loop" step. So we dont delay Particle connection, Bonjour so much. Never mind it will be process one step later
        result |= descriptors.at(i)->handle();
    }
    return result;
}

void HAPCompositeAccessory::initAccessorySet(){
    Accessory *accessory = new Accessory();

    AccessorySet *accSet = &AccessorySet::getInstance();
    addInfoServiceToAccessory(accessory, "Composite accessory", "Vendor name", "Model  name", "1","1.0.0", std::bind(&HAPCompositeAccessory::identity, this, std::placeholders::_1, std::placeholders::_2,std::placeholders::_3));
    accSet->addAccessory(accessory);

    for(uint i = 0; i < descriptors.size(); i++) {
        descriptors.at(i)->initService(accessory);
    }
}
