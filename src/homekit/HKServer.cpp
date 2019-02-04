#include "HKServer.h"
#include "HKConnection.h"

#ifndef PARTICLE_COMPAT
#include "spark_wiring_tcpclient.h"
#include "spark_wiring_tcpserver.h"
#include "spark_wiring_network.h"
#include "spark_wiring_thread.h"
#endif

HKServer::HKServer() {
  persistor = new HKPersistor();

}
void HKServer::setup () {
  persistor->loadRecordStorage();

  if (!socket_handle_valid(server_socket_handle)) {
      server_socket_handle = socket_create_tcp_server(TCP_SERVER_PORT, _nif);
      if (socket_handle_valid(server_socket_handle)) {
        Serial.printf("Socket server created at port %d\n", TCP_SERVER_PORT);
      }
  }


  bonjour.setUDP( &udp );
  bonjour.begin(hapName);
  setPaired(false);
}

void HKServer::setPaired(bool p) {
    paired = p;
    bonjour.removeAllServiceRecords();
    char recordTxt[512];
    memset(recordTxt, 0, 512);
    sprintf(recordTxt, "%csf=1%cid=%s%cpv=1.0%cc#=1%cs#=1%cs#=1%cff=0%cmd=%s%cci=%d",4,(char)strlen(deviceIdentity)+3,deviceIdentity,6,4,4,4,4,(char)(strlen(hapName) + 3),hapName,4,5);
    bonjour.addServiceRecord(hapName "._hap",
                             TCP_SERVER_PORT,
                             MDNSServiceTCP,
                             recordTxt); //ci=5-lightbulb, ci=2 bridge
}

void HKServer::handle() {
  bonjour.run();

  if(clients.size() < MAX_CONNECTIONS) {
    if (socket_handle_valid(server_socket_handle)) {
      int client_socket = socket_accept(server_socket_handle);

      if (socket_handle_valid(client_socket))
      {
        Serial.println("Client connected.");
        //clients.push_back(client_socket);
        clients.push_back(new HKConnection(this,client_socket));
      }
    }
  }

  int i = clients.size() - 1;
  while(i >= 0) {
    HKConnection *conn = clients.at(i);

    conn->handleConnection();
    /*if(!conn->isConnected()) {
      conn->close();
      Serial.println("Client removed.");
      clients.erase(clients.begin() + i);
    }*/

    i--;
  }


}
