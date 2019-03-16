/*
 * Project particle-hap
 * Description:
 * Author:
 * Date:
 */

#include "homekit/HKServer.h"

#include "WindowsShutterAccessory.h"
#include "LightSensorAccessory.h"
#include "MotionSensorAccessory.h"
#include "HomekitBridgeAccessory.h"
#include "LEDStripLightBulbAccessory.h"

//SYSTEM_THREAD(ENABLED);

HKServer *hkServer = NULL;

//HAPAccessoryDescriptor *acc = new WindowsShutterAccessory();
//HAPAccessoryDescriptor *acc = new LEDStripLightBulbAccessory(D2,D1,D0); //Moon project wiring
//HAPAccessoryDescriptor *acc = new LightSensorAccessory();
HomekitBridgeAccessory *acc = new HomekitBridgeAccessory();

void progress(Progress_t progress) {
    Serial.printf("PROGRESS: %d\n",progress);
}


// setup() runs once, when the device is first turned on.
void setup() {

	randomSeed(Time.now());//we need to somehow init random seed, so device identity will be unique
  Serial.begin();

  //HKPersistor().resetAll();

  //BEGIN MYHOME
  acc->descriptors.push_back(new WindowsShutterAccessory(14678913,14678916,1 * sizeof(int)));
  acc->descriptors.push_back(new WindowsShutterAccessory(4102033,4102036,2 * sizeof(int)));
  acc->descriptors.push_back(new WindowsShutterAccessory(4102034,4102040,3 * sizeof(int)));
  //END MYHOME

  acc->initAccessorySet();


}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
  if(WiFi.ready()) {
    if(!hkServer){//first time init
      hkServer = new HKServer(acc->getDeviceType(),"Windows","523-12-643",progress);
      hkServer->setup();
    }
    hkServer->handle();
  }

  acc->handle();
}
