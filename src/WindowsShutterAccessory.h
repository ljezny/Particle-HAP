//
//  WindowsShutterAccessory.h
//  HKTester
//
//  Created by Lukas Jezny on 10/02/2019.
//  Copyright © 2019 Lukas Jezny. All rights reserved.
//

#ifndef WindowsShutterAccessory_hpp
#define WindowsShutterAccessory_hpp

#include "homekit/HKAccessory.h"
#include "HAPAccessoryDescriptor.h"
#include "rcswitch/RCSwitch.h"
class WindowsShutterAccessory: public HAPAccessoryDescriptor {
private:
    int rcOutputPIN = D6;
    int upCode = 0;
    int downCode = 0;
    int eepromAddr = 0;

    intCharacteristics *positionStateChar;
    intCharacteristics *currentPositionChar;
    intCharacteristics *currentTiltAngleChar;

    //initially fully closed
    int state = 2; //0 - going to minimum value, closing, down , 1 - going to maximum light - opening, up
    int position = 50; //initially assume it's half open, so user can open/close to sync the state

    int targetPosition = position;

    long endMS = LONG_MAX;

    void shutterIdentity(bool oldValue, bool newValue, HKConnection *sender);
    void setState(int newState);
    void setPosition(int newPosition);
    void setTargetPosition (int oldValue, int newValue, HKConnection *sender);


public:
    WindowsShutterAccessory(int upCode,int downCode,int eepromAddr){
        this->upCode = upCode;
        this->downCode = downCode;
        this->eepromAddr = eepromAddr;
    }

    virtual void initAccessorySet();

    virtual int getDeviceType(){
        return deviceType_windowCover;
    }
    virtual bool handle();
};

#endif /* WindowsShutterAccessory_hpp */
