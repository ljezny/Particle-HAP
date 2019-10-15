//
//  LightSensorAccessory.cpp
//  HKTester
//
//  Created by Lukas Jezny on 10/02/2019.
//  Copyright © 2019 Lukas Jezny. All rights reserved.
//

#include "LightSensorAccessory.h"

#include "HKConnection.h"

#include <set>

#include <Particle.h>
#include "HKLog.h"



void LightSensorAccessory::sensorIdentity(bool oldValue, bool newValue, HKConnection *sender) {

}

std::string LightSensorAccessory::getCurrentAmbilightLevel (HKConnection *sender) {
    int v = analogRead(photosensor); // read the input pin 0 - 4095
    return format("%d",v + 1);
}


bool LightSensorAccessory::handle() {
    if((lastReportMS + 5000) < millis()) { //expired, stop
        lastReportMS = millis();
        currentAmbilightChar->notify(NULL);
        return true;
    }
    return false;
    //hkLog.info("photosensor: %d\n", analogRead(photosensor));
}

void LightSensorAccessory::initAccessorySet() {
    Accessory *lightSensorAccessory = new Accessory();

    AccessorySet *accSet = &AccessorySet::getInstance();
    addInfoServiceToAccessory(lightSensorAccessory, "Sensor name", "Vendor name", "Model  name", "1","1.0.0", std::bind(&LightSensorAccessory::sensorIdentity, this, std::placeholders::_1, std::placeholders::_2,std::placeholders::_3));
    accSet->addAccessory(lightSensorAccessory);

    Service *lightSensorService = new Service(serviceType_lightSensor);
    lightSensorAccessory->addService(lightSensorService);

    stringCharacteristics *nameCharacteristic = new stringCharacteristics(charType_serviceName, permission_read, 0);
    nameCharacteristic->characteristics::setValue("Light sensor");
    lightSensorAccessory->addCharacteristics(lightSensorService, nameCharacteristic);

    boolCharacteristics *statusActive = new boolCharacteristics(charType_sensorActive, permission_read|permission_notify);
    statusActive->characteristics::setValue("true");
    lightSensorAccessory->addCharacteristics(lightSensorService, statusActive);

    currentAmbilightChar = new floatCharacteristics(charType_currentAmbientLightLevel, permission_read|permission_notify, 0.0001, 100000.0, 0, unit_lux);
    currentAmbilightChar->perUserQuery = std::bind(&LightSensorAccessory::getCurrentAmbilightLevel, this, std::placeholders::_1);
    lightSensorAccessory->addCharacteristics(lightSensorService, currentAmbilightChar);
};
