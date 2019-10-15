//
//  WindowsShutterAccessory.cpp
//  HKTester
//
//  Created by Lukas Jezny on 10/02/2019.
//  Copyright © 2019 Lukas Jezny. All rights reserved.
//

#include "WindowsShutterService.h"

#include "HKConnection.h"

#include <set>


#include <Particle.h>
#include "HKLog.h"

#define COVER_OPEN_TO_CLOSE_MS 56000
#define COVER_OPEN_TO_CLOSE_TRANSMIT_REPEATS 1400 //100 repeat tooked 4500ms
#define TILT_OPEN_TO_CLOSE_TRANSMIT_REPEATS 40 //100 repeat tooked 4500ms

LEDStatus RGB_STATUS_BLUE(RGB_COLOR_BLUE, LED_PATTERN_SOLID, LED_SPEED_NORMAL, LED_PRIORITY_IMPORTANT);

void WindowsShutterService::setState(int newState) {
    if(state != newState) {
      state = newState;
      positionStateChar->characteristics::setValue(format("%d",state)); //report state
      positionStateChar->notify(NULL);

      setPosition(position);
    }
}

void WindowsShutterService::setPosition(int newPosition) {
    if(newPosition != position) {
      position = newPosition;
      currentPositionChar->characteristics::setValue(format("%d",position));//report position
      currentPositionChar->notify(NULL);

      EEPROM.put(this->eepromAddr, this->position);
    }
}

bool WindowsShutterService::handle() {
  bool result = false;
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
            RGB_STATUS_BLUE.setActive(true);
            rcSwitch->send(downCode, 24);
            hkLog.info("Sending downCode: %d,repeats: %d, took: %d ms\n", downCode,transmit_repeats, millis() - start);
            RGB_STATUS_BLUE.setActive(false);
            newPosition -= diff;
            result = true;
            break;
        case 1:
            RGB_STATUS_BLUE.setActive(true);
            rcSwitch->send(upCode, 24);
            hkLog.info("Sending upCode: %d,repeats: %d, took: %d ms\n", upCode,transmit_repeats,millis() - start);
            RGB_STATUS_BLUE.setActive(false);
            newPosition += diff;
            result = true;
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
    int old_state = state;
    setState(2);
    setPosition(targetPosition);

    if(old_state == 0 && targetPosition > 0) { //send one more code up to shutters remain in open position
      delay(200);
      rcSwitch->setRepeatTransmit(TILT_OPEN_TO_CLOSE_TRANSMIT_REPEATS);
      rcSwitch->send(upCode, 24);
      result = true;
    } else if(old_state == 0 && targetPosition == 0) { //just make sure to really close the shutters
      rcSwitch->setRepeatTransmit(COVER_OPEN_TO_CLOSE_TRANSMIT_REPEATS / 8);
      rcSwitch->send(downCode, 24);
    }
  }
  return false;
}

void WindowsShutterService::setTargetPosition (int oldValue, int newValue, HKConnection *sender) {
  targetPosition = newValue;
}

void WindowsShutterService::initService(Accessory *accessory) {
    rcSwitch = new RCSwitch();
    rcSwitch->enableTransmit(rcOutputPIN);
    rcSwitch->setProtocol(1);

    EEPROM.get(this->eepromAddr, this->position);
    if(this->position < 0 ||this->position > 100) {
      this->position = 50;
    }
    this->targetPosition = this->position;

    Service *windowsCoverService = new Service(serviceType_windowCover);
    accessory->addService(windowsCoverService);

    stringCharacteristics *nameCharacteristic = new stringCharacteristics(charType_serviceName, permission_read, 0);
    nameCharacteristic->characteristics::setValue("Window name");
    accessory->addCharacteristics(windowsCoverService, nameCharacteristic);

    intCharacteristics *targetPositionChar = new intCharacteristics(charType_targetPosition, permission_read|permission_write|permission_notify, 0, 100, 1, unit_percentage);
    targetPositionChar->characteristics::setValue(format("%d",targetPosition));
    targetPositionChar->valueChangeFunctionCall = std::bind(&WindowsShutterService::setTargetPosition, this, std::placeholders::_1, std::placeholders::_2,std::placeholders::_3);
    accessory->addCharacteristics(windowsCoverService, targetPositionChar);

    currentPositionChar = new intCharacteristics(charType_currentPosition, permission_read|permission_notify, 0, 100, 1, unit_percentage);
    currentPositionChar->characteristics::setValue(format("%d",position));
    accessory->addCharacteristics(windowsCoverService, currentPositionChar);

    positionStateChar = new intCharacteristics(charType_positionState, permission_read|permission_notify, 0, 2, 1, unit_percentage);
    positionStateChar->characteristics::setValue(format("%d",state));
    accessory->addCharacteristics(windowsCoverService, positionStateChar);

};
