#pragma once

#include "homekit/HKAccessory.h"
#include "HAPAccessoryDescriptor.h"

class LightBulbAccessory: public HAPAccessoryDescriptor {
private:
    void powerTrackable (bool oldValue, bool newValue, HKConnection *sender);
    void brightTrackable (int oldValue, int newValue, HKConnection *sender);
    std::string getLedHue (HKConnection *sender);
    void setLedHue (int oldValue, int newValue, HKConnection *sender);
    std::string getLedBrightness (HKConnection *sender);
    void setLedBrightness (int oldValue, int newValue, HKConnection *sender);
    std::string getLedSaturation (HKConnection *sender);
    void setLedSaturation (int oldValue, int newValue, HKConnection *sender);
    void lightIdentify(bool oldValue, bool newValue, HKConnection *sender);
public:
    virtual void initAccessorySet();
    
    virtual int getDeviceType(){
        return deviceType_lightBulb;
    }
    virtual void handle(){
        
    }
};
