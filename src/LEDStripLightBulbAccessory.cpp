//
//  LEDStripLightBulbAccessory.cpp
//  HKTester
//
//  Created by Lukas Jezny on 13/03/2019.
//  Copyright © 2019 Lukas Jezny. All rights reserved.
//

#include "LEDStripLightBulbAccessory.h"

void LEDStripLightBulbAccessory::updateColor(RgbColor color) {
    analogWrite(redPIN,color.r);
    analogWrite(greenPIN,color.g);
    analogWrite(bluePIN,color.b);
}

void LEDStripLightBulbAccessory::initAccessorySet() {
  pinMode(redPIN, OUTPUT);
  pinMode(greenPIN, OUTPUT);
  pinMode(bluePIN, OUTPUT);

  LightBulbAccessoryBase::initAccessorySet();
}
