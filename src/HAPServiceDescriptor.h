//
//  HAPServiceDescriptor.hpp
//  HKTester
//
//  Created by Lukas Jezny on 10/02/2019.
//  Copyright © 2019 Lukas Jezny. All rights reserved.
//

#ifndef HAPServiceDescriptor_hpp
#define HAPServiceDescriptor_hpp

#include <stdio.h>
#include "HKAccessory.h"

class HAPServiceDescriptor {
private:
public:
    virtual void initService(Accessory *accessory) = 0;
    virtual bool handle() = 0;
};

#endif /* HAPServiceDescriptor_hpp */
