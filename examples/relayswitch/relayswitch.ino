/*
 * Project particle-hap
 * Description:
 * Author:
 * Date:
 */

#include "HKServer.h"
#include "HKLog.h"

#include "RelaySwitchAccessory.h"


SerialLogHandler logHandler(LOG_LEVEL_ALL);


RelaySwitchAccessory *acc = new RelaySwitchAccessory(D0,1);

HKServer *hkServer = NULL;

void progress(Progress_t progress) {
    hkLog.info("Homekit progress callback: %d",progress);
}
// Cloud functions must return int and take one String
int restart(String extra) {
  System.reset();
  return 0;
}

// setup() runs once, when the device is first turned on.
void setup() {
	randomSeed(Time.now());//we need to somehow init random seed, so device identity will be unique
  Serial.begin();

  hkServer = new HKServer(acc->getDeviceType(),"RelaySwitch","523-12-643",progress);

  acc->initAccessorySet();

  hkServer->start();

  Particle.function("restart", restart);

}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
  bool didAnything = false; //!hkServer->hasConnections();
  didAnything |= hkServer->handle(); //handle connections, did anything (i.e processed some requests etc.)
  didAnything |= acc->handle(); //handle accessory, did anything (i.e read some sensors)
  if(didAnything) {
    //hkLog.info("Free memory %lu",System.freeMemory());
  }
}
