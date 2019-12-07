#ifndef  HKServer_H
#define HKServer_H
#include "HKConfig.h"
#include "HKBonjour.h"
#include "HKNetworkMessage.h"
#include "HKNetworkMessageData.h"
#include "HKPersistor.h"
#include "HKConfig.h"
#include "HKConnection.h"
#ifdef PARTICLE_COMPAT
#include "../utils/HKTester/HKTester/Particle_Compat/particle_compat.h"
#else
#include <Particle.h>
#endif


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
    int connections = 0;
public:
    HKPersistor *persistor;
    void (*progressPtr)(Progress_t);
    HKServer(int deviceType, std::string hapName, std::string passcode, void (*progressPtr)(Progress_t));
    ~HKServer();
    void start ();
    void stop();

    bool isPaired(){
      return paired;
    }
    void setPaired(bool paired);
    
    bool handle();
    std::string getDeviceIdentity() {
        return deviceIdentity;
    }
    std::string getPasscode() {
        return passcode;
    }
    bool hasConnections() {
      return connections > 0;
    }
};
#endif /* end of include guard: HKServer */
