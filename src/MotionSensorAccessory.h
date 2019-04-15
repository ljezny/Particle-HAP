//
//  MotionSensorAccessory.hpp
//  HKTester
//
//  Created by Lukas Jezny on 02/03/2019.
//  Copyright © 2019 Lukas Jezny. All rights reserved.
//

#ifndef MotionSensorAccessory_hpp
#define MotionSensorAccessory_hpp

#include <stdio.h>

#include "homekit/HKAccessory.h"
#include "HAPAccessoryDescriptor.h"

class MotionSensorAccessory: public HAPAccessoryDescriptor {
private:
    boolCharacteristics *motionDetectedChar = NULL;
    int motionInputPin = D0;
    void sensorIdentity(bool oldValue, bool newValue, HKConnection *sender);
    bool motionDetected = true;

    std::string getMotion (HKConnection *sender);
public:
    MotionSensorAccessory(int pin) {
      this->motionInputPin = pin;
    }
    virtual void initAccessorySet();

    virtual int getDeviceType(){
        return deviceType_sensor;
    }
    virtual bool handle();
};


#endif /* MotionSensorAccessory_hpp */
