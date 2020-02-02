
#include "NixieClockAccessory.h"

#include "HKConnection.h"

#include <set>

#include <Particle.h>
#include "HKLog.h"

#include "time/TimeLib.h"

#include "SoftPWM.h"

int digitPINs[10] = {D4,D0,A0,D3,D1,A3,A4,D2,A1,A2};//{A0,A1,A2,A3,A4,D0,D1,D2,D3,D4};
int powerPIN = A5;

int MIN_BRIGHTNESS = 0;
int MAX_BRIGHTNESS = 255;

intCharacteristics *brightnessStateChar = NULL;

std::string NixieClockAccessory::getPower (HKConnection *sender) {
    return on == 1 ? "true" : "false";
}

void NixieClockAccessory::setPower (bool oldValue, bool newValue, HKConnection *sender){
    Particle.publish("nixie/power", newValue ? "on" : "off", PRIVATE);
    on = newValue ? 1 : 0;
}

std::string NixieClockAccessory::getBrightness (HKConnection *sender) {
    return format("%d",(brightness * 100) / 255);
}

void NixieClockAccessory::setBrightness (int oldValue, int newValue, HKConnection *sender) {
    brightness = (255 * newValue) / 100;
}

void NixieClockAccessory::lightIdentify(bool oldValue, bool newValue, HKConnection *sender) {
    Serial.printf("Start Identify Light\n");
}
int fade(int pin, int from, int to) {
  int step = from < to ? 1 : -1;
  int v = from;
  int totalDelay = 250;
  int stepDelay = (totalDelay * 1000) / abs(to - from);
  while(v != to) {
    SoftPWMSet(pin,v);
    delayMicroseconds(stepDelay);
    v += step;
  }
  return v;
}

int showDigit(int pin, int brightness) {
  fade(pin,MIN_BRIGHTNESS,brightness);
  delay(200);
  fade(pin,brightness,MIN_BRIGHTNESS);
}

bool NixieClockAccessory::handle() {
    if(on) {
      if((millis() - lastShowMS) > TIME_PERIOD) {


          time_t utcNow = Time.now();
          time_t local = timezone->toLocal(utcNow);

          int h = hour(local);
          int m = minute(local);

          digitalWrite(powerPIN, 1);
          showDigit(digitPINs[(h / 10) % 10], brightness);
          showDigit(digitPINs[h % 10], brightness);
          delay(400);
          showDigit(digitPINs[(m / 10) % 10], brightness);
          showDigit(digitPINs[m % 10], brightness);
          digitalWrite(powerPIN, 0);

          lastShowMS = millis();


      }

    }
}

void NixieClockAccessory::initAccessorySet() {
  Particle.variable("nixie_on", &this->on, INT);

  pinMode(powerPIN, OUTPUT);


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

  brightnessStateChar = new intCharacteristics(charType_brightness, permission_read|permission_write|permission_notify, 0, 100, 1, unit_percentage);
  brightnessStateChar->perUserQuery = std::bind(&NixieClockAccessory::getBrightness, this, std::placeholders::_1);
  brightnessStateChar->valueChangeFunctionCall = std::bind(&NixieClockAccessory::setBrightness, this, std::placeholders::_1, std::placeholders::_2,std::placeholders::_3);
  nixieAcc->addCharacteristics(lightService, brightnessStateChar);

  SoftPWMBegin(SOFTPWM_INVERTED);
  digitalWrite(powerPIN, 1);
  for(int i = 0; i<10; i++) {
    showDigit(digitPINs[i], brightness);
  }
  digitalWrite(powerPIN, 0);
}
