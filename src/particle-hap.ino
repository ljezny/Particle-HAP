/*
 * Project particle-hap
 * Description:
 * Author:
 * Date:
 */

#include "homekit/HKServer.h"
#include "Accessory.h"

HKServer hkServer = HKServer("ParticleTester","6A:AB:47:A9:B7:24","523-12-643");

// setup() runs once, when the device is first turned on.
void setup() {
  Serial.begin();
  //while(!Serial.isConnected()) Particle.process();
  Serial.println("Hello World!");

  hkServer.setup();
  initAccessorySet();

  //hkServer.persistor->resetPersistor();
}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
  hkServer.handle();
}
