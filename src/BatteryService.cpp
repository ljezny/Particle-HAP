//
//  BatteryService.cpp
//  HKTester
//
//  Created by Lukas Jezny on 10/02/2019.
//  Copyright © 2019 Lukas Jezny. All rights reserved.
//

#include "BatteryService.h"

#include "HKConnection.h"

#include <set>

#ifdef PARTICLE_COMPAT
#include "../example/HKTester/HKTester/Particle_Compat/particle_compat.h"
#else
#include <Particle.h>
#endif
#include "HKLog.h"



void BatteryService::sensorIdentity(bool oldValue, bool newValue, HKConnection *sender) {

}

std::string BatteryService::getCurrentBatteryLevel (HKConnection *sender){
  return format("%d",levelPercent);
}

std::string BatteryService::getStatusLow (HKConnection *sender){
  return format("%d",levelPercent < 10 ? 1 : 0);
}

std::string BatteryService::getChargingState (HKConnection *sender){
  return format("%d",isCharging ? 1 : 0);
}

bool BatteryService::handle() {
  bool result = false;
  int batteryAValue = analogRead(batteryAPIN);
  int chargingValue = digitalRead(chargingDPIN);
  int newPercent = (100 * (batteryAValue - batteryAPINMin)) / (batteryAPINMax - batteryAPINMin);
  if(newPercent > 100) newPercent = 100;
  if(newPercent < 0) newPercent = 0;

  bool newCharging = chargingValue == chargingPINValue;

  if(levelPercent != newPercent) {
    levelPercent = newPercent;
    batteryLevelChar->notify(NULL);
    batteryStatusLowChar->notify(NULL);
    result = true;
  }

  if(isCharging != newCharging) {
    isCharging = newCharging;
    batteryChargingStateChar->notify(NULL);
    result = true;
  }

  //BEGIN BATTERY VALUE READING
  Particle.publish("battery/value", String(batteryAValue), PUBLIC);
  Particle.publish("battery/charging", String(chargingValue), PUBLIC);
  //END BATTERY VALUE READING

  return result;
}

void BatteryService::initService(Accessory *accessory) {
    pinMode(chargingDPIN, INPUT);

    Service *batteryService = new Service(serviceType_battery);
    accessory->addService(batteryService);

    batteryLevelChar = new intCharacteristics(charType_batteryLevel, premission_read|premission_notify, 0, 100, 1, unit_percentage);
    batteryLevelChar->perUserQuery = std::bind(&BatteryService::getCurrentBatteryLevel, this, std::placeholders::_1);
    accessory->addCharacteristics(batteryService, batteryLevelChar);

    batteryChargingStateChar = new intCharacteristics(charType_sensorChargingState, premission_read|premission_notify, 0, 2, 1, unit_percentage);
    batteryChargingStateChar->perUserQuery = std::bind(&BatteryService::getChargingState, this, std::placeholders::_1);
    accessory->addCharacteristics(batteryService, batteryChargingStateChar);

    batteryStatusLowChar = new intCharacteristics(charType_sensorLowBattery, premission_read|premission_notify, 0, 1, 1, unit_percentage);
    batteryStatusLowChar->perUserQuery = std::bind(&BatteryService::getStatusLow, this, std::placeholders::_1);
    accessory->addCharacteristics(batteryService, batteryStatusLowChar);
};
