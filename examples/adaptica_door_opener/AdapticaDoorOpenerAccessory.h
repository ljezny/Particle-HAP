//
//  AdapticaDoorOpenerAccessory.hpp
//  HKTester
//
//  Created by Lukas Jezny on 02/03/2019.
//  Copyright © 2019 Lukas Jezny. All rights reserved.
//

#ifndef AdapticaDoorOpenerAccessory_hpp
#define AdapticaDoorOpenerAccessory_hpp

#include <stdio.h>

#include "HKAccessory.h"
#include "HAPAccessoryDescriptor.h"
#include "HAPServiceDescriptor.h"
#include <vector>
#include "SerialModemSwitchService.h"
#include "HAPCompositeAccessory.h"

class AdapticaDoorOpenerAccessory: public HAPCompositeAccessory {
private:
    SerialModemSwitchService *gateService = new SerialModemSwitchService("ATD,42,,,T44,,;\r");
    SerialModemSwitchService *doorService = new SerialModemSwitchService("ATD,42,,,T55,,;\r");
    
    void identity(bool oldValue, bool newValue, HKConnection *sender);
public:
    AdapticaDoorOpenerAccessory() {
        this->descriptors.push_back(gateService);
        this->descriptors.push_back(doorService);
    }

    virtual bool handle() {
        bool b = HAPCompositeAccessory::handle();

        return b;
    }
};

#endif /* AdapticaDoorOpenerAccessory_hpp */
