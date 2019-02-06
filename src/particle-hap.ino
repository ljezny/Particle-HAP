/*
 * Project particle-hap
 * Description:
 * Author:
 * Date:
 */

#include "homekit/HKServer.h"
#include "Accessory.h"

HKServer *hkServer = NULL;

// setup() runs once, when the device is first turned on.
void setup() {
	randomSeed(Time.now());//we need to somehow init random seed, so device identity will be unique
  Serial.begin();
  //while(!Serial.isConnected()) Particle.process();

  Serial.println(macString.c_str());
  hkServer = new HKServer("Particle1","523-12-643");
  hkServer->setup();
  initAccessorySet();

  //hkServer.persistor->resetPersistor();
}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
  hkServer->handle();
}
