/* $Id: SoftPWM.cpp 132 2012-02-14 04:15:05Z bhagman@roguerobotics.com $

  A Software PWM Library

  Written by Brett Hagman
  http://www.roguerobotics.com/
  bhagman@roguerobotics.com

  Minor modification by Paul Stoffregen to support different timers

  Adapted for Spark Core by Paul Kourany, 2014-08-12 pkourany@gmail.com

    This library is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*************************************************/

#include "SparkIntervalTimer.h"
#include "SoftPWM.h"

#if !defined(PLATFORM_ID)		// Core v0.3.4
#warning "CORE"
  #define pinSetFast(_pin)		PIN_MAP[_pin].gpio_peripheral->BSRR = PIN_MAP[_pin].gpio_pin
  #define pinResetFast(_pin)	PIN_MAP[_pin].gpio_peripheral->BRR = PIN_MAP[_pin].gpio_pin
  #define digitalWriteFast(pin, value)	(value) ? pinSetFast(pin) : pinResetFast(pin)
#endif

//Define hardware IntervalTimer
IntervalTimer refreshTimer;

volatile uint8_t _isr_softcount = 0xff;
uint8_t _softpwm_defaultPolarity = SOFTPWM_NORMAL;

typedef struct
{
  // hardware I/O port and pin for this channel
  int8_t pin;
  uint8_t polarity;
  uint8_t pwmvalue;
  uint8_t checkval;
  uint8_t fadeuprate;
  uint8_t fadedownrate;
} softPWMChannel;

softPWMChannel _softpwm_channels[SOFTPWM_MAXCHANNELS];


// Here is the meat and gravy
void SoftPWM_Timer_Interrupt(void)
{
  uint8_t i;
  int16_t newvalue;
  int16_t direction;

  if(++_isr_softcount == 0)
  {
    // set all channels high - let's start again
    // and accept new checkvals
    for (i = 0; i < SOFTPWM_MAXCHANNELS; i++)
    {
      if (_softpwm_channels[i].fadeuprate > 0 || _softpwm_channels[i].fadedownrate > 0)
      {
        // we want to fade to the new value
        direction = _softpwm_channels[i].pwmvalue - _softpwm_channels[i].checkval;

        // we will default to jumping to the new value
        newvalue = _softpwm_channels[i].pwmvalue;

        if (direction > 0 && _softpwm_channels[i].fadeuprate > 0)
        {
          newvalue = _softpwm_channels[i].checkval + _softpwm_channels[i].fadeuprate;
          if (newvalue > _softpwm_channels[i].pwmvalue)
            newvalue = _softpwm_channels[i].pwmvalue;
        }
        else if (direction < 0 && _softpwm_channels[i].fadedownrate > 0)
        {
          newvalue = _softpwm_channels[i].checkval - _softpwm_channels[i].fadedownrate;
          if (newvalue < _softpwm_channels[i].pwmvalue)
            newvalue = _softpwm_channels[i].pwmvalue;
        }

        _softpwm_channels[i].checkval = newvalue;
      }
      else  // just set the channel to the new value
        _softpwm_channels[i].checkval = _softpwm_channels[i].pwmvalue;

      // now set the pin high (if not 0)
      if (_softpwm_channels[i].checkval > 0)  // don't set if checkval == 0
      {
        if (_softpwm_channels[i].polarity == SOFTPWM_NORMAL)
		  //PIN_MAP[_softpwm_channels[i].pin].gpio_peripheral->BSRR = PIN_MAP[_softpwm_channels[i].pin].gpio_pin;	//hi
		  pinSetFast(_softpwm_channels[i].pin);
        else
		  //PIN_MAP[_softpwm_channels[i].pin].gpio_peripheral->BRR = PIN_MAP[_softpwm_channels[i].pin].gpio_pin;	//lo
		  pinResetFast(_softpwm_channels[i].pin);
      }

    }
  }

  for (i = 0; i < SOFTPWM_MAXCHANNELS; i++)
  {
    if (_softpwm_channels[i].pin >= 0)  // if it's a valid pin
    {
      if (_softpwm_channels[i].checkval == _isr_softcount)  // if we have hit the width
      {
        // turn off the channel
        if (_softpwm_channels[i].polarity == SOFTPWM_NORMAL)
		  //PIN_MAP[_softpwm_channels[i].pin].gpio_peripheral->BRR = PIN_MAP[_softpwm_channels[i].pin].gpio_pin;	//lo
		  pinResetFast(_softpwm_channels[i].pin);
        else
		  //PIN_MAP[_softpwm_channels[i].pin].gpio_peripheral->BSRR = PIN_MAP[_softpwm_channels[i].pin].gpio_pin;	//hi
		  pinSetFast(_softpwm_channels[i].pin);
      }
    }
  }
}



void SoftPWMBegin(uint8_t defaultPolarity)
{
  uint8_t i;

  refreshTimer.begin(SoftPWM_Timer_Interrupt, 66, uSec);	//Set for 60Hz

  for (i = 0; i < SOFTPWM_MAXCHANNELS; i++)
  {
    _softpwm_channels[i].pin = -1;
    _softpwm_channels[i].polarity = SOFTPWM_NORMAL;
    _softpwm_channels[i].fadeuprate = 0;
    _softpwm_channels[i].fadedownrate = 0;
  }

  _softpwm_defaultPolarity = defaultPolarity;
}


void SoftPWMSetPolarity(int8_t pin, uint8_t polarity)
{
  uint8_t i;

  if (polarity != SOFTPWM_NORMAL)
    polarity = SOFTPWM_INVERTED;

  for (i = 0; i < SOFTPWM_MAXCHANNELS; i++)
  {
    if ((pin < 0 && _softpwm_channels[i].pin >= 0) ||  // ALL pins
       (pin >= 0 && _softpwm_channels[i].pin == pin))  // individual pin
    {
      _softpwm_channels[i].polarity = polarity;
    }
  }
}


void SoftPWMSetPercent(int8_t pin, uint8_t percent, uint8_t hardset)
{
  SoftPWMSet(pin, ((uint16_t)percent * 255) / 100, hardset);
}


void SoftPWMSet(int8_t pin, uint8_t value, uint8_t hardset)
{
  int8_t firstfree = -1;  // first free index
  uint8_t i;

  if (hardset)
  {
	//Reset hardware timer?
    _isr_softcount = 0xff;
  }

  // If the pin isn't already set, add it
  for (i = 0; i < SOFTPWM_MAXCHANNELS; i++)
  {
    if ((pin < 0 && _softpwm_channels[i].pin >= 0) ||  // ALL pins
       (pin >= 0 && _softpwm_channels[i].pin == pin))  // individual pin
    {
      // set the pin (and exit, if individual pin)
      _softpwm_channels[i].pwmvalue = value;

      if (pin >= 0) // we've set the individual pin
        return;
    }

    // get the first free pin if available
    if (firstfree < 0 && _softpwm_channels[i].pin < 0)
      firstfree = i;
  }

  if (pin >= 0 && firstfree >= 0)
  {
    // we have a free pin we can use
    _softpwm_channels[firstfree].pin = pin;
    _softpwm_channels[firstfree].polarity = _softpwm_defaultPolarity;
    _softpwm_channels[firstfree].pwmvalue = value;
//    _softpwm_channels[firstfree].checkval = 0;

    // now prepare the pin for output
    // turn it off to start (no glitch)
    if (_softpwm_defaultPolarity == SOFTPWM_NORMAL)
      digitalWrite(pin, LOW);
    else
      digitalWrite(pin, HIGH);
    pinMode(pin, OUTPUT);
  }
}

void SoftPWMEnd(int8_t pin)
{
  uint8_t i;

  for (i = 0; i < SOFTPWM_MAXCHANNELS; i++)
  {
    if ((pin < 0 && _softpwm_channels[i].pin >= 0) ||  // ALL pins
       (pin >= 0 && _softpwm_channels[i].pin == pin))  // individual pin
    {
      // now disable the pin (put it into INPUT mode)
      digitalWrite(_softpwm_channels[i].pin, 1);
      pinMode(_softpwm_channels[i].pin, INPUT);

      // remove the pin
      _softpwm_channels[i].pin = -1;
    }
  }

  refreshTimer.end();
}


void SoftPWMSetFadeTime(int8_t pin, uint16_t fadeUpTime, uint16_t fadeDownTime)
{
  int16_t fadeAmount;
  uint8_t i;

  for (i = 0; i < SOFTPWM_MAXCHANNELS; i++)
  {
    if ((pin < 0 && _softpwm_channels[i].pin >= 0) ||  // ALL pins
       (pin >= 0 && _softpwm_channels[i].pin == pin))  // individual pin
    {

      fadeAmount = 0;
      if (fadeUpTime > 0)
        fadeAmount = 255UL * 16 / fadeUpTime;

      _softpwm_channels[i].fadeuprate = fadeAmount;

      fadeAmount = 0;
      if (fadeDownTime > 0)
        fadeAmount = 255UL * 16 / fadeDownTime;

      _softpwm_channels[i].fadedownrate = fadeAmount;

      if (pin >= 0)  // we've set individual pin
        break;
    }
  }
}
