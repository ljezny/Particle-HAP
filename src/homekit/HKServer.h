#ifndef  HKServer_H
#define HKServer_H
#include "HKConfig.h"
#include "HKBonjour.h"
#include "HKNetworkMessage.h"
#include "HKNetworkMessageData.h"
#include "HKPersistor.h"
#include <Particle.h>
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
