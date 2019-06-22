#ifndef JeznyHomeStationBridge_hpp
#define JeznyHomeStationBridge_hpp

#include <stdio.h>

#include "HKAccessory.h"
#include "HAPAccessoryDescriptor.h"
#include "HAPHomekitBridgeAccessory.h"
#include "WindowsShutterService.h"
#include "HAPCompositeAccessory.h"
#include "RFRelaySwitchService.h"

class JeznyHomeStationBridge: public HAPHomekitBridgeAccessory {
private:
    HAPCompositeAccessory *windowShuttersAndSwitches = new HAPCompositeAccessory();
    
public:
    JeznyHomeStationBridge() {
        windowShuttersAndSwitches->descriptors.push_back(new WindowsShutterService(14678913,14678916,1 * sizeof(int)));
        windowShuttersAndSwitches->descriptors.push_back(new WindowsShutterService(4102033,4102036,2 * sizeof(int)));
        windowShuttersAndSwitches->descriptors.push_back(new WindowsShutterService(4102034,4102040,3 * sizeof(int)));
        windowShuttersAndSwitches->descriptors.push_back(new RFRelaySwitchService(D6, 4102038));
        windowShuttersAndSwitches->descriptors.push_back(new RFRelaySwitchService(D6, 4102039));
        
        this->descriptors.push_back(windowShuttersAndSwitches);
    }    
};

#endif /* JeznyHomeStationBridge_hpp */
