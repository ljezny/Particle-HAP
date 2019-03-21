#ifndef RoombaAccessory_hpp
#define RoombaAccessory_hpp

#include "homekit/HKAccessory.h"
#include "HAPAccessoryDescriptor.h"


class RoombaAccessory: public HAPAccessoryDescriptor {
private:
  bool on = false;
  void roombaIdentify(bool oldValue, bool newValue, HKConnection *sender);

  std::string getPower (HKConnection *sender);
  void setPower (bool oldValue, bool newValue, HKConnection *sender);

public:
  RoombaAccessory() {
  }

  virtual void initAccessorySet();

  virtual int getDeviceType(){
      return deviceType_switch;
  }
  virtual void handle();
};

#endif
