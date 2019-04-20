/*
 * Project particle-hap
 * Description:
 * Author:
 * Date:
 */

#include "homekit/HKServer.h"
#include "homekit/HKLog.h"

#include "WindowsShutterService.h"
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



//HAPAccessoryDescriptor *acc = new WindowsShutterAccessory();
//HAPAccessoryDescriptor *acc = new LEDStripLightBulbAccessory(D2,D1,D0); //Moon project wiring
//HAPAccessoryDescriptor *acc = new LightSensorAccessory();
//HAPAccessoryDescriptor *acc = new NixieClockAccessory();
CompositeAccessory *acc = new CompositeAccessory();
//HAPAccessoryDescriptor *acc = new RoombaAccessory();

HKServer hkServer = HKServer(acc->getDeviceType(),"Windows","523-12-643",progress);
//hkServer = HKServer(acc->getDeviceType(),"Roomba","523-12-643",progress);
//hkServer = HKServer(acc->getDeviceType(),"Moon","523-12-643",progress);
//HKServer hkServer = HKServer(acc->getDeviceType(),"SingleNixie","523-12-643",progress);

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

  //BEGIN MYHOME
  acc->descriptors.push_back(new WindowsShutterService(14678913,14678916,1 * sizeof(int)));
  acc->descriptors.push_back(new WindowsShutterService(4102033,4102036,2 * sizeof(int)));
  acc->descriptors.push_back(new WindowsShutterService(4102034,4102040,3 * sizeof(int)));
  acc->descriptors.push_back(new RFRelaySwitchService(D6, 4102038));
  acc->descriptors.push_back(new RFRelaySwitchService(D6, 4102039));

  //END MYHOME

  //BEGIN WeatherStation
  //acc->descriptors.push_back(new BH1750LightSensorAccessory(new BatteryService(D4,HIGH,A1,0, 4095)));
  //acc->descriptors.push_back(new BME280TemperatureHumiditySensorAccessory(new BatteryService(D4,HIGH,A1,0, 4095)));
  //END WeatherStation

  acc->initAccessorySet();

  hkServer.start();

  Particle.function("restart", restart);

}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
  bool didAnything = false; //!hkServer->hasConnections();
  didAnything |= hkServer.handle(); //handle connections, did anything (i.e processed some requests etc.)
  didAnything |= acc->handle(); //handle accessory, did anything (i.e read some sensors)
  if(didAnything) {
    hkLog.info("Free memory %lu",System.freeMemory());
  }
}
