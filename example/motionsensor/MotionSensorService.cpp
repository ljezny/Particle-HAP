//
//  MotionSensorService.cpp
//  HKTester
//
//  Created by Lukas Jezny on 02/03/2019.
//  Copyright © 2019 Lukas Jezny. All rights reserved.
//

#include "MotionSensorService.h"

#include "homekit/HKConnection.h"

#include <set>

#ifdef PARTICLE_COMPAT
#include "../example/HKTester/HKTester/Particle_Compat/particle_compat.h"
#else
#include <Particle.h>
#endif
#include "homekit/HKLog.h"

std::string MotionSensorService::getMotion (HKConnection *sender){
  return motionDetected ? "true" : "false";
}

bool MotionSensorService::handle() {
    int v = digitalRead(motionInputPin);
    if(v != motionDetected) {
        motionDetected = v;
        motionDetectedChar->notify(NULL);//report value
        return true;
    }
    return false;
}

void MotionSensorService::initService(Accessory *accessory) {
    pinMode(motionInputPin, INPUT);

    Service *motionSensorService = new Service(serviceType_motionSensor);
    accessory->addService(motionSensorService);

    stringCharacteristics *nameCharacteristic = new stringCharacteristics(charType_serviceName, premission_read, 0);
    nameCharacteristic->characteristics::setValue("Motion sensor");
    accessory->addCharacteristics(motionSensorService, nameCharacteristic);

    motionDetectedChar = new boolCharacteristics(charType_motionDetect, premission_read|premission_notify);
    motionDetectedChar->perUserQuery = std::bind(&MotionSensorService::getMotion, this, std::placeholders::_1);
    accessory->addCharacteristics(motionSensorService, motionDetectedChar);
};
