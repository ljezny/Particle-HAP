//
//  LEDStripLightBulbAccessory.cpp
//  HKTester
//
//  Created by Lukas Jezny on 13/03/2019.
//  Copyright © 2019 Lukas Jezny. All rights reserved.
//

#include "HKLog.h"

#include <FastLED.h>
#include <pixeltypes.h>
FASTLED_USING_NAMESPACE;
#define PARTICLE_NO_ARDUINO_COMPATIBILITY 1

#include "FastLEDStripLightBulbAccessory.h"

CRGB leds[NUM_LEDS];

void FastLEDStripLightBulbAccessory::updateColor(RgbColor color)
{
  hkLog.info("Update color: RGBColor(%d, %d, %d)", color.r, color.g, color.b);
  char colourInfo[30] = {0};
  sprintf(colourInfo, "RGBColor(%d, %d, %d)", color.r, color.g, color.b);
  Particle.publish("fastledstrip/updateColor", colourInfo, PRIVATE);
  // Use FastLED to change strip color
  for (int i = 0; i < NUM_LEDS; ++i)
  {
    leds[i].r = color.r;
    leds[i].g = color.g;
    leds[i].b = color.b;
  }
  FastLED.show();
}

void FastLEDStripLightBulbAccessory::initAccessorySet()
{
  pinMode(dataPin, OUTPUT);

  // Use FastLED to initialize strip
  FastLED.addLeds<WS2812, FASTLED_PIN, GRB>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setMaxRefreshRate(60);
  FastLED.setBrightness(BRIGHTNESS);

  LightBulbAccessoryBase::initAccessorySet();

  FastLED.show();
  FastLED.delay(100);
}
