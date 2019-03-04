//
//  MotionSensorAccessory.cpp
//  HKTester
//
//  Created by Lukas Jezny on 02/03/2019.
//  Copyright © 2019 Lukas Jezny. All rights reserved.
//

#include "MotionSensorAccessory.h"

#include "homekit/HKConnection.h"

#include <set>

#ifdef PARTICLE_COMPAT
#include "../example/HKTester/HKTester/Particle_Compat/particle_compat.h"
#else
#include <Particle.h>
#endif
#include "homekit/HKLog.h"



void MotionSensorAccessory::sensorIdentity(bool oldValue, bool newValue, HKConnection *sender) {
    
}

void MotionSensorAccessory::handle() {
    int v = digitalRead(motionInputPin);
    if(v != motionDetected) {
        motionDetected = v;
        
        motionDetectedChar->characteristics::setValue(motionDetected ? "true" : "false");
        motionDetectedChar->notify(NULL);//report value
    }
}

void MotionSensorAccessory::initAccessorySet() {
    pinMode(motionInputPin, INPUT);
    
    Accessory *motionSensorAccessory = new Accessory();
    
    AccessorySet *accSet = &AccessorySet::getInstance();
    addInfoServiceToAccessory(motionSensorAccessory, "Sensor name", "Vendor name", "Model  name", "1","1.0.0", std::bind(&MotionSensorAccessory::sensorIdentity, this, std::placeholders::_1, std::placeholders::_2,std::placeholders::_3));
    accSet->addAccessory(motionSensorAccessory);
    
    Service *motionSensorService = new Service(serviceType_motionSensor);
    motionSensorAccessory->addService(motionSensorService);
    
    stringCharacteristics *nameCharacteristic = new stringCharacteristics(charType_serviceName, premission_read, 0);
    nameCharacteristic->characteristics::setValue("Motion sensor");
    motionSensorAccessory->addCharacteristics(motionSensorService, nameCharacteristic);
    
    motionDetectedChar = new boolCharacteristics(charType_motionDetect, premission_read|premission_notify);
    motionDetectedChar->characteristics::setValue("true");
    motionSensorAccessory->addCharacteristics(motionSensorService, motionDetectedChar);
};
