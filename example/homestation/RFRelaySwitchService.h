#ifndef RFRelaySwitchService_hpp
#define RFRelaySwitchService_hpp

#include "homekit/HKAccessory.h"
#include "HAPAccessoryDescriptor.h"
#include "ServiceDescriptor.h"
#include "rcswitch/RCSwitch.h"

class RFRelaySwitchService: public ServiceDescriptor {
private:
  bool on = false;

  int pin = D0;
  int code = 0;

  bool needsSendCode = false;

  RCSwitch *rcSwitch = NULL;

  std::string getPower (HKConnection *sender);
  void setPower (bool oldValue, bool newValue, HKConnection *sender);
public:
  RFRelaySwitchService(int rcPinOutput, int code) {
    this->pin = rcPinOutput;
    this->code = code;
  }

  virtual void initService(Accessory *accessory);
  virtual bool handle();
};

#endif
