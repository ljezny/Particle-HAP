//
//  BatteryService.h
//  HKTester
//
//  Created by Lukas Jezny on 10/02/2019.
//  Copyright © 2019 Lukas Jezny. All rights reserved.
//

#ifndef BatteryService_hpp
#define BatteryService_hpp

#include "ServiceDescriptor.h"

class BatteryService : public ServiceDescriptor {
private:
    intCharacteristics *batteryLevelChar;
    intCharacteristics *batteryStatusLowChar;//0-normal,1-low
    intCharacteristics *batteryChargingStateChar;//0-not,1-charging,2-not chargable

    bool isCharging = false;
    int levelPercent = 100;

    void sensorIdentity(bool oldValue, bool newValue, HKConnection *sender);
    std::string getCurrentBatteryLevel (HKConnection *sender);
    std::string getStatusLow (HKConnection *sender);
    std::string getChargingState (HKConnection *sender);

    int chargingDPIN = D0;
    int chargingPINValue = HIGH;
    int batteryAPIN = A0;
    int batteryAPINMin = 0;
    int batteryAPINMax = 4095;
public:

    BatteryService(int chargingDPIN,int chargingPINValue,int batteryAPIN,int batteryAPINMin, int batteryAPINMax) {
        this->chargingDPIN = chargingDPIN;
        this->chargingPINValue = chargingPINValue;
        this->batteryAPIN = batteryAPIN;
        this->batteryAPINMin = batteryAPINMin;
        this->batteryAPINMax = batteryAPINMax;
    }

    virtual void initService(Accessory *accessory);

    virtual bool handle();
};

#endif /* BatteryService_hpp */
