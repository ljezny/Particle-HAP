//
//  BME280TemperatureHumiditySensorAccessory.cpp
//  HKTester
//
//  Created by Lukas Jezny on 10/02/2019.
//  Copyright © 2019 Lukas Jezny. All rights reserved.
//

#include "BME280TemperatureHumiditySensorAccessory.h"

#include "homekit/HKConnection.h"

#include <set>

#ifdef PARTICLE_COMPAT
#include "../example/HKTester/HKTester/Particle_Compat/particle_compat.h"
#else
#include <Particle.h>
#endif
#include "homekit/HKLog.h"

void BME280TemperatureHumiditySensorAccessory::sensorIdentity(bool oldValue, bool newValue, HKConnection *sender) {

}

std::string BME280TemperatureHumiditySensorAccessory::getCurrentAmbilightLevel (HKConnection *sender) {
    int v = analogRead(photosensor); // read the input pin 0 - 4095
    return format("%d",v + 1);
}


bool BME280TemperatureHumiditySensorAccessory::handle() {
    if((lastReportMS + 5000) < millis()) { //expired, stop
        lastReportMS = millis();
        currentAmbilightChar->notify(NULL);
        return true;
    }
    return false;
    //hkLog.info("photosensor: %d\n", analogRead(photosensor));
}

void BME280TemperatureHumiditySensorAccessory::initAccessorySet() {
    Accessory *BME280TemperatureHumiditySensorAccessory = new Accessory();

    AccessorySet *accSet = &AccessorySet::getInstance();
    addInfoServiceToAccessory(BME280TemperatureHumiditySensorAccessory, "Sensor name", "Vendor name", "Model  name", "1","1.0.0", std::bind(&BME280TemperatureHumiditySensorAccessory::sensorIdentity, this, std::placeholders::_1, std::placeholders::_2,std::placeholders::_3));
    accSet->addAccessory(BME280TemperatureHumiditySensorAccessory);

    Service *lightSensorService = new Service(serviceType_lightSensor);
    BME280TemperatureHumiditySensorAccessory->addService(lightSensorService);

    stringCharacteristics *nameCharacteristic = new stringCharacteristics(charType_serviceName, premission_read, 0);
    nameCharacteristic->characteristics::setValue("Light sensor");
    BME280TemperatureHumiditySensorAccessory->addCharacteristics(lightSensorService, nameCharacteristic);

    boolCharacteristics *statusActive = new boolCharacteristics(charType_sensorActive, premission_read|premission_notify);
    statusActive->characteristics::setValue("true");
    BME280TemperatureHumiditySensorAccessory->addCharacteristics(lightSensorService, statusActive);

    currentAmbilightChar = new floatCharacteristics(charType_currentAmbientLightLevel, premission_read|premission_notify, 0.0001, 100000.0, 0, unit_lux);
    currentAmbilightChar->perUserQuery = std::bind(&BME280TemperatureHumiditySensorAccessory::getCurrentAmbilightLevel, this, std::placeholders::_1);
    BME280TemperatureHumiditySensorAccessory->addCharacteristics(lightSensorService, currentAmbilightChar);
};
