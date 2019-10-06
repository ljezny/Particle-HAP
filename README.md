# particle-hap

Particle HAP is a ~~proof-of-concept~~ tested and working TCP server, which allows you to connect Particle devices to HomeKit, directly, without any in-the-middle HAP JS server.
I am successfully using it to control my Window shutters, Lights, Nixie Clocks, Roomba (using serial link) and I've created simple weather station (temperature, humidity, light). Every device works just fine and stable.

## BIG THANKS TO:
I've used lot of inspiration and code from these repositories. I would really like to thank:

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
//if you are using EEPROM, you can set offset for HomeKit to store pairings
#define EEPROM_STORAGE_ADDRESS_OFFSET 512
//Apple requires 8 connections at least and must accommodate a new ones. Photon can handle up to 10 connections, but one is cloud connection.
//So allow 8 connections, every 9th will be closed send HTTP 503 and closed.
//It seems that even 2 connections are working just fine, if you need to use sockets for something else.
#define MAX_CONNECTIONS 8

```

Also, be aware that there's constant `ACCESSORY_KEY` (in `HKConsts.h`). Should be unique per device. I've used constant just to save memory. But it can be a security leak. If you need to really generate accessory key, look for 'generateAccessoryKey' method and store that key somehow in EEPROM.

## Integration

This project is available as a Community Library, `particle-hap`, which you can [import into your project](https://docs.particle.io/tutorials/device-os/libraries/#using-libraries).

Please see the example projects in the `examples` folder for a better idea of how to use `particle-hap` in your project.

## Troubleshooting

If your Particle device isn't showing up in HomeKit, try resetting the `HKPersistor`.

```cpp
HKPersistor().resetAll();
System.reset();
```

If there are too many instances of your Particle device showing up in HomeKit, you may want to restart your iPhone/iPad, and if it doesn't work you need to flush your iOS device's DNS cache. Unfortunately, this also means you will lose any WiFi networks you have set up, and you'll need to re-enter those passwords. Go to _Settings_, _General_, _Reset_ and choose _Reset Network Settings_.

## Dependencies
There are no library dependencies. All the crypto is based on WolfSSL, but highly customized to work on DeviceOS.

## Known major issues
- no major issues currently known

## Limitations
I had to use static shared memory buffers for processing request and response of HomeKit TCP server. Dynamic allocations of memory causes fragmentation and instability. So currently SHARED_REQUEST_BUFFER_LEN=1024 bytes and SHARED_RESPONSE_BUFFER_LEN=5000 bytes is used. You may reach the response buffer limit if you create some complex composite accessory (more than 5 services).

## Troubleshooting
Q: I am not able to see a device in pairing. Why?
A: You need to be exactly on same Wifi network. Photon connects to 2.4Ghz and your iPhone connects to 5Ghz network by default. Devices not see each other.

Q: How to debug HomeKit accessory discovery?
A: Download Discovery app from AppStore (Bonjour Discovery) and look for _hap._tcp node in that app. You should see your device in there.

Q: Is there any logs?
A: Yes, of course. I am using standard Serial log. I log only the warnings (let's say, the negative branches of IFs :-) )

Q: I can see device ready for pairing, but pairing fails.
A: Mostly you have an error in describing your devices or characteristics values are bad/not in range etc. HomeKit accessory must be described using Apple defined Services. Each service must be described using mandatory and optional characteristics. All can be found in `HAP-Specification-Non-Commercial-Version.pdf` document.

## What's planned:
- more accessory types in examples (programmable switch, sensors). What do you need?

## Goal:
- integration with [Nixie Clocks](https://www.daliborfarny.com) so it will support Non-commercial HAP.

## Compiling project
When you're ready to compile your project, make sure you have the correct Particle device target selected and run `particle compile <platform>` in the CLI or click the Compile button in the Desktop IDE. The following files in your project folder will be sent to the compile service:

- Everything in the `/src` folder, including your `.ino` application file
- Everything in the `/lib` folder (which includes the `particle-hap` library)
- The `project.properties` file for your project

# Contribution
Any contributions are more than welcomed. I would prefer to create PR instead of just posting as issues.

## Hire us:
We are two guys, freelancers. We develop Android, iOS applications mostly. We also did some parts of FW for Nixie Clock and of course its iOS app.
Just contact me at [ljezny@gmail.com](mailto:ljezny@gmail.com)

## Licence
Copyright 2019 Lukas Jezny

Licensed under the MIT license.
