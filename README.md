# particle-hap

Particle HAP is proof of concept to connection Photon to Homekit, directly, without any in-the-middle HAP JS server.

## BIG THANKS TO:
I've used lot of inspiration and code from this repositories. I would really thanks.

- [Crypto inspiration](https://github.com/maximkulkin/esp-homekit-demo)
- [Accessory handling inspiration](https://github.com/etwmc/Personal-HomeKit-HAP)
- [Bonjour inspiration](https://github.com/moflo/homekit-particle)

##Configuration

```
//TCP for handling server port
#define TCP_SERVER_PORT 5115
//Maximum pairings available. Apple recommends 16. But be aware, one pairings needs (36+32)B of your EEPROM
#define MAX_PAIRINGS 16
//if you are using EEPROM, you can set offset for Homekit to store pairings
#define EEPROM_STORAGE_ADDRESS_OFFSET 512

//ID of device, must be unique per device
#define deviceIdentity "5D:C6:5E:50:CE:38"
//Device name, which will be visible during pairing. Should be unique per device, so user can distinguish devices.
#define deviceName "particle"
//Pairing code, should be unique per device
#define devicePassword "523-12-643"
```

Also be aware that there's constant ACCESSORY_KEY (in HKConsts.h). Should be unique per device. I've used constant just to save memory. But it can be a security leak.

## Integration
For integration with your project, look in particle-hap.ino.

```
HKServer hkServer;

void setup() {
  //call to setup TCP Server, Bonjour etc.
  hkServer.setup();
  //implement this method to create custom accessory
  initAccessorySet();

  //uncomment this to clean all paired devices, you will need this a lot when you will debug homekit ;)
  //hkServer.persistor->resetPersistor();
}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
  //call this to handle incoming TCP connections
  hkServer.handle();
}

```

## What's planned:
- need a lot of debugging, currently it's very unstable. Still it proves that it can work and thats important!
- need lot of code refactorings - very messy code for now.
- better logging.

## Goal:
- integration with [Nixie Clocks](https://www.daliborfarny.com) so it will support Non-commercial HAP.

##

## Compiling project

When you're ready to compile your project, make sure you have the correct Particle device target selected and run `particle compile <platform>` in the CLI or click the Compile button in the Desktop IDE. The following files in your project folder will be sent to the compile service:

- Everything in the `/src` folder, including your `.ino` application file
- The `project.properties` file for your project

## Hire us:
We are two guys, freelancers. We develop Android, iOS applications mostly. We also did some parts of FW for Nixie Clock and of course its iOS app.
Just contact me at [ljezny@gmail.com](mailto:ljezny@gmail.com)

## Licence
Copyright 2019 Lukas Jezny

Licensed under the GNU LPL license.
