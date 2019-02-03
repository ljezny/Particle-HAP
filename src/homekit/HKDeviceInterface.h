#pragma once
//
//  HKDeviceInterface.h
//  Workbench
//
//  Created by Wai Man Chan on 11/3/14.
//
//

#include <stdio.h>

#ifdef PARTICLE_COMPAT
#include "../../example/HKTester/HKTester/Particle_Compat/particle_compat.h"
#else
#include <Particle.h>
#endif

void setupPort();
void startIdentify();
void setLightStrength(int strengthLevel);
//You need to implement this to use Fan+Light configuration
void setFanSpeed(int strengthLevel);
