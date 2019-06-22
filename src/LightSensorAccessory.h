//
//  LightSensorAccessory.h
//  HKTester
//
//  Created by Lukas Jezny on 10/02/2019.
//  Copyright © 2019 Lukas Jezny. All rights reserved.
//

#ifndef LightSensorAccessory_hpp
#define LightSensorAccessory_hpp

#include "HKAccessory.h"
#include "HAPAccessoryDescriptor.h"

class LightSensorAccessory: public HAPAccessoryDescriptor {
private:
    floatCharacteristics *currentAmbilightChar;
    int photosensor = A0;
    long lastReportMS = 0;
    void sensorIdentity(bool oldValue, bool newValue, HKConnection *sender);
    std::string getCurrentAmbilightLevel (HKConnection *sender);
public:

    virtual void initAccessorySet();

    virtual int getDeviceType(){
        return deviceType_sensor;
    }
    virtual bool handle();
};

#endif /* LightSensorAccessory_hpp */
