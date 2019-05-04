/*
 * Project particle-hap
 * Description:
 * Author:
 * Date:
 */

#include "homekit/HKServer.h"
#include "homekit/HKLog.h"

#include "JeznyHomeStationBridge.h"
#include "LightSensorAccessory.h"
#include "MotionSensorAccessory.h"
#include "HomekitBridgeAccessory.h"
#include "LEDStripLightBulbAccessory.h"
#include "NixieClockAccessory.h"
#include "RoombaAccessory.h"
#include "BH1750LightSensorAccessory.h"
#include "BME280TemperatureHumiditySensorAccessory.h"
#include "BatteryService.h"
#include "RFRelaySwitchService.h"
#include "CompositeAccessory.h"

SerialLogHandler logHandler;


JeznyHomeStationBridge *acc = new JeznyHomeStationBridge();
//HAPAccessoryDescriptor *acc = new LEDStripLightBulbAccessory(D2,D1,D0); //Moon project wiring
//HAPAccessoryDescriptor *acc = new LightSensorAccessory();
//HAPAccessoryDescriptor *acc = new NixieClockAccessory();
//CompositeAccessory *acc = new CompositeAccessory();
//HAPAccessoryDescriptor *acc = new RoombaAccessory();

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

  //HKPersistor().resetAll();
  
  //hkServer = new HKServer(acc->getDeviceType(),"Windows","523-12-643",progress);
  //hkServer = new HKServer(acc->getDeviceType(),"Roomba","523-12-643",progress);
  //hkServer = new HKServer(acc->getDeviceType(),"Moon1","523-12-643",progress);
  //hkServer = new HKServer(acc->getDeviceType(),"SingleNixie","523-12-643",progress);
  hkServer = new HKServer(acc->getDeviceType(),"Homestation","523-12-643",progress);

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
    hkLog.info("Free memory %lu",System.freeMemory());
  }
}
