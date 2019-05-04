//
//  RemoteWeatherStationAccessory.h
//  HKTester
//
//  Created by Lukas Jezny on 10/02/2019.
//  Copyright © 2019 Lukas Jezny. All rights reserved.
//

#ifndef RemoteWeatherStationAccessory_hpp
#define RemoteWeatherStationAccessory_hpp

#include "homekit/HKAccessory.h"
#include "HAPAccessoryDescriptor.h"


class RemoteWeatherStationAccessory: public HAPAccessoryDescriptor {
private:
    floatCharacteristics *currentAmbilightChar;
    floatCharacteristics *currentTemperatureChar;
    floatCharacteristics *currentHumidityChar;
    intCharacteristics *batteryLevelChar;
    intCharacteristics *batteryStatusLowChar;//0-normal,1-low
    intCharacteristics *batteryChargingStateChar;//0-not,1-charging,2-not chargable

    float lux = 0.0001;
    float temperature = 0;
    float humidity = 50;
    
    bool batteryCharging = false;
    int batteryPercent = 100;

    int batteryLevelMin = 0;
    int batteryLevelMax = 4095;

    void sensorIdentity(bool oldValue, bool newValue, HKConnection *sender);
    std::string getCurrentAmbilightLevel (HKConnection *sender);
    std::string getCurrentTemperature (HKConnection *sender);
    std::string getCurrentHumidity (HKConnection *sender);

    std::string getCurrentBatteryLevel (HKConnection *sender);
    std::string getStatusLow (HKConnection *sender);
    std::string getChargingState (HKConnection *sender);

    void remoteEventHandler(const char *eventName, const char *data);
public:
    RemoteWeatherStationAccessory(){      
    }

    virtual void initAccessorySet();

    virtual int getDeviceType(){
        return deviceType_sensor;
    }
    virtual bool handle();
};

#endif /* RemoteWeatherStationAccessory_hpp */
