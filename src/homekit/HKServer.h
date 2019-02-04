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
  HKBonjour bonjour;
  UDP udp;
  bool paired = false;
  std::vector<HKConnection*> clients;

  sock_result_t server_socket_handle = socket_handle_invalid();
  network_interface_t _nif = 0;

public:
  HKPersistor *persistor;
  HKServer();
  void setup ();
  void setPaired(bool paired);
  void handle();
};
#endif /* end of include guard: HKServer */
