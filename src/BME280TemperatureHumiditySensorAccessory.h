//
//  BME280TemperatureHumiditySensorAccessory.h
//  HKTester
//
//  Created by Lukas Jezny on 10/02/2019.
//  Copyright © 2019 Lukas Jezny. All rights reserved.
//

#ifndef BME280TemperatureHumiditySensorAccessory_hpp
#define BME280TemperatureHumiditySensorAccessory_hpp

#include "homekit/HKAccessory.h"
#include "HAPAccessoryDescriptor.h"

class BME280TemperatureHumiditySensorAccessory: public HAPAccessoryDescriptor {
private:
    floatCharacteristics *currentAmbilightChar;
    int photosensor = A0;
    long lastReportMS = 0;
    long REPORT_PERIOD_MS = 5000;
    void sensorIdentity(bool oldValue, bool newValue, HKConnection *sender);
    std::string getCurrentAmbilightLevel (HKConnection *sender);
public:

    virtual void initAccessorySet();

    virtual int getDeviceType(){
        return deviceType_sensor;
    }
    virtual bool handle();
};

#endif /* BME280TemperatureHumiditySensorAccessory_hpp */
