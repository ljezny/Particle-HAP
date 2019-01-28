# particle-hap

Particle HAP is proof of concept to connection Photon to Homekit, directly, without any in-the-middle HAP JS server.

## BIG THANKS TO:
I've used lot of inspiration and code from this repositories. I would really thanks.

- [Crypto inspiration](https://github.com/maximkulkin/esp-homekit-demo)
- [Accessory handling inspiration](https://github.com/etwmc/Personal-HomeKit-HAP)
- [Bonjour inspiration](https://github.com/moflo/homekit-particle)

## Configuring project
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


## Compiling project

When you're ready to compile your project, make sure you have the correct Particle device target selected and run `particle compile <platform>` in the CLI or click the Compile button in the Desktop IDE. The following files in your project folder will be sent to the compile service:

- Everything in the `/src` folder, including your `.ino` application file
- The `project.properties` file for your project

##Licence
Copyright 2019 Lukas Jezny

Licensed under the GNU LPL license.
