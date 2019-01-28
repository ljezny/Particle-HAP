/*
 * This accessory.cpp is configurated for light accessory
 */

#include "Accessory.h"

#include "HKAccessory.h"

#include "HKConnection.h"


#include <set>
#include <Particle.h>

//set <string> trackingUserList;
//set <HKConnection*> activeUsers;

intCharacteristics *occupyState;

#define userListAddr "./userList"

LEDStatus led(RGB_COLOR_BLUE, LED_PATTERN_BLINK);

void powerTrackable (bool oldValue, bool newValue, HKConnection *sender) {
  led.setActive(newValue);
}
/*
void _newConnection(HKConnection* info) {
    Serial.printf("New connection %s\n", info->hostname.c_str());

    bool originalOutput = activeUsers.size() > 0;
    if ( trackingUserList.count(info->hostname) > 0 )
        activeUsers.insert(info);

    bool laterOutput = activeUsers.size() > 0;

    if (originalOutput != laterOutput) {
        //Should notify
        printf("Changed\n");
        occupyState->notify();
    }
}

void _deadConnection(HKConnection *info) {
    bool originalOutput = activeUsers.size() > 0;
    activeUsers.erase(info);

    bool laterOutput = activeUsers.size() > 0;

    if (originalOutput != laterOutput) {
        //Should notify
        printf("Changed\n");
        occupyState->notify();
    }
}

void loadUserList() {
    ifstream fs;
    fs.open(userListAddr, std::ifstream::in);
    char buffer[256];
    bool isEmpty = fs.peek() == EOF;
    while (!isEmpty&&fs.is_open()&&fs.good()&&!fs.eof()) {
        fs.getline(buffer, 256);
        string s = string(buffer);
        trackingUserList.insert(s);
    }
    fs.close();
}

void saveUserList() {
    ofstream fs;
    fs.open(userListAddr, std::ifstream::out);
    for (set<string>::iterator it = trackingUserList.begin(); it != trackingUserList.end(); it++) {
        fs << *it << "\n";
    }
    fs.close();
}

string trackable(HKConnection *sender) {
    pthread_mutex_lock(&recordMutex);
    string result = trackingUserList.count(sender->hostname) > 0? "1": "0";
    pthread_mutex_unlock(&recordMutex);
    return result;
}

string calculateOccupy(HKConnection *sender) {
    pthread_mutex_lock(&recordMutex);
    string result = activeUsers.size() > 0? "1": "0";
    pthread_mutex_unlock(&recordMutex);
    return result;
}

void switchTrackable(bool oldValue, bool newValue, HKConnection *sender) {
    if (newValue) {
        //Track this device
        trackingUserList.insert(sender->hostname);
        saveUserList();
        //Update active list
        _newConnection(sender);
    } else {
        //Stop tracking
        trackingUserList.erase(sender->hostname);
        saveUserList();
        //Update active list
        _deadConnection(sender);
    }
}*/
int lightStength = 0;
int fanSpeedVal = 0;
void identity(bool oldValue, bool newValue, HKConnection *sender) {
    Serial.printf("Identify\n");
}

void lightIdentify(bool oldValue, bool newValue, HKConnection *sender) {
    Serial.printf("Start Identify Light\n");
}

void fanIdentify(bool oldValue, bool newValue, HKConnection *sender) {
    Serial.printf("Start Identify Fan\n");
}

AccessorySet *accSet;

void initAccessorySet() {

    //newConnection = &_newConnection;
    //deadConnection = &_deadConnection;

    //loadUserList();

    //currentDeviceType = deviceType_bridge;

    Serial.printf("Initial Sensor\n");
/*
    accSet = &AccessorySet::getInstance();
    Accessory *clockAcc = new Accessory();
    addInfoServiceToAccessory(clockAcc, "Nixie Clock", "DaliborFarny", "Nixie Clock v1", "12345678", &identity);
    accSet->addAccessory(clockAcc);

    Service *lightService = new Service(serviceType_lightBulb);
    clockAcc->addService(lightService);

    stringCharacteristics *lightServiceName = new stringCharacteristics(charType_serviceName, premission_read, 0);
    lightServiceName->characteristics::setValue("Clock");
    clockAcc->addCharacteristics(lightService, lightServiceName);

    boolCharacteristics *powerState = new boolCharacteristics(charType_on, premission_read|premission_write|premission_notify);
    powerState->characteristics::setValue("true");
    clockAcc->addCharacteristics(lightService, powerState);

    intCharacteristics *brightnessState = new intCharacteristics(charType_brightness, premission_read|premission_write, 0, 100, 1, unit_percentage);
    brightnessState->characteristics::setValue("50");
    clockAcc->addCharacteristics(lightService, brightnessState);


/*
    stringCharacteristics *sensorServiceName = new stringCharacteristics(charType_serviceName, premission_read, 0);
    sensorServiceName->characteristics::setValue("Wi-Fi Sensor");
    sensorAcc->addCharacteristics(sensorService, sensorServiceName);

    boolCharacteristics *trackableState = new boolCharacteristics(0x10000, premission_read|premission_write);
    trackableState->characteristics::setValue("false");
    trackableState->perUserQuery = &trackable;
    trackableState->valueChangeFunctionCall = &switchTrackable;
    sensorAcc->addCharacteristics(sensorService, trackableState);

    occupyState = new intCharacteristics(charType_occupancyDetected, premission_read|premission_notify, 0, 1, 1, unit_none);
    occupyState->characteristics::setValue("0");
    occupyState->perUserQuery = &calculateOccupy;
    sensorAcc->addCharacteristics(sensorService, occupyState);*/


    Accessory *lightAcc1 = new Accessory();

    //Add Light
    accSet = &AccessorySet::getInstance();
    addInfoServiceToAccessory(lightAcc1, "Tubes", "DaliborFarny", "Nixie Clock v1", "12345678", &lightIdentify);
    accSet->addAccessory(lightAcc1);

    Service *lightService1 = new Service(serviceType_lightBulb);
    lightAcc1->addService(lightService1);

    stringCharacteristics *lightServiceName1 = new stringCharacteristics(charType_serviceName, premission_read, 0);
    lightServiceName1->characteristics::setValue("Tubes");
    lightAcc1->addCharacteristics(lightService1, lightServiceName1);

    boolCharacteristics *powerState1 = new boolCharacteristics(charType_on, premission_read|premission_write|premission_notify);
    powerState1->characteristics::setValue("false");
    powerState1->valueChangeFunctionCall = &powerTrackable;
    lightAcc1->addCharacteristics(lightService1, powerState1);
/*
    intCharacteristics *brightnessState1 = new intCharacteristics(charType_brightness, premission_read|premission_write, 0, 100, 1, unit_percentage);
    brightnessState1->characteristics::setValue("50");
    lightAcc1->addCharacteristics(lightService1, brightnessState1);*/
    /*
    //Add Light
    Accessory *lightAcc2 = new Accessory();

    addInfoServiceToAccessory(lightAcc2, "Underlight", "DaliborFarny", "Nixie Clock v1", "12345678", &lightIdentify);
    accSet->addAccessory(lightAcc2);

    Service *lightService2 = new Service(serviceType_lightBulb);
    lightAcc2->addService(lightService2);

    stringCharacteristics *lightServiceName2 = new stringCharacteristics(charType_serviceName, premission_read, 0);
    lightServiceName2->characteristics::setValue("Underlight");
    lightAcc2->addCharacteristics(lightService2, lightServiceName2);

    boolCharacteristics *powerState2 = new boolCharacteristics(charType_on, premission_read|premission_write|premission_notify);
    powerState2->characteristics::setValue("true");
    lightAcc2->addCharacteristics(lightService2, powerState2);

    intCharacteristics *brightnessState2 = new intCharacteristics(charType_brightness, premission_read|premission_write, 0, 100, 1, unit_percentage);
    brightnessState2->characteristics::setValue("50");
    lightAcc2->addCharacteristics(lightService2, brightnessState2);


    //Add fan
    Accessory *fan = new Accessory();
    addInfoServiceToAccessory(fan, "Fan 1", "ET", "Fan", "12345678", &fanIdentify);
    accSet->addAccessory(fan);

    Service *fanService = new Service(serviceType_fan);
    fan->addService(fanService);

    stringCharacteristics *fanServiceName = new stringCharacteristics(charType_serviceName, premission_read, 0);
    fanServiceName->characteristics::setValue("Fan");
    fan->addCharacteristics(fanService, fanServiceName);

    boolCharacteristics *fanPower = new boolCharacteristics(charType_on, premission_read|premission_write|premission_notify);
    fanPower->characteristics::setValue("true");
    fan->addCharacteristics(fanService, fanPower);*/
};
