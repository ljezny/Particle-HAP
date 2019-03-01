//
//  LightSensorAccessory.cpp
//  HKTester
//
//  Created by Lukas Jezny on 10/02/2019.
//  Copyright © 2019 Lukas Jezny. All rights reserved.
//

#include "LightSensorAccessory.h"

#include "homekit/HKConnection.h"

#include <set>

#ifdef PARTICLE_COMPAT
#include "../example/HKTester/HKTester/Particle_Compat/particle_compat.h"
#else
#include <Particle.h>
#endif
#include "homekit/HKLog.h"

floatCharacteristics *currentAmbilightChar;

int photosensor = A0;

void sensorIdentity(bool oldValue, bool newValue, HKConnection *sender) {

}

std::string getCurrentAmbilightLevel (HKConnection *sender) {
    int v = analogRead(photosensor); // read the input pin 0 - 4095
    return format("%d",v + 1);
}

long lastReportMS = 0;

void LightSensorAccessory::handle() {
    if((lastReportMS + 5000) < millis()) { //expired, stop
        lastReportMS = millis();
        currentAmbilightChar->notify(NULL);
    }
    //Serial.printf("photosensor: %d\n", analogRead(photosensor));
}

void LightSensorAccessory::initAccessorySet() {
    Accessory *lightSensorAccessory = new Accessory();

    AccessorySet *accSet = &AccessorySet::getInstance();
    addInfoServiceToAccessory(lightSensorAccessory, "Sensor name", "Vendor name", "Model  name", "1","1.0.0", &sensorIdentity);
    accSet->addAccessory(lightSensorAccessory);

    Service *lightSensorService = new Service(serviceType_lightSensor);
    lightSensorAccessory->addService(lightSensorService);

    stringCharacteristics *nameCharacteristic = new stringCharacteristics(charType_serviceName, premission_read, 0);
    nameCharacteristic->characteristics::setValue("Window name");
    lightSensorAccessory->addCharacteristics(lightSensorService, nameCharacteristic);

    boolCharacteristics *statusActive = new boolCharacteristics(charType_sensorActive, premission_read|premission_notify);
    statusActive->characteristics::setValue("true");
    lightSensorAccessory->addCharacteristics(lightSensorService, statusActive);

    currentAmbilightChar = new floatCharacteristics(charType_currentAmbientLightLevel, premission_read|premission_notify, 0.0001, 100000.0, 0, unit_lux);
    currentAmbilightChar->perUserQuery = &getCurrentAmbilightLevel;
    lightSensorAccessory->addCharacteristics(lightSensorService, currentAmbilightChar);
};
