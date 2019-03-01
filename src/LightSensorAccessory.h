//
//  LightSensorAccessory.h
//  HKTester
//
//  Created by Lukas Jezny on 10/02/2019.
//  Copyright © 2019 Lukas Jezny. All rights reserved.
//

#ifndef LightSensorAccessory_hpp
#define LightSensorAccessory_hpp

#include "homekit/HKAccessory.h"
#include "HAPAccessoryDescriptor.h"

class LightSensorAccessory: public HAPAccessoryDescriptor {
private:
public:

    virtual void initAccessorySet();

    virtual int getDeviceType(){
        return deviceType_sensor;
    }
    virtual void handle();
};

#endif /* LightSensorAccessory_hpp */
