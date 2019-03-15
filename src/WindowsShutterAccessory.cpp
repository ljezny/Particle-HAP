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
#define COVER_OPEN_TO_CLOSE_TRANSMIT_REPEATS 1244 //100 repeat tooked 4500ms

RCSwitch *rcSwitch = NULL; //this is static, it will be initialized one time

void WindowsShutterAccessory::shutterIdentity(bool oldValue, bool newValue, HKConnection *sender) {

}

void WindowsShutterAccessory::setState(int newState) {
    if(state != newState) {
      state = newState;
      positionStateChar->characteristics::setValue(format("%d",state)); //report state
      positionStateChar->notify(NULL);

      setPosition(position);
    }
}

void WindowsShutterAccessory::setPosition(int newPosition) {
    if(newPosition != position) {
      position = newPosition;
      currentPositionChar->characteristics::setValue(format("%d",position));//report position
      currentPositionChar->notify(NULL);

      EEPROM.put(this->eepromAddr, this->position);
    }
}

void WindowsShutterAccessory::handle() {
  int diff = abs(targetPosition - position);
  if(diff > 0) {
    setState(position < targetPosition ? 1 : 0);
    int newPosition = position;
    long start = millis();
    if(diff > 10) { //limit percent in one step
      diff = 10;
    }
    int transmit_repeats = (diff * COVER_OPEN_TO_CLOSE_TRANSMIT_REPEATS) / 100;

    rcSwitch->setRepeatTransmit(transmit_repeats);
    switch (state) {
        case 0:
            RGB.control(true);
            RGB.color(0, 0, 255);
            rcSwitch->send(downCode, 24);
            Serial.printf("Sending downCode: %d,repeats: %d, took: %d ms\n", downCode,transmit_repeats, millis() - start);
            RGB.control(false);
            newPosition -= diff;
            break;
        case 1:
            RGB.control(true);
            RGB.color(0, 0, 255);
            rcSwitch->send(upCode, 24);
            Serial.printf("Sending upCode: %d,repeats: %d, took: %d ms\n", upCode,transmit_repeats,millis() - start);
            RGB.control(false);
            newPosition += diff;
            break;
        case 2:
            break;
    }

    if(newPosition < 0) {
      newPosition = 0;
    }
    if(newPosition > 100) {
      newPosition = 100;
    }
    setPosition(newPosition);
  } else {
    setState(2);
    setPosition(targetPosition);
  }


}

void WindowsShutterAccessory::setTargetPosition (int oldValue, int newValue, HKConnection *sender) {
  targetPosition = newValue;
}

void WindowsShutterAccessory::initAccessorySet() {
    if(!rcSwitch) {
      rcSwitch = new RCSwitch();
      rcSwitch->enableTransmit(rcOutputPIN);
      rcSwitch->setProtocol(1);
    }
    EEPROM.get(this->eepromAddr, this->position);
    if(this->position < 0 ||this->position > 100) {
      this->position = 50;
    }
    this->targetPosition = this->position;

    Accessory *shutterAccessory = new Accessory();

    AccessorySet *accSet = &AccessorySet::getInstance();
    addInfoServiceToAccessory(shutterAccessory, "Shutter name", "Vendor name", "Model  name", "1","1.0.0", std::bind(&WindowsShutterAccessory::shutterIdentity, this, std::placeholders::_1, std::placeholders::_2,std::placeholders::_3));
    accSet->addAccessory(shutterAccessory);

    Service *windowsCoverService = new Service(serviceType_windowCover);
    shutterAccessory->addService(windowsCoverService);

    stringCharacteristics *nameCharacteristic = new stringCharacteristics(charType_serviceName, premission_read, 0);
    nameCharacteristic->characteristics::setValue("Window name");
    shutterAccessory->addCharacteristics(windowsCoverService, nameCharacteristic);

    intCharacteristics *targetPositionChar = new intCharacteristics(charType_targetPosition, premission_read|premission_write|premission_notify, 0, 100, 1, unit_percentage);
    targetPositionChar->characteristics::setValue(format("%d",targetPosition));
    targetPositionChar->valueChangeFunctionCall = std::bind(&WindowsShutterAccessory::setTargetPosition, this, std::placeholders::_1, std::placeholders::_2,std::placeholders::_3);
    shutterAccessory->addCharacteristics(windowsCoverService, targetPositionChar);

    currentPositionChar = new intCharacteristics(charType_currentPosition, premission_read|premission_notify, 0, 100, 1, unit_percentage);
    currentPositionChar->characteristics::setValue(format("%d",position));
    shutterAccessory->addCharacteristics(windowsCoverService, currentPositionChar);

    positionStateChar = new intCharacteristics(charType_positionState, premission_read|premission_notify, 0, 2, 1, unit_percentage);
    positionStateChar->characteristics::setValue(format("%d",state));
    shutterAccessory->addCharacteristics(windowsCoverService, positionStateChar);

};
