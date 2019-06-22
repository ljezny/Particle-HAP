//
//  LEDStripLightBulbAccessory.hpp
//  HKTester
//
//  Created by Lukas Jezny on 13/03/2019.
//  Copyright © 2019 Lukas Jezny. All rights reserved.
//

#ifndef LEDStripLightBulbAccessory_hpp
#define LEDStripLightBulbAccessory_hpp

#include <stdio.h>

#include "LightBulbAccessoryBase.h"
#include <stdio.h>

class LEDStripLightBulbAccessory: public LightBulbAccessoryBase {
private:
    int redPIN,bluePIN,greenPIN;
public:
    virtual void initAccessorySet();
    virtual void updateColor(RgbColor color);
    LEDStripLightBulbAccessory(int redPIN,int bluePIN,int greenPIN) {
        this->redPIN = redPIN;
        this->bluePIN = bluePIN;
        this->greenPIN = greenPIN;
    }
};


#endif /* LEDStripLightBulbAccessory_hpp */
