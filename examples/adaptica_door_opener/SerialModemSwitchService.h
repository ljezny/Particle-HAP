#ifndef SerialModemSwitchService_hpp
#define SerialModemSwitchService_hpp

#include "HKAccessory.h"
#include "HAPAccessoryDescriptor.h"
#include "HAPServiceDescriptor.h"

class SerialModemSwitchService: public HAPServiceDescriptor {
private:
  bool on = false;
  const char* openATCommand;
  long closeTimeout = 0;
  bool needsSendAT = false;
  std::string getPower (HKConnection *sender);
  void setPower (bool oldValue, bool newValue, HKConnection *sender);
  boolCharacteristics *powerState = NULL;
public:
  SerialModemSwitchService(const char* openATCommand) {
    this->openATCommand = openATCommand;
  }

  virtual void initService(Accessory *accessory);
  virtual bool handle();
};

#endif
