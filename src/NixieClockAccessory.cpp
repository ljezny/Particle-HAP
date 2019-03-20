
#include "NixieClockAccessory.h"

#include "homekit/HKConnection.h"

#include <set>

#ifdef PARTICLE_COMPAT
#include "../example/HKTester/HKTester/Particle_Compat/particle_compat.h"
#else
#include <Particle.h>
#endif
#include "homekit/HKLog.h"

#include "time/TimeLib.h"

int digitPINs[10] = {A0,A1,A2,A3,A4,D0,D1,D2,D3,D4};
int powerPIN = A5;

std::string NixieClockAccessory::getPower (HKConnection *sender){
    return on ? "true" : "false";
}

void NixieClockAccessory::setPower (bool oldValue, bool newValue, HKConnection *sender){
    on = newValue;
}

std::string NixieClockAccessory::getBrightness (HKConnection *sender){
    return format("%d",(maxBrightness * 100) / 255);
}

void NixieClockAccessory::setBrightness (int oldValue, int newValue, HKConnection *sender){
    maxBrightness = (newValue * 255) / 100;
}

void NixieClockAccessory::lightIdentify(bool oldValue, bool newValue, HKConnection *sender) {
    Serial.printf("Start Identify Light\n");
}

void NixieClockAccessory::handle() {
    if(on) {
      if((millis() - lastShowMS) > TIME_PERIOD) {
          lastShowMS = millis();

          time_t utcNow = now();
          time_t local = timezone->toLocal(utcNow);

          analogWrite(powerPIN,maxBrightness);
          int h = hour(local);
          int m = minute(local);

          digitalWrite(digitPINs[(h / 10) % 10], 1);
          delay(200);
          digitalWrite(digitPINs[(h / 10) % 10], 0);
          delay(100);
          digitalWrite(digitPINs[h % 10], 1);
          delay(200);
          digitalWrite(digitPINs[h % 10], 0);
          delay(100);

          digitalWrite(digitPINs[(m / 10) % 10], 1);
          delay(200);
          digitalWrite(digitPINs[(m / 10) % 10], 0);
          delay(100);
          digitalWrite(digitPINs[m % 10], 1);
          delay(200);
          digitalWrite(digitPINs[m % 10], 0);
          delay(100);
      }
    }
}

void NixieClockAccessory::initAccessorySet() {
  pinMode(powerPIN, OUTPUT);
  for(int i = 0; i<10; i++) {
    pinMode(digitPINs[i], OUTPUT);
  }

  Accessory *nixieAcc = new Accessory();

  //Add Light
  AccessorySet *accSet = &AccessorySet::getInstance();
  addInfoServiceToAccessory(nixieAcc, "SingleNixie", "Dalibor Farny", "Single Nixie", "1","1.0.0", std::bind(&NixieClockAccessory::lightIdentify, this, std::placeholders::_1, std::placeholders::_2,std::placeholders::_3));
  accSet->addAccessory(nixieAcc);

  Service *lightService = new Service(serviceType_lightBulb);
  nixieAcc->addService(lightService);

  stringCharacteristics *lightServiceName = new stringCharacteristics(charType_serviceName, premission_read, 0);
  lightServiceName->characteristics::setValue("Single Nixie");
  nixieAcc->addCharacteristics(lightService, lightServiceName);

  boolCharacteristics *powerState = new boolCharacteristics(charType_on, premission_read|premission_write|premission_notify);
  powerState->perUserQuery = std::bind(&NixieClockAccessory::getPower, this, std::placeholders::_1);
  powerState->valueChangeFunctionCall = std::bind(&NixieClockAccessory::setPower, this, std::placeholders::_1, std::placeholders::_2,std::placeholders::_3);
  nixieAcc->addCharacteristics(lightService, powerState);

  intCharacteristics *brightnessState = new intCharacteristics(charType_brightness, premission_read|premission_write, 0, 100, 1, unit_percentage);
  brightnessState->perUserQuery = std::bind(&NixieClockAccessory::getBrightness, this, std::placeholders::_1);
  brightnessState->valueChangeFunctionCall = std::bind(&NixieClockAccessory::setBrightness, this, std::placeholders::_1, std::placeholders::_2,std::placeholders::_3);
  nixieAcc->addCharacteristics(lightService, brightnessState);

}
