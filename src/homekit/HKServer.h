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
  TCPClient newClient;
  HKBonjour bonjour;
  UDP udp;
  bool paired = false;
  std::vector<HKConnection*> clients;
public:
  HKPersistor *persistor;
  HKServer();
  void setup ();
  void setPaired(bool paired);
  void handle();
};
#endif /* end of include guard: HKServer */
