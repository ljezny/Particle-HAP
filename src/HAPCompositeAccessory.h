//
//  CompositeAccessory.hpp
//  HKTester
//
//  Created by Lukas Jezny on 02/03/2019.
//  Copyright © 2019 Lukas Jezny. All rights reserved.
//

#ifndef HAPCompositeAccessory_hpp
#define HAPCompositeAccessory_hpp

#include <stdio.h>

#include "HKAccessory.h"
#include "HAPAccessoryDescriptor.h"
#include "HAPServiceDescriptor.h"
#include <vector>

class HAPCompositeAccessory: public HAPAccessoryDescriptor {
private:
  void identity(bool oldValue, bool newValue, HKConnection *sender);
public:
    std::vector<HAPServiceDescriptor*> descriptors;

    virtual void initAccessorySet();

    virtual int getDeviceType(){
        return deviceType_bridge;
    }
    virtual bool handle();
};

#endif /* HAPCompositeAccessory_hpp */
