//
//  CompositeAccessory.hpp
//  HKTester
//
//  Created by Lukas Jezny on 02/03/2019.
//  Copyright © 2019 Lukas Jezny. All rights reserved.
//

#ifndef CompositeAccessory_hpp
#define CompositeAccessory_hpp

#include <stdio.h>

#include "HKAccessory.h"
#include "HAPAccessoryDescriptor.h"
#include "ServiceDescriptor.h"
#include <vector>

class CompositeAccessory: public HAPAccessoryDescriptor {
private:
  void identity(bool oldValue, bool newValue, HKConnection *sender);
public:
    std::vector<ServiceDescriptor*> descriptors;

    virtual void initAccessorySet();

    virtual int getDeviceType(){
        return deviceType_bridge;
    }
    virtual bool handle();
};

#endif /* CompositeAccessory_hpp */
