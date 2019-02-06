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
  Serial.begin();
  //while(!Serial.isConnected()) Particle.process();
  byte mac[6];
  WiFi.macAddress(mac);
  String macString = String::format("%02X:%02X:%02X:%02X:%02X:%02X",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
  Serial.println("Starting HKServer.");
  //6C:0B:84:59:2E:DE
  Serial.println(macString.c_str());
  hkServer = new HKServer("Particle1",macString.c_str(),"523-12-643");
  hkServer->setup();
  initAccessorySet();

  //hkServer.persistor->resetPersistor();
}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
  hkServer->handle();
}
