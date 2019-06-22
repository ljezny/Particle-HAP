
#include "SerialModemSwitchService.h"

#include "HKConnection.h"

#include <set>

#ifdef PARTICLE_COMPAT
#include "../example/HKTester/HKTester/Particle_Compat/particle_compat.h"
#else
#include <Particle.h>
#endif
#include "HKLog.h"


std::string SerialModemSwitchService::getPower (HKConnection *sender){
    return on ? "true" : "false";
}


void SerialModemSwitchService::setPower (bool oldValue, bool newValue, HKConnection *sender){
    on = newValue;
    Particle.publish("serialmodem/power", String(newValue), PUBLIC);
    if(on) {
        needsSendAT = true;
        
    }
}

bool SerialModemSwitchService::handle() {
    if(needsSendAT) {
        needsSendAT = false;
        Particle.publish("serialmodem/sendcommand", String(openATCommand), PUBLIC);
        Serial1.write(openATCommand);
        delay(12000);
        Particle.publish("serialmodem/sendcommand", String("ATH\r"), PUBLIC);
        Serial1.write("ATH\r");
        closeTimeout = millis() + 4000;
    }
    
    if(closeTimeout != 0 && closeTimeout < millis()) {
      closeTimeout = 0;
      on = false;

      if(powerState != NULL) {
        powerState->notify(NULL);
      } 

      return true;
    }
    return false;
}

void SerialModemSwitchService::initService(Accessory *accessory) {
  //init serial link
  Serial1.begin(9600);
  Service *switchService = new Service(serviceType_switch);
  accessory->addService(switchService);

  stringCharacteristics *serviceName = new stringCharacteristics(charType_serviceName, premission_read, 0);
  serviceName->characteristics::setValue("Switch");
  accessory->addCharacteristics(switchService, serviceName);

  powerState = new boolCharacteristics(charType_on, premission_read|premission_write|premission_notify);
  powerState->perUserQuery = std::bind(&SerialModemSwitchService::getPower, this, std::placeholders::_1);
  powerState->valueChangeFunctionCall = std::bind(&SerialModemSwitchService::setPower, this, std::placeholders::_1, std::placeholders::_2,std::placeholders::_3);
  accessory->addCharacteristics(switchService, powerState);
}
