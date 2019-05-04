//
//  RemoteWeatherStationAccessory.cpp
//  HKTester
//
//  Created by Lukas Jezny on 10/02/2019.
//  Copyright © 2019 Lukas Jezny. All rights reserved.
//

#include "RemoteWeatherStationAccessory.h"

#include "homekit/HKConnection.h"

#include <set>

#ifdef PARTICLE_COMPAT
#include "../example/HKTester/HKTester/Particle_Compat/particle_compat.h"
#else
#include <Particle.h>
#endif
#include "homekit/HKLog.h"

void RemoteWeatherStationAccessory::sensorIdentity(bool oldValue, bool newValue, HKConnection *sender) {
}

std::string RemoteWeatherStationAccessory::getCurrentAmbilightLevel (HKConnection *sender) {
  return format("%d",((int)lux) + 1);
}
std::string RemoteWeatherStationAccessory::getCurrentTemperature (HKConnection *sender) {
  return format("%.1f",temperature);
}

std::string RemoteWeatherStationAccessory::getCurrentHumidity (HKConnection *sender){
  return format("%.1f", humidity);
}

std::string RemoteWeatherStationAccessory::getCurrentBatteryLevel (HKConnection *sender){
  return format("%d",batteryPercent);
}

std::string RemoteWeatherStationAccessory::getStatusLow (HKConnection *sender){
  return format("%d",batteryPercent < 10 ? 1 : 0);
}

std::string RemoteWeatherStationAccessory::getChargingState (HKConnection *sender){
  return format("%d",batteryCharging ? 1 : 0);
}

bool RemoteWeatherStationAccessory::handle() {
    bool result = false;
    
    return result;
}

void RemoteWeatherStationAccessory::remoteEventHandler(const char *eventName, const char *data) {
  Serial.printf("event:%s, data:%s\n",eventName,data);

  if(strcmp(eventName,"bh1750/light") == 0) {
    lux = String(data).toFloat();
    currentAmbilightChar->notify(NULL);
  } else if(strcmp(eventName,"bme280/temperature") == 0) {
    temperature = String(data).toFloat();
    currentTemperatureChar->notify(NULL);
  } else if(strcmp(eventName,"bme280/humidity") == 0) {
    humidity = String(data).toFloat();
    currentHumidityChar->notify(NULL);
  } else if(strcmp(eventName,"battery/level") == 0) {
    int level = String(data).toInt();
    batteryPercent = (100 * (level - batteryLevelMin)) / (batteryLevelMax - batteryLevelMin);
    if(batteryPercent > 100) batteryPercent = 100;
    if(batteryPercent < 0) batteryPercent = 0; 
    batteryLevelChar->notify(NULL);
    batteryStatusLowChar->notify(NULL);
  } else if(strcmp(eventName,"battery/charging") == 0) {
    batteryCharging = String(data).toInt();    
    batteryChargingStateChar->notify(NULL);
  }
}

void RemoteWeatherStationAccessory::initAccessorySet() {
    Accessory *RemoteWeatherStationAccessory = new Accessory();

    AccessorySet *accSet = &AccessorySet::getInstance();
    addInfoServiceToAccessory(RemoteWeatherStationAccessory, "Sensor name", "Vendor name", "Model  name", "1","1.0.0", std::bind(&RemoteWeatherStationAccessory::sensorIdentity, this, std::placeholders::_1, std::placeholders::_2,std::placeholders::_3));
    accSet->addAccessory(RemoteWeatherStationAccessory);

    Service *lightSensorService = new Service(serviceType_lightSensor);
    RemoteWeatherStationAccessory->addService(lightSensorService);

    stringCharacteristics *nameCharacteristic = new stringCharacteristics(charType_serviceName, premission_read, 0);
    nameCharacteristic->characteristics::setValue("Light sensor");
    RemoteWeatherStationAccessory->addCharacteristics(lightSensorService, nameCharacteristic);

    boolCharacteristics *statusActive = new boolCharacteristics(charType_sensorActive, premission_read|premission_notify);
    statusActive->characteristics::setValue("true");
    RemoteWeatherStationAccessory->addCharacteristics(lightSensorService, statusActive);

    currentAmbilightChar = new floatCharacteristics(charType_currentAmbientLightLevel, premission_read|premission_notify, 0.0001, 100000.0, 0, unit_lux);
    currentAmbilightChar->perUserQuery = std::bind(&RemoteWeatherStationAccessory::getCurrentAmbilightLevel, this, std::placeholders::_1);
    RemoteWeatherStationAccessory->addCharacteristics(lightSensorService, currentAmbilightChar);

    Service *temperatureSensorService = new Service(serviceType_temperatureSensor);
    RemoteWeatherStationAccessory->addService(temperatureSensorService);

    stringCharacteristics *temperatureNameCharacteristic = new stringCharacteristics(charType_serviceName, premission_read, 0);
    temperatureNameCharacteristic->characteristics::setValue("Temperature sensor");
    RemoteWeatherStationAccessory->addCharacteristics(temperatureSensorService, temperatureNameCharacteristic);

    currentTemperatureChar = new floatCharacteristics(charType_currentTemperature, premission_read|premission_notify,-50, 50, 0.1, unit_celsius);
    currentTemperatureChar->perUserQuery = std::bind(&RemoteWeatherStationAccessory::getCurrentTemperature, this, std::placeholders::_1);
    RemoteWeatherStationAccessory->addCharacteristics(temperatureSensorService, currentTemperatureChar);

    Service *humiditySensorService = new Service(serviceType_humiditySensor);
    RemoteWeatherStationAccessory->addService(humiditySensorService);

    stringCharacteristics *humidityNameCharacteristic = new stringCharacteristics(charType_serviceName, premission_read, 0);
    humidityNameCharacteristic->characteristics::setValue("Humidity sensor");
    RemoteWeatherStationAccessory->addCharacteristics(humiditySensorService, humidityNameCharacteristic);

    currentHumidityChar = new floatCharacteristics(charType_currentHumidity, premission_read|premission_notify, 0, 100, 1, unit_percentage);
    currentHumidityChar->perUserQuery = std::bind(&RemoteWeatherStationAccessory::getCurrentHumidity, this, std::placeholders::_1);
    RemoteWeatherStationAccessory->addCharacteristics(humiditySensorService, currentHumidityChar);

    Service *batteryService = new Service(serviceType_battery);
    RemoteWeatherStationAccessory->addService(batteryService);

    batteryLevelChar = new intCharacteristics(charType_batteryLevel, premission_read|premission_notify, 0, 100, 1, unit_percentage);
    batteryLevelChar->perUserQuery = std::bind(&RemoteWeatherStationAccessory::getCurrentBatteryLevel, this, std::placeholders::_1);
    RemoteWeatherStationAccessory->addCharacteristics(batteryService, batteryLevelChar);

    batteryChargingStateChar = new intCharacteristics(charType_sensorChargingState, premission_read|premission_notify, 0, 2, 1, unit_percentage);
    batteryChargingStateChar->perUserQuery = std::bind(&RemoteWeatherStationAccessory::getChargingState, this, std::placeholders::_1);
    RemoteWeatherStationAccessory->addCharacteristics(batteryService, batteryChargingStateChar);

    batteryStatusLowChar = new intCharacteristics(charType_sensorLowBattery, premission_read|premission_notify, 0, 1, 1, unit_percentage);
    batteryStatusLowChar->perUserQuery = std::bind(&RemoteWeatherStationAccessory::getStatusLow, this, std::placeholders::_1);
    RemoteWeatherStationAccessory->addCharacteristics(batteryService, batteryStatusLowChar);

    Particle.subscribe("bme280", &RemoteWeatherStationAccessory::remoteEventHandler, this, MY_DEVICES);
    Particle.subscribe("bh1750", &RemoteWeatherStationAccessory::remoteEventHandler, this, MY_DEVICES);
    Particle.subscribe("battery", &RemoteWeatherStationAccessory::remoteEventHandler, this, MY_DEVICES);
};
