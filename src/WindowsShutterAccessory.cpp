//
//  WindowsShutterAccessory.cpp
//  HKTester
//
//  Created by Lukas Jezny on 10/02/2019.
//  Copyright © 2019 Lukas Jezny. All rights reserved.
//

#include "WindowsShutterAccessory.h"

#include "homekit/HKConnection.h"

#include <set>

#ifdef PARTICLE_COMPAT
#include "../example/HKTester/HKTester/Particle_Compat/particle_compat.h"
#else
#include <Particle.h>
#endif
#include <string.h>
#include <stdlib.h>
#include <cstdio>

void shutterIdentity(bool oldValue, bool newValue, HKConnection *sender) {
    Serial.printf("Identify\n");
}

void WindowsShutterAccessory::initAccessorySet() {
    Accessory *shutterAccessory = new Accessory();

    //Add Light
    AccessorySet *accSet = &AccessorySet::getInstance();
    addInfoServiceToAccessory(shutterAccessory, "Shutter name", "Vendor name", "Model name", "1", &shutterIdentity);
    accSet->addAccessory(shutterAccessory);

    Service *windowsCoverService = new Service(serviceType_windowCover);
    shutterAccessory->addService(windowsCoverService);

    stringCharacteristics *nameCharacteristic = new stringCharacteristics(charType_serviceName, premission_read, 0);
    nameCharacteristic->characteristics::setValue("Window name");
    shutterAccessory->addCharacteristics(windowsCoverService, nameCharacteristic);
/*
    intCharacteristics *targetPosition = new intCharacteristics(charType_targetPosition, premission_read|premission_write|premission_notify, 0, 100, 1, unit_percentage);
    targetPosition->characteristics::setValue("50");
    //targetPosition->valueChangeFunctionCall = &brightTrackable;
    shutterAccessory->addCharacteristics(windowsCoverService, targetPosition);

    intCharacteristics *currentPosition = new intCharacteristics(charType_currentPosition, premission_read|premission_notify, 0, 100, 1, unit_percentage);
    currentPosition->characteristics::setValue("50");
    //targetPosition->valueChangeFunctionCall = &brightTrackable;
    shutterAccessory->addCharacteristics(windowsCoverService, currentPosition);

    intCharacteristics *positionState = new intCharacteristics(charType_positionState, premission_read|premission_notify, 0, 2, 1, unit_percentage);
    currentPosition->characteristics::setValue("2");
    //targetPosition->valueChangeFunctionCall = &brightTrackable;
    shutterAccessory->addCharacteristics(windowsCoverService, positionState);*/

};
