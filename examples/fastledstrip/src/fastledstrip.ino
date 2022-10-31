/*
 * Project particle-hap
 * Description:
 * Author:
 * Date:
 */

#include "HKServer.h"
#include "HKLog.h"

#include "FastLEDStripLightBulbAccessory.h"
#include "PhotoSystemLEDLightBulbAccessory.h"

#include <FastLED.h>
#include <pixeltypes.h>
FASTLED_USING_NAMESPACE;
#define PARTICLE_NO_ARDUINO_COMPATIBILITY 1

SerialLogHandler logHandler;

HAPAccessoryDescriptor *stripAcc = new FastLEDStripLightBulbAccessory(D0);
HAPAccessoryDescriptor *acc = new PhotonSystemLEDLightBulbAccessory();
HKServer *hkServer = NULL;

// TODO: Try and setup logging to Home Assistant via MQTT: https://github.com/hirotakaster/MQTT/

void progress(Progress_t progress)
{
  hkLog.info("Homekit progress callback: %d", progress);
}
// Cloud functions must return int and take one String
int restart(String extra)
{
  System.reset();
  return 0;
}

int resetAll(String extra)
{
  HKPersistor().resetAll();
  return 0;
}

// setup() runs once, when the device is first turned on.
void setup()
{
  randomSeed(Time.now()); // we need to somehow init random seed, so device identity will be unique
  Serial.begin();
  hkServer = new HKServer(acc->getDeviceType(), "PhotonLED", "523-12-643", progress);
  hkServer = new HKServer(stripAcc->getDeviceType(), "LEDStrip", "523-12-643", progress);

  acc->initAccessorySet();
  stripAcc->initAccessorySet();

  hkServer->start();

  Particle.function("restart", restart);
  Particle.function("resetAll", resetAll);
}

// loop() runs over and over again, as quickly as it can execute.
void loop()
{
  FastLED.show();
  FastLED.delay(100);
  bool didAnything = false;          //! hkServer->hasConnections();
  didAnything |= hkServer->handle(); // handle connections, did anything (i.e processed some requests etc.)
  didAnything |= acc->handle();      // handle accessory, did anything (i.e read some sensors)
  didAnything |= stripAcc->handle();
  if (didAnything)
  {
    // hkLog.info("Free memory %lu",System.freeMemory());
  }
}
