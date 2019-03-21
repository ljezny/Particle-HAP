
#include "RoombaAccessory.h"

#include "homekit/HKConnection.h"

#include <set>

#ifdef PARTICLE_COMPAT
#include "../example/HKTester/HKTester/Particle_Compat/particle_compat.h"
#else
#include <Particle.h>
#endif
#include "homekit/HKLog.h"


std::string RoombaAccessory::getPower (HKConnection *sender){
    return on ? "true" : "false";
}

std::string RoombaAccessory::getBatteryLevel (HKConnection *sender){
  return "50"; //percent
}
std::string RoombaAccessory::getChargingState (HKConnection *sender){
  return "0"; //0-charging, 1-not charging, 2-not chargable
}
std::string RoombaAccessory::getStatusLowBattery (HKConnection *sender){
  return "0";
}

void RoombaAccessory::setPower (bool oldValue, bool newValue, HKConnection *sender){
    on = newValue;
    if(on) {
      Serial1.write(128); //start communication
      Serial1.write(7); //reset
      Serial1.write(128); //reset
      Serial1.write(135); //clean
      Serial1.write(173);//stop communication
    } else {
      Serial1.write(128); //start communication
      Serial1.write(7); //reset
      Serial1.write(128); //reset
      Serial1.write(143); //dock
      Serial1.write(173);//stop communication
    }
}

void RoombaAccessory::roombaIdentify(bool oldValue, bool newValue, HKConnection *sender) {
    Serial.printf("Start Identify Roomba\n");
}


void RoombaAccessory::handle() {
    //query state?
}

void RoombaAccessory::initAccessorySet() {
  Serial1.begin(115200,SERIAL_8N1);

  Accessory *roombaAcc = new Accessory();

  //Add Light
  AccessorySet *accSet = &AccessorySet::getInstance();
  addInfoServiceToAccessory(roombaAcc, "Roomba", "iRobot", "Roomba", "1","1.0.0", std::bind(&RoombaAccessory::roombaIdentify, this, std::placeholders::_1, std::placeholders::_2,std::placeholders::_3));
  accSet->addAccessory(roombaAcc);

  Service *switchService = new Service(serviceType_switch);
  roombaAcc->addService(switchService);

  stringCharacteristics *roombaServiceName = new stringCharacteristics(charType_serviceName, premission_read, 0);
  roombaServiceName->characteristics::setValue("Roomba");
  roombaAcc->addCharacteristics(switchService, roombaServiceName);

  boolCharacteristics *powerState = new boolCharacteristics(charType_on, premission_read|premission_write|premission_notify);
  powerState->perUserQuery = std::bind(&RoombaAccessory::getPower, this, std::placeholders::_1);
  powerState->valueChangeFunctionCall = std::bind(&RoombaAccessory::setPower, this, std::placeholders::_1, std::placeholders::_2,std::placeholders::_3);
  roombaAcc->addCharacteristics(switchService, powerState);

  Service *batteryService = new Service(serviceType_battery);
  roombaAcc->addService(batteryService);

  intCharacteristics *batteryLevelChar = new intCharacteristics(charType_batteryLevel, premission_read|premission_notify, 0, 100, 1, unit_percentage);
  batteryLevelChar->perUserQuery = std::bind(&RoombaAccessory::getBatteryLevel, this, std::placeholders::_1);  batteryLevelChar->valueChangeFunctionCall = std::bind(&RoombaAccessory::setPower, this, std::placeholders::_1, std::placeholders::_2,std::placeholders::_3);
  roombaAcc->addCharacteristics(batteryService, batteryLevelChar);

  intCharacteristics *chargingStateChar = new intCharacteristics(charType_sensorChargingState, premission_read|premission_notify, 0, 2, 1, unit_percentage);
  chargingStateChar->perUserQuery = std::bind(&RoombaAccessory::getChargingState, this, std::placeholders::_1);
  roombaAcc->addCharacteristics(batteryService, chargingStateChar);

  intCharacteristics *statusLowBatteryChar = new intCharacteristics(charType_sensorLowBattery, premission_read|premission_notify, 0, 1, 1, unit_percentage);
  statusLowBatteryChar->perUserQuery = std::bind(&RoombaAccessory::getStatusLowBattery, this, std::placeholders::_1);
  roombaAcc->addCharacteristics(batteryService, statusLowBatteryChar);
}
