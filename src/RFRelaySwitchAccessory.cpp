
#include "RFRelaySwitchAccessory.h"

#include "homekit/HKConnection.h"

#include <set>

#ifdef PARTICLE_COMPAT
#include "../example/HKTester/HKTester/Particle_Compat/particle_compat.h"
#else
#include <Particle.h>
#endif
#include "homekit/HKLog.h"


std::string RFRelaySwitchAccessory::getPower (HKConnection *sender){
    return on ? "true" : "false";
}


void RFRelaySwitchAccessory::setPower (bool oldValue, bool newValue, HKConnection *sender){
    on = newValue;
    needsSendCode = true;

    Particle.publish("rfrelay/power", String(newValue), PUBLIC);
}

void RFRelaySwitchAccessory::identify(bool oldValue, bool newValue, HKConnection *sender) {
    Serial.printf("Start Identify\n");
}


bool RFRelaySwitchAccessory::handle() {
    if(needsSendCode) {
      needsSendCode = false;

      rcSwitch->setRepeatTransmit(10);
      rcSwitch->send(code, 24);

      Particle.publish("rfrelay/sendcode", String(code), PUBLIC);

      return true;
    }
    return false;
}

void RFRelaySwitchAccessory::initAccessorySet() {
  rcSwitch = new RCSwitch();
  rcSwitch->enableTransmit(pin);
  rcSwitch->setProtocol(1);

  Accessory *switchAcc = new Accessory();

  //Add Light
  AccessorySet *accSet = &AccessorySet::getInstance();
  addInfoServiceToAccessory(switchAcc, "Switch", "ljezny", "Switch", "1","1.0.0", std::bind(&RFRelaySwitchAccessory::identify, this, std::placeholders::_1, std::placeholders::_2,std::placeholders::_3));
  accSet->addAccessory(switchAcc);

  Service *switchService = new Service(serviceType_switch);
  switchAcc->addService(switchService);

  stringCharacteristics *roombaServiceName = new stringCharacteristics(charType_serviceName, premission_read, 0);
  roombaServiceName->characteristics::setValue("Relay Switch");
  switchAcc->addCharacteristics(switchService, roombaServiceName);

  boolCharacteristics *powerState = new boolCharacteristics(charType_on, premission_read|premission_write|premission_notify);
  powerState->perUserQuery = std::bind(&RFRelaySwitchAccessory::getPower, this, std::placeholders::_1);
  powerState->valueChangeFunctionCall = std::bind(&RFRelaySwitchAccessory::setPower, this, std::placeholders::_1, std::placeholders::_2,std::placeholders::_3);
  switchAcc->addCharacteristics(switchService, powerState);
}
