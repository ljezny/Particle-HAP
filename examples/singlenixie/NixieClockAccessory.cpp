
#include "NixieClockAccessory.h"

#include "HKConnection.h"

#include <set>

#include <Particle.h>
#include "HKLog.h"

#include "time/TimeLib.h"

int digitPINs[10] = {D4,D0,A0,D3,D1,A3,A4,D2,A1,A2};//{A0,A1,A2,A3,A4,D0,D1,D2,D3,D4};
int powerPIN = A5;
int MIN_BRIGHTNESS = 32;
int MAX_BRIGHTNESS = 255;


std::string NixieClockAccessory::getPower (HKConnection *sender){

    return on == 1 ? "true" : "false";
}

void NixieClockAccessory::setPower (bool oldValue, bool newValue, HKConnection *sender){
    Particle.publish("nixie/power", newValue ? "on" : "off", PRIVATE);
    on = newValue ? 1 : 0;
}

void NixieClockAccessory::lightIdentify(bool oldValue, bool newValue, HKConnection *sender) {
    Serial.printf("Start Identify Light\n");
}

int fade(int pin, int from, int to) {
  int step = from < to ? 1 : -1;
  int v = from;
  while(v != to) {
    analogWrite(pin,v,30);
    delay(3);
    v += step;
  }
  return v;
}

int showDigit(int pin) {
  digitalWrite(pin,0);
  fade(powerPIN,MIN_BRIGHTNESS,MAX_BRIGHTNESS);
  //delay(200);
  fade(powerPIN,MAX_BRIGHTNESS,MIN_BRIGHTNESS);
  digitalWrite(pin,1);
}

bool NixieClockAccessory::handle() {
    if(on) {
      if((millis() - lastShowMS) > TIME_PERIOD) {
          time_t utcNow = Time.now();
          time_t local = timezone->toLocal(utcNow);

          int max = MAX_BRIGHTNESS;
          int min = MIN_BRIGHTNESS;

          int h = hour(local);
          int m = minute(local);

          showDigit(digitPINs[(h / 10) % 10]);
          showDigit(digitPINs[h % 10]);
          //delay(300);
          showDigit(digitPINs[(m / 10) % 10]);
          showDigit(digitPINs[m % 10]);

          lastShowMS = millis();
      }
    }
}

void NixieClockAccessory::initAccessorySet() {
  Particle.variable("nixie_on", &this->on, INT);
  pinMode(powerPIN, OUTPUT);
  for(int i = 0; i<10; i++) {
    pinMode(digitPINs[i], OUTPUT);

    showDigit(digitPINs[i]);
  }

  Accessory *nixieAcc = new Accessory();

  //Add Light
  AccessorySet *accSet = &AccessorySet::getInstance();
  addInfoServiceToAccessory(nixieAcc, "SingleNixie", "Dalibor Farny", "Single Nixie", "1","1.0.0", std::bind(&NixieClockAccessory::lightIdentify, this, std::placeholders::_1, std::placeholders::_2,std::placeholders::_3));
  accSet->addAccessory(nixieAcc);

  Service *lightService = new Service(serviceType_lightBulb);
  nixieAcc->addService(lightService);

  stringCharacteristics *lightServiceName = new stringCharacteristics(charType_serviceName, permission_read, 0);
  lightServiceName->characteristics::setValue("Single Nixie");
  nixieAcc->addCharacteristics(lightService, lightServiceName);

  boolCharacteristics *powerState = new boolCharacteristics(charType_on, permission_read|permission_write|permission_notify);
  powerState->perUserQuery = std::bind(&NixieClockAccessory::getPower, this, std::placeholders::_1);
  powerState->valueChangeFunctionCall = std::bind(&NixieClockAccessory::setPower, this, std::placeholders::_1, std::placeholders::_2,std::placeholders::_3);
  nixieAcc->addCharacteristics(lightService, powerState);

}
