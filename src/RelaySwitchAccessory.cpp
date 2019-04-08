
#include "RelaySwitchAccessory.h"

#include "homekit/HKConnection.h"

#include <set>

#ifdef PARTICLE_COMPAT
#include "../example/HKTester/HKTester/Particle_Compat/particle_compat.h"
#else
#include <Particle.h>
#endif
#include "homekit/HKLog.h"


std::string RelaySwitchAccessory::getPower (HKConnection *sender){
    return on ? "true" : "false";
}


void RelaySwitchAccessory::setPower (bool oldValue, bool newValue, HKConnection *sender){
    on = newValue;
}

void RelaySwitchAccessory::identify(bool oldValue, bool newValue, HKConnection *sender) {
    Serial.printf("Start Identify\n");
}


bool RelaySwitchAccessory::handle() {
    digitalWrite(pin, on ? HIGH : LOW);
    return false;
}

void RelaySwitchAccessory::initAccessorySet() {
  pinMode(pin, OUTPUT);
  Accessory *switchAcc = new Accessory();

  //Add Light
  AccessorySet *accSet = &AccessorySet::getInstance();
  addInfoServiceToAccessory(switchAcc, "Switch", "ljezny", "Switch", "1","1.0.0", std::bind(&RelaySwitchAccessory::identify, this, std::placeholders::_1, std::placeholders::_2,std::placeholders::_3));
  accSet->addAccessory(switchAcc);

  Service *switchService = new Service(serviceType_switch);
  switchAcc->addService(switchService);

  stringCharacteristics *roombaServiceName = new stringCharacteristics(charType_serviceName, premission_read, 0);
  roombaServiceName->characteristics::setValue("Relay Switch");
  switchAcc->addCharacteristics(switchService, roombaServiceName);

  boolCharacteristics *powerState = new boolCharacteristics(charType_on, premission_read|premission_write|premission_notify);
  powerState->perUserQuery = std::bind(&RelaySwitchAccessory::getPower, this, std::placeholders::_1);
  powerState->valueChangeFunctionCall = std::bind(&RelaySwitchAccessory::setPower, this, std::placeholders::_1, std::placeholders::_2,std::placeholders::_3);
  switchAcc->addCharacteristics(switchService, powerState);
}
