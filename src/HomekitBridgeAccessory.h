//
//  HomekitBridgeAccessory.hpp
//  HKTester
//
//  Created by Lukas Jezny on 02/03/2019.
//  Copyright © 2019 Lukas Jezny. All rights reserved.
//

#ifndef HomekitBridgeAccessory_hpp
#define HomekitBridgeAccessory_hpp

#include <stdio.h>

#include "homekit/HKAccessory.h"
#include "HAPAccessoryDescriptor.h"
#include <vector>

class HomekitBridgeAccessory: public HAPAccessoryDescriptor {
private:
public:
    std::vector<HAPAccessoryDescriptor*> descriptors;

    virtual void initAccessorySet();

    virtual int getDeviceType(){
        return deviceType_bridge;
    }
    virtual bool handle();
};

#endif /* HomekitBridgeAccessory_hpp */
