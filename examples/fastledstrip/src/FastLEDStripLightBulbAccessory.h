//
//  LEDStripLightBulbAccessory.hpp
//  HKTester
//
//  Created by Lukas Jezny on 13/03/2019.
//  Copyright © 2019 Lukas Jezny. All rights reserved.
//

#ifndef FastLEDStripLightBulbAccessory_hpp
#define FastLEDStripLightBulbAccessory_hpp

#include <stdio.h>

#include "LightBulbAccessoryBase.h"
#include <stdio.h>

#define NUM_LEDS 61 * 4
#define FASTLED_PIN 0

#define BRIGHTNESS 128

class FastLEDStripLightBulbAccessory : public LightBulbAccessoryBase
{
private:
    int dataPin;

public:
    virtual void initAccessorySet();
    virtual void updateColor(RgbColor color);
    FastLEDStripLightBulbAccessory(int dataPin)
    {
        this->dataPin = dataPin;
    }
};

#endif /* FastLEDStripLightBulbAccessory_hpp */
