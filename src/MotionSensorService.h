//
//  MotionSensorService.hpp
//  HKTester
//
//  Created by Lukas Jezny on 02/03/2019.
//  Copyright © 2019 Lukas Jezny. All rights reserved.
//

#ifndef MotionSensorService_hpp
#define MotionSensorService_hpp

#include <stdio.h>

#include "HKAccessory.h"
#include "ServiceDescriptor.h"

class MotionSensorService: public ServiceDescriptor {
private:
    boolCharacteristics *motionDetectedChar = NULL;
    int motionInputPin = D0;
    bool motionDetected = true;

    std::string getMotion (HKConnection *sender);
public:
    MotionSensorService(int pin) {
      this->motionInputPin = pin;
    }
    
    virtual void initService(Accessory *accessory);
    virtual bool handle();
};


#endif /* MotionSensorService_hpp */
