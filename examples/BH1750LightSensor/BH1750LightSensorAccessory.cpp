//
//  BH1750LightSensorAccessory.cpp
//  HKTester
//
//  Created by Lukas Jezny on 10/02/2019.
//  Copyright © 2019 Lukas Jezny. All rights reserved.
//

#include "BH1750LightSensorAccessory.h"

#include "HKConnection.h"

#include <set>

#include <Particle.h>
#include "HKLog.h"


void BH1750LightSensorAccessory::sensorIdentity(bool oldValue, bool newValue, HKConnection *sender) {

}

std::string BH1750LightSensorAccessory::getCurrentAmbilightLevel (HKConnection *sender) {
  return format("%d",((int)lastValueLux) + 1);
}


bool BH1750LightSensorAccessory::handle() {
    bool result = false;
    if((lastReportMS + REPORT_PERIOD_MS) < millis()) { //expired, stop
        lastReportMS = millis();

        int v = (float) lightMeter.readLightLevel();
        if(v < 0.0001){
          v = 0.0001;
        }
        if(v > 100000.0) {
          v = 100000.0;
        }
        lastValueLux = v;

        if(currentAmbilightChar!=NULL) {
          currentAmbilightChar->notify(NULL);
        }

        result = true;
    }

    return result;
}

void BH1750LightSensorAccessory::initAccessorySet() {
    lightMeter.begin();

    Accessory *BH1750LightSensorAccessory = new Accessory();

    AccessorySet *accSet = &AccessorySet::getInstance();
    addInfoServiceToAccessory(BH1750LightSensorAccessory, "Sensor name", "Vendor name", "Model  name", "1","1.0.0", std::bind(&BH1750LightSensorAccessory::sensorIdentity, this, std::placeholders::_1, std::placeholders::_2,std::placeholders::_3));
    accSet->addAccessory(BH1750LightSensorAccessory);

    Service *lightSensorService = new Service(serviceType_lightSensor);
    BH1750LightSensorAccessory->addService(lightSensorService);

    stringCharacteristics *nameCharacteristic = new stringCharacteristics(charType_serviceName, permission_read, 0);
    nameCharacteristic->characteristics::setValue("Light sensor");
    BH1750LightSensorAccessory->addCharacteristics(lightSensorService, nameCharacteristic);

    boolCharacteristics *statusActive = new boolCharacteristics(charType_sensorActive, permission_read|permission_notify);
    statusActive->characteristics::setValue("true");
    BH1750LightSensorAccessory->addCharacteristics(lightSensorService, statusActive);

    currentAmbilightChar = new floatCharacteristics(charType_currentAmbientLightLevel, permission_read|permission_notify, 0.0001, 100000.0, 0, unit_lux);
    currentAmbilightChar->perUserQuery = std::bind(&BH1750LightSensorAccessory::getCurrentAmbilightLevel, this, std::placeholders::_1);
    BH1750LightSensorAccessory->addCharacteristics(lightSensorService, currentAmbilightChar);

};
