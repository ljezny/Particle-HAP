#ifndef RoombaAccessory_hpp
#define RoombaAccessory_hpp

#include "HKAccessory.h"
#include "HAPAccessoryDescriptor.h"


class RoombaAccessory: public HAPAccessoryDescriptor {
private:
  bool on = false;
  int batteryLevel = 100;
  int chargingState = 0;
  int statusLow = 0;

  int REPORT_PERIOD = 2000;
  int lastMS = 0;

  void roombaIdentify(bool oldValue, bool newValue, HKConnection *sender);

  std::string getPower (HKConnection *sender);
  void setPower (bool oldValue, bool newValue, HKConnection *sender);
  std::string getBatteryLevel (HKConnection *sender);
  std::string getChargingState (HKConnection *sender);
  std::string getStatusLowBattery (HKConnection *sender);
public:
  RoombaAccessory() {
  }

  virtual void initAccessorySet();

  virtual int getDeviceType(){
      return deviceType_switch;
  }
  virtual bool handle();
};

#endif
