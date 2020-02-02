#ifndef NixieClockAccessory_hpp
#define NixieClockAccessory_hpp

#include "HKAccessory.h"
#include "HAPAccessoryDescriptor.h"

#include "time/Timezone.h"



class NixieClockAccessory: public HAPAccessoryDescriptor {
private:
  int on = 1;
  int lastShowMS = 0;
  int TIME_PERIOD = 2000;
  int brightness = 255;
  void lightIdentify(bool oldValue, bool newValue, HKConnection *sender);

  std::string getPower (HKConnection *sender);
  void setPower (bool oldValue, bool newValue, HKConnection *sender);
  std::string getBrightness (HKConnection *sender);
  void setBrightness (int oldValue, int newValue, HKConnection *sender);
  Timezone *timezone = NULL;
public:
  NixieClockAccessory() {
    TimeChangeRule CEST = {"CEST", Last, Sun, Mar, 2, 120};     // Central European Summer Time
    TimeChangeRule CET = {"CET ", Last, Sun, Oct, 3, 60};       // Central European Standard Time
    this->timezone = new Timezone(CEST,CET);
  }

  virtual void initAccessorySet();

  virtual int getDeviceType(){
      return deviceType_lightBulb;
  }
  virtual bool handle();
};

#endif
