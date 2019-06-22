//
//  HomekitBridgeAccessory.hpp
//  HKTester
//
//  Created by Lukas Jezny on 02/03/2019.
//  Copyright © 2019 Lukas Jezny. All rights reserved.
//

#ifndef HAPHomekitBridgeAccessory_hpp
#define HAPHomekitBridgeAccessory_hpp

#include <stdio.h>

#include "HKAccessory.h"
#include "HAPAccessoryDescriptor.h"
#include <vector>

class HAPHomekitBridgeAccessory: public HAPAccessoryDescriptor {
private:
public:
    std::vector<HAPAccessoryDescriptor*> descriptors;

    virtual void initAccessorySet();

    virtual int getDeviceType(){
        return deviceType_bridge;
    }
    virtual bool handle();
};

#endif /* HAPHomekitBridgeAccessory_hpp */
