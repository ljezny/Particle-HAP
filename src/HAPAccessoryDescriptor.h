//
//  Accessory.hpp
//  HKTester
//
//  Created by Lukas Jezny on 10/02/2019.
//  Copyright © 2019 Lukas Jezny. All rights reserved.
//

#ifndef Accessory_hpp
#define Accessory_hpp

#include <stdio.h>

class HAPAccessoryDescriptor {
private:
public:
    virtual void initAccessorySet() = 0;
    virtual int getDeviceType() = 0;
    virtual bool handle() = 0;
};

#endif /* Accessory_hpp */
