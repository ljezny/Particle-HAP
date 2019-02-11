//
//  WindowsShutterAccessory.h
//  HKTester
//
//  Created by Lukas Jezny on 10/02/2019.
//  Copyright © 2019 Lukas Jezny. All rights reserved.
//

#ifndef WindowsShutterAccessory_hpp
#define WindowsShutterAccessory_hpp

#include "homekit/HKAccessory.h"
#include "HAPAccessoryDescriptor.h"

class WindowsShutterAccessory: public HAPAccessoryDescriptor {
private:
public:
    
    virtual void initAccessorySet();
    
    virtual int getDeviceType(){
        return deviceType_windowCover;
    }
    virtual void handle();
};

#endif /* WindowsShutterAccessory_hpp */
