
#include "RFRelaySwitchService.h"

#include "HKConnection.h"

#include <set>

#ifdef PARTICLE_COMPAT
#include "../example/HKTester/HKTester/Particle_Compat/particle_compat.h"
#else
#include <Particle.h>
#endif
#include "HKLog.h"


std::string RFRelaySwitchService::getPower (HKConnection *sender){
    return on ? "true" : "false";
}


void RFRelaySwitchService::setPower (bool oldValue, bool newValue, HKConnection *sender){
    on = newValue;
    needsSendCode = true;

    Particle.publish("rfrelay/power", String(newValue), PUBLIC);
}

bool RFRelaySwitchService::handle() {
    if(needsSendCode) {
      needsSendCode = false;

      rcSwitch->setRepeatTransmit(10);
      rcSwitch->send(code, 24);

      Particle.publish("rfrelay/sendcode", String(code), PUBLIC);

      return true;
    }
    return false;
}

void RFRelaySwitchService::initService(Accessory *accessory) {
  rcSwitch = new RCSwitch();
  rcSwitch->enableTransmit(pin);
  rcSwitch->setProtocol(1);

  Service *switchService = new Service(serviceType_switch);
  accessory->addService(switchService);

  stringCharacteristics *roombaServiceName = new stringCharacteristics(charType_serviceName, premission_read, 0);
  roombaServiceName->characteristics::setValue("Relay Switch");
  accessory->addCharacteristics(switchService, roombaServiceName);

  boolCharacteristics *powerState = new boolCharacteristics(charType_on, premission_read|premission_write|premission_notify);
  powerState->perUserQuery = std::bind(&RFRelaySwitchService::getPower, this, std::placeholders::_1);
  powerState->valueChangeFunctionCall = std::bind(&RFRelaySwitchService::setPower, this, std::placeholders::_1, std::placeholders::_2,std::placeholders::_3);
  accessory->addCharacteristics(switchService, powerState);
}
