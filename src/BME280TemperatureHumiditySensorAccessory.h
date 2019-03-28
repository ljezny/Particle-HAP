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
#include "BME280/Adafruit_BME280.h"

class BME280TemperatureHumiditySensorAccessory: public HAPAccessoryDescriptor {
private:
    Adafruit_BME280 sensor = Adafruit_BME280();
    floatCharacteristics *currentTemperatureChar;
    floatCharacteristics *currentHumidityChar;
    long lastReportMS = 0;
    long REPORT_PERIOD_MS = 1000;
    float lastValueTemperature = 25;
    float lastValueHumidity = 50;
    void sensorIdentity(bool oldValue, bool newValue, HKConnection *sender);
    std::string getCurrentTemperature (HKConnection *sender);
    std::string getCurrentHumidity (HKConnection *sender);
public:

    virtual void initAccessorySet();

    virtual int getDeviceType(){
        return deviceType_sensor;
    }
    virtual bool handle();
};

#endif /* BME280TemperatureHumiditySensorAccessory_hpp */
