#ifndef  HKServer_H
#define HKServer_H
#include "HKConfig.h"
#include "HKBonjour.h"
#include "HKNetworkMessage.h"
#include "HKNetworkMessageData.h"
#include "HKPersistor.h"
#include "HKConfig.h"

#ifdef PARTICLE_COMPAT
#include "../../example/HKTester/HKTester/Particle_Compat/particle_compat.h"
#else
#include <Particle.h>
#endif

#include <vector>

class HKConnection;

class HKServer {
private:
    TCPServer server = TCPServer(TCP_SERVER_PORT);
    HKBonjour bonjour;
    UDP udp;
    bool paired = false;
    std::vector<HKConnection*> clients;
    int deviceType;
    std::string hapName;
    std::string deviceIdentity;
    std::string passcode;

public:
    HKPersistor *persistor;
    void (*progressPtr)(Progress_t);
    HKServer(int deviceType, std::string hapName, std::string passcode, void (*progressPtr)(Progress_t));
    void setup ();
    void setPaired(bool paired);
    void handle();
    std::string getDeviceIdentity() {
        return deviceIdentity;
    }
    std::string getPasscode() {
        return passcode;
    }
};
#endif /* end of include guard: HKServer */
