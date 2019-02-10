#pragma once

#include "homekit/HKAccessory.h"
#include "HAPAccessoryDescriptor.h"

class LightBulbAccessory: public HAPAccessoryDescriptor {
public:
    virtual void initAccessorySet();
    
    virtual int getDeviceType(){
        return deviceType_lightBulb;
    }
    virtual void handle(){
        
    }
};
