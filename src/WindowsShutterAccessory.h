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
    
    intCharacteristics *positionStateChar;
    intCharacteristics *currentPositionChar;
    intCharacteristics *currentTiltAngleChar;
    
    //initially fully closed
    int state = 2; //0 - going to minimum value, closing, down , 1 - going to maximum light - opening, up
    int position = 50; //initially assume it's half open, so user can open/close to sync the state
    int tilt = -90;
    
    int targetTilt = tilt;
    int targetPosition = position;
    
    long endMS = LONG_MAX;
    
    void shutterIdentity(bool oldValue, bool newValue, HKConnection *sender);
    void setState(int newState);
    void setTilt(int newTilt);
    void setPosition(int newPosition);
    void setTargetPosition (int oldValue, int newValue, HKConnection *sender);
    void setTargetTiltAngle (int oldValue, int newValue, HKConnection *sender);
    
    RCSwitch rcSwitch;
public:
    WindowsShutterAccessory(int upCode,int downCode){
        this->upCode = upCode;
        this->downCode = downCode;
    }
    
    virtual void initAccessorySet();
    
    virtual int getDeviceType(){
        return deviceType_windowCover;
    }
    virtual void handle();
};

#endif /* WindowsShutterAccessory_hpp */
