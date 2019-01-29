#ifndef  HKConnection_H
#define HKConnection_H

#include "HKConfig.h"
#include "HKBonjour.h"
#include "HKNetworkMessage.h"
#include "HKNetworkMessageData.h"
#include "HKStringUtils.h"
#include "HKConsts.h"
#include "HKNetworkResponse.h"
#include "HKPersistor.h"
#include <Particle.h>
#include <vector>


extern "C" {
  #include "crypto/types.h"
  #include "crypto/srp.h"
  #include "crypto/error-crypt.h"
  #include "crypto/ed25519.h"
  #include "crypto/chacha.h"
  #include "crypto/chacha20_poly1305.h"
  #include "crypto/curve25519.h"
  #include "crypto/hmac.h"
}

class HKServer;
class characteristics;

class HKConnection {

private:
  TCPClient client;
  Srp srp;

  byte controllerKeyData[CHACHA20_POLY1305_AEAD_KEYSIZE];
  byte sessionKeyData[CHACHA20_POLY1305_AEAD_KEYSIZE];
  byte publicSecretKeyData[CHACHA20_POLY1305_AEAD_KEYSIZE];

  ed25519_key *accessoryKey = (ed25519_key*) ACESSORY_KEY;

  size_t publicKeyLength = 4096;
  char* publicKey = new char[publicKeyLength];

  size_t responseLength = 128;
  char* response = new char[responseLength];

  char* *secretKey = NULL;
  unsigned int secretKeyLength = 0;

  byte sharedKey[CURVE25519_KEYSIZE];
  byte readKey[CHACHA20_POLY1305_AEAD_KEYSIZE];
  int readsCount = 0;
  byte writeKey[CHACHA20_POLY1305_AEAD_KEYSIZE];
  int writesCount = 0;
  bool isEncrypted = false;
  int lastKeepAliveMs = 0;

  std::vector<characteristics *> notifiableCharacteristics;

  void writeEncryptedData(uint8_t* payload,size_t size);
  void decryptData(uint8_t* buffer,size_t *size);
  void readData(uint8_t* buffer,size_t *size);

  void handlePairSetup(const char *buffer);
  bool handlePairVerify(const char *buffer);
  void handleAccessoryRequest(const char *buffer,size_t size);
  void processNotifiableCharacteristics();
public:
  HKServer *server;
  bool relay = false;

  HKConnection(HKServer *s,TCPClient c);
  void handleConnection();
  void keepAlive();

  void writeData(uint8_t* buffer,size_t size);

  bool isConnected(){
    return client.status();
  }
  void close(){
    client.stop();
  }

  void addNotify(characteristics *c);
  void removeNotify(characteristics *c);
};
#endif
