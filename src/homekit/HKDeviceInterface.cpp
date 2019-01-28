//
//  HKDeviceInterface.c
//  Workbench
//
//  Created by Wai Man Chan on 11/3/14.
//
//

#include "HKDeviceInterface.h"
#include <stdio.h>
#include <Particle.h>



void setupPort() {
  Serial.println("setupPort");
}

void startIdentify() {
  Serial.println("startIdentify");
}

void setLightStrength(int strengthLevel) {
  Serial.println("setLightStrength");
}

void setFanSpeed(int strengthLevel) {
  Serial.println("setFanSpeed");
}
