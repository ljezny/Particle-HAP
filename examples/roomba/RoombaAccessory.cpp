
#include "RoombaAccessory.h"

#include "HKConnection.h"

#include <set>

#include <Particle.h>

#include "HKLog.h"

#define OI_START_COMM 128
#define OI_STOP_COMM 173
#define OI_RESET 7
#define OI_CLEAN 135
#define OI_STOP 141
#define OI_DOCK 143
#define OI_QUERY_LIST 149

#define PACKET_ID_BATTERY_CHARGE 25
#define PACKET_ID_BATTERY_CAPACITY 26

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
    Particle.publish("roomba/power", newValue ? "on" : "off", PRIVATE);
    on = newValue;
    if(on) {
      Serial1.write(OI_START_COMM);
      Serial1.write(OI_STOP_COMM);//stop communication
      Serial1.write(OI_START_COMM); //start communication
      Serial1.write(OI_RESET); //reset
      delay(6000);
      Serial1.write(OI_START_COMM); //start communication
      Serial1.write(OI_CLEAN); //clean
      Serial1.write(OI_STOP_COMM);//stop communication
    } else {
      Serial1.write(OI_START_COMM); //start communication
      Serial1.write(OI_DOCK); //stop
      delay(2000);
      Serial1.write(OI_DOCK); //dock
      Serial1.write(OI_STOP_COMM);//stop communication
    }
}

void RoombaAccessory::roombaIdentify(bool oldValue, bool newValue, HKConnection *sender) {
    Serial.printf("Start Identify Roomba\n");
}


bool RoombaAccessory::handle() {
    //query state?
    if((millis() - lastMS) > REPORT_PERIOD) {
        lastMS = REPORT_PERIOD;


    }

    while(Serial1.available()) {
      int v = Serial1.read();

    }

    return false;
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

  stringCharacteristics *roombaServiceName = new stringCharacteristics(charType_serviceName, permission_read, 0);
  roombaServiceName->characteristics::setValue("Roomba");
  roombaAcc->addCharacteristics(switchService, roombaServiceName);

  boolCharacteristics *powerState = new boolCharacteristics(charType_on, permission_read|permission_write|permission_notify);
  powerState->perUserQuery = std::bind(&RoombaAccessory::getPower, this, std::placeholders::_1);
  powerState->valueChangeFunctionCall = std::bind(&RoombaAccessory::setPower, this, std::placeholders::_1, std::placeholders::_2,std::placeholders::_3);
  roombaAcc->addCharacteristics(switchService, powerState);

  Service *batteryService = new Service(serviceType_battery);
  roombaAcc->addService(batteryService);

  intCharacteristics *batteryLevelChar = new intCharacteristics(charType_batteryLevel, permission_read|permission_notify, 0, 100, 1, unit_percentage);
  batteryLevelChar->perUserQuery = std::bind(&RoombaAccessory::getBatteryLevel, this, std::placeholders::_1);
  roombaAcc->addCharacteristics(batteryService, batteryLevelChar);

  intCharacteristics *chargingStateChar = new intCharacteristics(charType_sensorChargingState, permission_read|permission_notify, 0, 2, 1, unit_percentage);
  chargingStateChar->perUserQuery = std::bind(&RoombaAccessory::getChargingState, this, std::placeholders::_1);
  roombaAcc->addCharacteristics(batteryService, chargingStateChar);

  intCharacteristics *statusLowBatteryChar = new intCharacteristics(charType_sensorLowBattery, permission_read|permission_notify, 0, 1, 1, unit_percentage);
  statusLowBatteryChar->perUserQuery = std::bind(&RoombaAccessory::getStatusLowBattery, this, std::placeholders::_1);
  roombaAcc->addCharacteristics(batteryService, statusLowBatteryChar);
}
