//
//  BatteryService.cpp
//  HKTester
//
//  Created by Lukas Jezny on 10/02/2019.
//  Copyright © 2019 Lukas Jezny. All rights reserved.
//

#include "BatteryService.h"

#include "homekit/HKConnection.h"

#include <set>

#ifdef PARTICLE_COMPAT
#include "../example/HKTester/HKTester/Particle_Compat/particle_compat.h"
#else
#include <Particle.h>
#endif
#include "homekit/HKLog.h"



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
  int newPercent = (100 * (analogRead(batteryAPIN) - batteryAPINMin)) / (batteryAPINMax - batteryAPINMin);
  if(newPercent > 100) newPercent = 100;
  if(newPercent < 0) newPercent = 0;

  int newCharging  = digitalRead(chargingDPIN) == chargingPINValue;
  bool result = false;
  if(levelPercent != newPercent) {
    levelPercent = newPercent;
    batteryLevelChar->notify(NULL);
    batteryStatusLowChar->notify(NULL);
    result = true;
  }

  if(newCharging != isCharging) {
    newCharging = isCharging;
    batteryChargingStateChar->notify(NULL);
    result = true;
  }

  return result;
}

void BatteryService::initService(Accessory *accessory) {
    pinMode(chargingDPIN, INPUT);
    pinMode(batteryAPIN, INPUT);

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
