#ifndef RFRelaySwitchAccessory_hpp
#define RFRelaySwitchAccessory_hpp

#include "homekit/HKAccessory.h"
#include "HAPAccessoryDescriptor.h"

#include "rcswitch/RCSwitch.h"

class RFRelaySwitchAccessory: public HAPAccessoryDescriptor {
private:
  bool on = false;

  int pin = D0;
  int code = 0;

  RCSwitch *rcSwitch = NULL;

  void identify(bool oldValue, bool newValue, HKConnection *sender);

  std::string getPower (HKConnection *sender);
  void setPower (bool oldValue, bool newValue, HKConnection *sender);
public:
  RFRelaySwitchAccessory(int rcPinOutput, int code) {
    this->pin = rcPinOutput;
    this->code = code;
  }

  virtual void initAccessorySet();

  virtual int getDeviceType(){
      return deviceType_switch;
  }
  virtual bool handle();
};

#endif
