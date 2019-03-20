#ifndef NixieClockAccessory_hpp
#define NixieClockAccessory_hpp

#include "homekit/HKAccessory.h"
#include "HAPAccessoryDescriptor.h"

class NixieClockAccessory: HAPAccessoryDescriptor {
private:
  int maxBrightness = 255;
  bool on = true;
  int lastShowMS = 0;
  int TIME_PERIOD = 2000;
  void lightIdentify(bool oldValue, bool newValue, HKConnection *sender);
  
  std::string getPower (HKConnection *sender);
  void setPower (bool oldValue, bool newValue, HKConnection *sender);

  std::string getBrightness (HKConnection *sender);
  void setBrightness (int oldValue, int newValue, HKConnection *sender);
public:
  virtual void initAccessorySet();

  virtual int getDeviceType(){
      return deviceType_lightBulb;
  }
  virtual void handle();
};

#endif
