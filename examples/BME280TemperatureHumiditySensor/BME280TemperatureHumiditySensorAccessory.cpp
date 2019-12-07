//
//  BME280TemperatureHumiditySensorAccessory.cpp
//  HKTester
//
//  Created by Lukas Jezny on 10/02/2019.
//  Copyright © 2019 Lukas Jezny. All rights reserved.
//

#include "BME280TemperatureHumiditySensorAccessory.h"

#include "HKConnection.h"

#include <set>

#include <Particle.h>
#include "HKLog.h"

void BME280TemperatureHumiditySensorAccessory::sensorIdentity(bool oldValue, bool newValue, HKConnection *sender) {

}


std::string BME280TemperatureHumiditySensorAccessory::getCurrentTemperature (HKConnection *sender) {
  return format("%.1f",lastValueTemperature);
}

std::string BME280TemperatureHumiditySensorAccessory::getCurrentHumidity (HKConnection *sender){
  return format("%.1f", lastValueHumidity);
}

bool BME280TemperatureHumiditySensorAccessory::handle() {
  bool result = false;
  if((lastReportMS + REPORT_PERIOD_MS) < millis()) {
      lastReportMS = millis();

      float t = sensor.readTemperature();
      if(t < -50){
        t = -50;
      }
      if(t > 50) {
        t = 50;
      }
      lastValueTemperature = t;

      float h = sensor.readHumidity();
      if(h < 0){
        h = 0;
      }
      if(h > 100) {
        h = 100;
      }
      lastValueHumidity = h;

      if(currentTemperatureChar) currentTemperatureChar->notify(NULL);
      if(currentHumidityChar) currentHumidityChar->notify(NULL);

      Particle.publish("bme280/temperature", String(lastValueTemperature), PRIVATE);
      Particle.publish("bme280/humidity", String(lastValueHumidity), PRIVATE);

      result = true;
  }
  return result;
}

void BME280TemperatureHumiditySensorAccessory::initAccessorySet() {
    sensor.begin(0x76);

    Accessory *sensorAccessory = new Accessory();

    AccessorySet *accSet = &AccessorySet::getInstance();

    addInfoServiceToAccessory(sensorAccessory, "Sensor name", "Vendor name", "Model  name", "1","1.0.0", std::bind(&BME280TemperatureHumiditySensorAccessory::sensorIdentity, this, std::placeholders::_1, std::placeholders::_2,std::placeholders::_3));
    accSet->addAccessory(sensorAccessory);

    Service *temperatureSensorService = new Service(serviceType_temperatureSensor);
    sensorAccessory->addService(temperatureSensorService);

    stringCharacteristics *temperatureNameCharacteristic = new stringCharacteristics(charType_serviceName, permission_read, 0);
    temperatureNameCharacteristic->characteristics::setValue("Temperature sensor");
    sensorAccessory->addCharacteristics(temperatureSensorService, temperatureNameCharacteristic);

    currentTemperatureChar = new floatCharacteristics(charType_currentTemperature, permission_read|permission_notify,-50, 50, 0.1, unit_celsius);
    currentTemperatureChar->perUserQuery = std::bind(&BME280TemperatureHumiditySensorAccessory::getCurrentTemperature, this, std::placeholders::_1);
    sensorAccessory->addCharacteristics(temperatureSensorService, currentTemperatureChar);

    Service *humiditySensorService = new Service(serviceType_humiditySensor);
    sensorAccessory->addService(humiditySensorService);

    stringCharacteristics *humidityNameCharacteristic = new stringCharacteristics(charType_serviceName, permission_read, 0);
    humidityNameCharacteristic->characteristics::setValue("Humidity sensor");
    sensorAccessory->addCharacteristics(humiditySensorService, humidityNameCharacteristic);

    currentHumidityChar = new floatCharacteristics(charType_currentHumidity, permission_read|permission_notify, 0, 100, 1, unit_percentage);
    currentHumidityChar->perUserQuery = std::bind(&BME280TemperatureHumiditySensorAccessory::getCurrentHumidity, this, std::placeholders::_1);
    sensorAccessory->addCharacteristics(humiditySensorService, currentHumidityChar);

};
