#ifndef RelaySwitchAccessory_hpp
#define RelaySwitchAccessory_hpp

#include "HKAccessory.h"
#include "HAPAccessoryDescriptor.h"


class RelaySwitchAccessory: public HAPAccessoryDescriptor {
private:
  bool on = false;

  int pin = D0;

  int REPORT_PERIOD = 2000;
  int lastMS = 0;

  void identify(bool oldValue, bool newValue, HKConnection *sender);

  std::string getPower (HKConnection *sender);
  void setPower (bool oldValue, bool newValue, HKConnection *sender);
public:
  RelaySwitchAccessory(int pinOutput, bool initialValue) {
    this->pin = pinOutput;
    this->on = initialValue;
  }

  virtual void initAccessorySet();

  virtual int getDeviceType(){
      return deviceType_switch;
  }
  virtual bool handle();
};

#endif
