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
#include "homekit/HKLog.h"

#define COVER_OPEN_TO_CLOSE_MS 56000
#define TILT_OPEN_TO_CLOSE_MS 2000

void WindowsShutterAccessory::shutterIdentity(bool oldValue, bool newValue, HKConnection *sender) {

}

void WindowsShutterAccessory::setState(int newState) {
    state = newState;
    positionStateChar->characteristics::setValue(format("%d",state)); //report state
    positionStateChar->notify(NULL);

}

void WindowsShutterAccessory::setTilt(int newTilt) {
    tilt = newTilt;
    currentTiltAngleChar->characteristics::setValue(format("%d",tilt)); //report state
    currentTiltAngleChar->notify(NULL);
}

void WindowsShutterAccessory::setPosition(int newPosition) {
    position = newPosition;
    currentPositionChar->characteristics::setValue(format("%d",position));//report position
    currentPositionChar->notify(NULL);
}

void WindowsShutterAccessory::handle() {
    if(state != 2) { //moving up or down
        if(endMS < millis()) { //expired, stop
            //stop current
            endMS = LONG_MAX;

            setState(2);
            setPosition(targetPosition);
            setTilt(targetTilt);
        }
    }

    switch (state) {
        case 0:
            rcSwitch.send(downCode, 24);
            break;
        case 1:
            rcSwitch.send(upCode, 24);
            break;
        case 2:
            break;
    }
}

void WindowsShutterAccessory::setTargetPosition (int oldValue, int newValue, HKConnection *sender) {
    //WindowsShutterAccessory *obj = (WindowsShutterAccessory*) arg;
    HKLogger.printf("setTargetPosition %d\n",newValue);
    int diff = abs(newValue - position);
    long time = (newValue == 0 || newValue == 100) ? COVER_OPEN_TO_CLOSE_MS : COVER_OPEN_TO_CLOSE_MS / 100 * diff;

    endMS = millis() + time;
    targetPosition = newValue;
    targetTilt = newValue > position ? 0 : -90;
    setState(newValue > position ? 1 : 0);
}

void WindowsShutterAccessory::setTargetTiltAngle (int oldValue, int newValue, HKConnection *sender) {
    HKLogger.printf("setTargetTiltAngle %d\n",newValue);

    int diff = abs(newValue - tilt);
    long time = TILT_OPEN_TO_CLOSE_MS / 90 * diff;

    endMS = millis() + time;
    targetTilt = newValue;

    setState(newValue > tilt ? 1 : 0);
}

void WindowsShutterAccessory::initAccessorySet() {
    pinMode(rcOutputPIN, OUTPUT);
    rcSwitch.enableTransmit(rcOutputPIN);
    rcSwitch.setProtocol(1);

    Accessory *shutterAccessory = new Accessory();

    AccessorySet *accSet = &AccessorySet::getInstance();
    addInfoServiceToAccessory(shutterAccessory, "Shutter name", "Vendor name", "Model  name", "1","1.0.0", std::bind(&WindowsShutterAccessory::shutterIdentity, this, std::placeholders::_1, std::placeholders::_2,std::placeholders::_3));
    accSet->addAccessory(shutterAccessory);

    Service *windowsCoverService = new Service(serviceType_windowCover);
    shutterAccessory->addService(windowsCoverService);

    stringCharacteristics *nameCharacteristic = new stringCharacteristics(charType_serviceName, premission_read, 0);
    nameCharacteristic->characteristics::setValue("Window name");
    shutterAccessory->addCharacteristics(windowsCoverService, nameCharacteristic);

    intCharacteristics *targetPosition = new intCharacteristics(charType_targetPosition, premission_read|premission_write|premission_notify, 0, 100, 1, unit_percentage);
    targetPosition->characteristics::setValue(format("%d",position));
    targetPosition->valueChangeFunctionCall = std::bind(&WindowsShutterAccessory::setTargetPosition, this, std::placeholders::_1, std::placeholders::_2,std::placeholders::_3);
    shutterAccessory->addCharacteristics(windowsCoverService, targetPosition);

    currentPositionChar = new intCharacteristics(charType_currentPosition, premission_read|premission_notify, 0, 100, 1, unit_percentage);
    currentPositionChar->characteristics::setValue(format("%d",position));
    shutterAccessory->addCharacteristics(windowsCoverService, currentPositionChar);

    positionStateChar = new intCharacteristics(charType_positionState, premission_read|premission_notify, 0, 2, 1, unit_percentage);
    positionStateChar->characteristics::setValue(format("%d",state));
    shutterAccessory->addCharacteristics(windowsCoverService, positionStateChar);

    intCharacteristics *targetHorizontalTiltAngle = new intCharacteristics(charType_targetHorizontalTiltAngle, premission_read|premission_write|premission_notify, -90, 0, 10, unit_arcDegree);
    targetHorizontalTiltAngle->characteristics::setValue(format("%d",tilt));
    targetHorizontalTiltAngle->valueChangeFunctionCall = std::bind(&WindowsShutterAccessory::setTargetTiltAngle, this, std::placeholders::_1, std::placeholders::_2,std::placeholders::_3);
    shutterAccessory->addCharacteristics(windowsCoverService, targetHorizontalTiltAngle);

    currentTiltAngleChar = new intCharacteristics(charType_currentHorizontalTiltAngle, premission_read|premission_notify, -90, 0, 10, unit_arcDegree);
    currentTiltAngleChar->characteristics::setValue(format("%d",tilt));
    shutterAccessory->addCharacteristics(windowsCoverService, currentTiltAngleChar);
};
