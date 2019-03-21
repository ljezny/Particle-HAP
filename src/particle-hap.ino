/*
 * Project particle-hap
 * Description:
 * Author:
 * Date:
 */

#include "homekit/HKServer.h"
#include "homekit/HKLog.h"

#include "WindowsShutterAccessory.h"
#include "LightSensorAccessory.h"
#include "MotionSensorAccessory.h"
#include "HomekitBridgeAccessory.h"
#include "LEDStripLightBulbAccessory.h"
#include "NixieClockAccessory.h"
#include "RoombaAccessory.h"

//SYSTEM_THREAD(ENABLED);
SerialLogHandler logHandler;

HKServer *hkServer = NULL;

//HAPAccessoryDescriptor *acc = new WindowsShutterAccessory();
//HAPAccessoryDescriptor *acc = new LEDStripLightBulbAccessory(D2,D1,D0); //Moon project wiring
//HAPAccessoryDescriptor *acc = new LightSensorAccessory();
//HAPAccessoryDescriptor *acc = new NixieClockAccessory();
//HomekitBridgeAccessory *acc = new HomekitBridgeAccessory();
HAPAccessoryDescriptor *acc = new RoombaAccessory();
void progress(Progress_t progress) {

    hkLog.info("Homekit progress callback: %d",progress);
}

// setup() runs once, when the device is first turned on.
void setup() {
	randomSeed(Time.now());//we need to somehow init random seed, so device identity will be unique
  Serial.begin();

  //HKPersistor().resetAll();

  //BEGIN MYHOME
  //acc->descriptors.push_back(new WindowsShutterAccessory(14678913,14678916,1 * sizeof(int)));
  //acc->descriptors.push_back(new WindowsShutterAccessory(4102033,4102036,2 * sizeof(int)));
  //acc->descriptors.push_back(new WindowsShutterAccessory(4102034,4102040,3 * sizeof(int)));
  //END MYHOME

  acc->initAccessorySet();

  hkServer = new HKServer(acc->getDeviceType(),"Windows","523-12-643",progress);
  //hkServer = new HKServer(acc->getDeviceType(),"Moon","523-12-643",progress);
  //hkServer = new HKServer(acc->getDeviceType(),"SingleNixie","523-12-643",progress);
  hkServer->start();
}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
  //if SYSTEM_THREAD is enable use following code:
  /*if(WiFi.ready()) { //wifi is ready
    if(!hkServer) { //start server
      //hkServer = new HKServer(acc->getDeviceType(),"Windows","523-12-643",progress);
      hkServer = new HKServer(acc->getDeviceType(),"Roomba","523-12-643",progress);
      //hkServer = new HKServer(acc->getDeviceType(),"Moon","523-12-643",progress);
      //hkServer = new HKServer(acc->getDeviceType(),"SingleNixie","523-12-643",progress);
      hkServer->start();
    }
    hkServer->handle(); //handle connections
  } else { //wifi has been lost
    if(hkServer) { //stop server and do cleanup
      hkServer->stop();
      delete hkServer;
      hkServer = NULL;
    }
  }*/

  hkServer->handle(); //handle connections
  acc->handle(); //handle accessory even if there's no wifi connection
}
