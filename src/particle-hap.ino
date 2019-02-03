/*
 * Project particle-hap
 * Description:
 * Author:
 * Date:
 */

#include "homekit/HKServer.h"
#include "homekit/Accessory.h"

HKServer hkServer;

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
