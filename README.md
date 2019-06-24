# particle-hap

Particle HAP is a -[proof-of-concept] tested and working tcp server, which allows you to connect Particle devices to Homekit, directly, without any in-the-middle HAP JS server.

## BIG THANKS TO:
I've used lot of inspiration and code from this repositories. I would really like to thank:

- [Crypto inspiration](https://github.com/maximkulkin/esp-homekit-demo)
- [Accessory handling inspiration](https://github.com/etwmc/Personal-HomeKit-HAP)
- [Bonjour inspiration](https://github.com/moflo/homekit-particle)

## Video

Example video of pairing and handling.

[![Watch the video](https://img.youtube.com/vi/jLw-PZ73Wlw/hqdefault.jpg)](https://youtu.be/jLw-PZ73Wlw)

## Configuration

```cpp
//TCP for handling server port
#define TCP_SERVER_PORT 5115
//Maximum pairings available. Apple recommends 16. But be aware, one pairing needs (36+32)B of your EEPROM
#define MAX_PAIRINGS 16
//if you are using EEPROM, you can set offset for Homekit to store pairings
#define EEPROM_STORAGE_ADDRESS_OFFSET 512
```

Also be aware that there's constant `ACCESSORY_KEY` (in `HKConsts.h`). Should be unique per device. I've used constant just to save memory. But it can be a security leak.

## Integration

This project is available as a Community Library, `particle-hap`, which you can [import into your project](https://docs.particle.io/tutorials/device-os/libraries/#using-libraries).

Please see the example projects in the `examples` folder for a better idea of how to use `particle-hap` in your project.

## Troubleshooting

If your Particle device isn't showing up in Homekit, try resetting the `HKPersistor`.

```cpp
HKPersistor().resetAll();
System.reset();
```

If there are too many instances of your Particle device showing up in Homekit, you may want to restart your iPhone,iPad and if it doesn't work you need to flush your iOS device's DNS cache. Unfortunately, this also means you will lose any WiFi networks you have set up, and you'll need to re-enter those passwords. Go to _Settings_, _General_, _Reset_ and choose _Reset Network Settings_.

## Dependencies
There are no library dependencies. All the crypto is based on WolfSSL, but highly customized to work on DeviceOS.

## Known major issues
- no major issues currently known

## What's planned:
- stability issues - but minor ones
- more accessory types in examples (programmable switch, sensors)

## Goal:
- integration with [Nixie Clocks](https://www.daliborfarny.com) so it will support Non-commercial HAP.

## Compiling project
When you're ready to compile your project, make sure you have the correct Particle device target selected and run `particle compile <platform>` in the CLI or click the Compile button in the Desktop IDE. The following files in your project folder will be sent to the compile service:

- Everything in the `/src` folder, including your `.ino` application file
- Everything in the `/lib` folder (which includes the `particle-hap` library)
- The `project.properties` file for your project

# Contribution
Any contributions are more than welcomed. I would prefer to create PR instead of just posting an issues.

## Hire us:
We are two guys, freelancers. We develop Android, iOS applications mostly. We also did some parts of FW for Nixie Clock and of course its iOS app.
Just contact me at [ljezny@gmail.com](mailto:ljezny@gmail.com)

## Licence
Copyright 2019 Lukas Jezny

Licensed under the GNU LPL license.
