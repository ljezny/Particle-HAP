//
//  PhotoSystemLEDLightBulbAccessory.cpp
//  HKTester
//
//  Created by Lukas Jezny on 13/03/2019.
//  Copyright © 2019 Lukas Jezny. All rights reserved.
//

#include "PhotoSystemLEDLightBulbAccessory.h"

void PhotonSystemLEDLightBulbAccessory::updateColor(RgbColor color) {
    RGB.control(true);
    RGB.color(color.r, color.g, color.b);
}
