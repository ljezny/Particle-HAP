#include "HKConnection.h"
#include "HKServer.h"
#include "HKAccessory.h"

void generateAccessoryKey(ed25519_key *key) {
  int r = wc_ed25519_init(key);
  Serial.printf("wc_ed25519_init key: r:%d\n",r);
  WC_RNG rng;
  r = wc_ed25519_make_key(&rng, ED25519_KEY_SIZE, key);
  Serial.printf("wc_ed25519_make_key key: r:%d\n",r);
  print_hex_memory(key,sizeof(ed25519_key));
}

HKConnection::HKConnection(HKServer *s,TCPClient c) {
  client =  c;
  server = s;
  //generateAccessoryKey(&accessoryKey);
}

void HKConnection::writeEncryptedData(uint8_t* payload,size_t size) {
  byte nonce[12];
  memset(nonce, 0, sizeof(nonce));

  byte encrypted[1024 + 18];
  int payload_offset = 0;
  int part = 0;
  Serial.printf("Encrypting message with total size:%d", size);
  while (payload_offset < size) {

      size_t chunk_size = size - payload_offset;
      if (chunk_size > 1024)
          chunk_size = 1024;
      Serial.printf("Encrypting message part: %d chunk_size:%d\n", part, chunk_size);
      byte aead[2] = {chunk_size % 256, chunk_size / 256};

      memcpy(encrypted, aead, 2);

      byte i = 4;
      int x = readsCount++;
      while (x) {
          nonce[i++] = x % 256;
          x /= 256;
      }
      Serial.printf("Encrypting message before wc_ChaCha20Poly1305_Encrypt.\n");
      int r = wc_ChaCha20Poly1305_Encrypt(
          (const byte *) readKey,
          nonce,
          aead, 2,
          (const byte *)payload+payload_offset, chunk_size,
          (byte *) encrypted+2,
          (byte *) (encrypted+chunk_size+2)
      );
      if (r) {
          Serial.printf("Failed to chacha encrypt payload (code %d)\n", r);
      }
      Serial.printf("Sending message part: %d\n", part);
      payload_offset += chunk_size;
      client.write(encrypted, chunk_size + 16 + 2, 2000);

      part++;
  }
}

void HKConnection::decryptData(uint8_t* payload,size_t *size) {
  uint8_t *decryptedData =(uint8_t *) malloc((*size) * sizeof(uint8_t));
  size_t decryptedTotalSize = 0;
  size_t payload_size = *size;
  size_t *decrypted_size = size;

  const size_t block_size = 1024 + 16 + 2;
  size_t required_decrypted_size =
      payload_size / block_size * 1024 + payload_size % block_size - 16 - 2;
  if (*decrypted_size < required_decrypted_size) {
      *decrypted_size = required_decrypted_size;
  }

  decryptedTotalSize = required_decrypted_size;

  byte nonce[12];
  memset(nonce, 0, sizeof(nonce));

  int payload_offset = 0;
  int decrypted_offset = 0;
  Serial.printf("Message decryption started: payloadSize:%d\n", payload_size);
  while (payload_offset < payload_size) {
      size_t chunk_size = payload[payload_offset] + payload[payload_offset+1]*256;
      if (chunk_size+18 > payload_size-payload_offset) {
          // Unfinished chunk
          break;
      }

      byte i = 4;
      int x = writesCount++;
      while (x) {
          nonce[i++] = x % 256;
          x /= 256;
      }
      size_t decrypted_len = *decrypted_size - decrypted_offset;

      Serial.printf("Message block decryption started: chunk:%d\n",chunk_size);
      int r= wc_ChaCha20Poly1305_Decrypt(
          (const byte *) writeKey,
          nonce,
          payload+payload_offset, 2,
          (const byte *)payload+payload_offset+2, chunk_size,
          (const byte *)payload+payload_offset+2 + chunk_size, decryptedData + decrypted_offset
      );
      if (r) {
          Serial.printf("Failed to chacha decrypt payload (code %d)\n", r);
          return;
      }

      decrypted_offset += chunk_size;
      payload_offset += chunk_size + 0x12; // 0x10 is for some auth bytes
  }
  memset(payload,0,*size);
  memcpy(payload,decryptedData,decryptedTotalSize);
  *size = decryptedTotalSize;
  Serial.printf("Message block decryption ended: total size:%d\n",decryptedTotalSize);
  free(decryptedData);
}

void HKConnection::readData(uint8_t* buffer,size_t *size) {
  int total = 0;
  int tempBufferSize = 4096;
  uint8_t *tempBuffer =(uint8_t *) malloc(tempBufferSize * sizeof(uint8_t));
  while(client.available()) {
    int len = client.read(tempBuffer,tempBufferSize);
    memcpy((buffer+total), tempBuffer,len);
    total += len;
    delay(500);
  }

  *size = total;

  if(isEncrypted) {
    decryptData(buffer,size);
  }

  free(tempBuffer);
}

void HKConnection::writeData(uint8_t* responseBuffer,size_t responseLen) {
  //Serial.printf("Response: %s, %d, responseBuffer = %s, responseLen = %d\n", __func__, __LINE__, responseBuffer, responseLen);
  if(client.status() && client.connected()){
    if(isEncrypted) {
      writeEncryptedData((uint8_t *)responseBuffer,responseLen);
    } else {
      client.write((uint8_t *)responseBuffer, (size_t)responseLen, 2000);
    }
  }
}


void HKConnection::handleConnection() {
  if (!client.status() || !client.available()) {
      return;
  }

  int input_buffer_size = 4096;
  uint8_t *inputBuffer =(uint8_t *) malloc(input_buffer_size * sizeof(uint8_t));
  size_t len = 0;

  readData(inputBuffer,&len);
  if (len > 0) {
      lastKeepAliveMs = millis();
      HKNetworkMessage msg((const char *)inputBuffer);
      Serial.printf("Request Message: %s read length: \n", msg.directory, len);
      if (!strcmp(msg.directory, "pair-setup")){
          Serial.printf("Handling Pair Setup...\n");
          handlePairSetup((const char *)inputBuffer);
      }
      else if (!strcmp(msg.directory, "pair-verify")){
        Serial.printf("Handling Pair Varify...\n");
        if(handlePairVerify((const char *)inputBuffer)){
          isEncrypted = true;
        }
      } else if (!strcmp(msg.directory, "identify")){
        client.stop();
      } else if(isEncrypted) { //connection is secured
        Serial.printf("Handling message request: %s\n",msg.directory);
        handleAccessoryRequest((const char *)inputBuffer, len);
      }
  }
  free(inputBuffer);
}

void HKConnection::keepAlive() {
  if((millis() - lastKeepAliveMs) > 10000) {
      lastKeepAliveMs = millis();
      if(client.status()) {
        if(isEncrypted && readsCount > 0) {
          Serial.printf("Keeping alive..\n");

          char *aliveMsg = new char[32];
          strncpy(aliveMsg, "{\"characteristics\": []}", 32);
          //writeData((byte*)aliveMsg,32);
        }
      }
  }
}

int wc_SrpSetKeyH(Srp *srp, byte *secret, word32 size) {
    SrpHash hash;
    int r = BAD_FUNC_ARG;

    srp->key = (byte*) XMALLOC(SHA512_DIGEST_SIZE, NULL, DYNAMIC_TYPE_SRP);
    if (!srp->key)
        return MEMORY_E;

    srp->keySz = SHA512_DIGEST_SIZE;

    r = wc_InitSha512(&hash.data.sha512);
    if (!r) r = wc_Sha512Update(&hash.data.sha512, secret, size);
    if (!r) r = wc_Sha512Final(&hash.data.sha512, srp->key);

    // clean up hash data from stack for security
    memset(&hash, 0, sizeof(hash));

    return r;
}
bool HKConnection::handlePairVerify(const char *buffer) {
  bool completed = false;
  char state = State_Pair_Verify_M1;

  Serial.printf("Start Pair Verify\n");
  HKNetworkMessage msg(buffer);
  HKNetworkResponse response = HKNetworkResponse(200);
  bcopy(msg.data.dataPtrForIndex(6), &state, 1);

  switch (state) {
      case State_Pair_Verify_M1: {
          Serial.printf("Pair Verify M1\n");
          curve25519_key controllerKey;
          int r = wc_curve25519_init(&controllerKey);
          r = wc_curve25519_import_public_ex((const byte *) msg.data.dataPtrForIndex(3) , 32, &controllerKey,EC25519_LITTLE_ENDIAN);
          memcpy(&controllerKeyData,msg.data.dataPtrForIndex(3) , 32);
          Serial.printf("wc_curve25519_import_public: %d\n", r);

          curve25519_key secretKey;
          r = wc_curve25519_init(&secretKey);
          WC_RNG rng;
          wc_curve25519_make_key(&rng,CURVE25519_KEYSIZE,&secretKey);

          size_t publicSecretKeySize = CURVE25519_KEYSIZE;
          r = wc_curve25519_export_public_ex(&secretKey, publicSecretKeyData, &publicSecretKeySize,EC25519_LITTLE_ENDIAN);

          size_t sharedKeySize = CURVE25519_KEYSIZE;


          r = wc_curve25519_shared_secret_ex(&secretKey,&controllerKey,sharedKey,&sharedKeySize,EC25519_LITTLE_ENDIAN);
          Serial.printf("crypto_curve25519_shared_secret: %d\n", r);

          int accessoryInfoSize = CURVE25519_KEYSIZE+CURVE25519_KEYSIZE+strlen(deviceIdentity);
          byte accessoryInfo[accessoryInfoSize];
          memcpy(accessoryInfo,publicSecretKeyData, CURVE25519_KEYSIZE);
          memcpy(&accessoryInfo[CURVE25519_KEYSIZE],deviceIdentity, strlen(deviceIdentity));
          memcpy(&accessoryInfo[CURVE25519_KEYSIZE+strlen(deviceIdentity)],msg.data.dataPtrForIndex(3), CURVE25519_KEYSIZE);

          size_t accessorySignSize = ED25519_SIG_SIZE;
          byte accesorySign[accessorySignSize];
          //ed25519_key *accessoryKey = (ed25519_key*)ACESSORY_KEY;
          r = wc_ed25519_sign_msg(accessoryInfo, accessoryInfoSize, accesorySign, &accessorySignSize,accessoryKey);
          Serial.printf("wc_ed25519_sign_msg: %d\n", r);

          HKNetworkMessageDataRecord signRecord;
          signRecord.activate = true;
          signRecord.data = new char[accessorySignSize];
          signRecord.index = 10;
          signRecord.length = accessorySignSize;
          memcpy(signRecord.data,accesorySign,accessorySignSize);
          Serial.printf("signRecord created: %d\n", r);

          HKNetworkMessageDataRecord idRecord;
          idRecord.index = 1;
          idRecord.activate = true;
          idRecord.length = strlen(deviceIdentity);
          idRecord.data = new char[idRecord.length];
          memcpy(idRecord.data,deviceIdentity, idRecord.length);
          Serial.printf("idRecord created: %d\n", r);

          HKNetworkMessageData data;
          data.addRecord(signRecord);
          data.addRecord(idRecord);
          Serial.printf("plain response created: %d\n", r);

          char salt[] = "Pair-Verify-Encrypt-Salt";
          char info[] = "Pair-Verify-Encrypt-Info";
          size_t sessionKeySize = CHACHA20_POLY1305_AEAD_KEYSIZE;
          r = wc_HKDF(SHA512,(const byte*) sharedKey, sharedKeySize,(const byte*) salt, strlen(salt),(const byte*) info, strlen(info),sessionKeyData, CHACHA20_POLY1305_AEAD_KEYSIZE);
          Serial.printf("wc_HKDF: r:%d\n",r);

          const char *plainMsg = 0;   unsigned short msgLen = 0;
          data.rawData(&plainMsg, &msgLen);

          size_t encryptMsgSize = 0;
          byte encryptMsg[msgLen+16];
          r = wc_ChaCha20Poly1305_Encrypt(
              (const byte *) sessionKeyData,
              (const byte *)"\x0\x0\x0\x0PV-Msg02",
              NULL, 0,
              (const byte *)plainMsg, msgLen,
              (byte *) encryptMsg,
              (byte *) (encryptMsg+msgLen)
          );
          Serial.printf("wc_ChaCha20Poly1305_Encrypt: r:%d\n",r);

          HKNetworkMessageDataRecord stage;
          stage.activate = true;
          stage.data = new char;
          stage.data[0] = State_Pair_Verify_M2;
          stage.index = 6;
          stage.length = 1;


          HKNetworkMessageDataRecord encryptRecord;
          encryptRecord.activate = true;
          encryptRecord.index = 5;
          encryptRecord.length = msgLen+16;
          encryptRecord.data = new char[encryptRecord.length];
          memcpy(encryptRecord.data,encryptMsg,encryptRecord.length);

          HKNetworkMessageDataRecord pubKeyRecord;
          pubKeyRecord.activate = true;
          pubKeyRecord.data = new char[publicSecretKeySize];
          pubKeyRecord.index = 3;
          pubKeyRecord.length = publicSecretKeySize;
          memcpy(pubKeyRecord.data, publicSecretKeyData, publicSecretKeySize);
          Serial.printf("pubKeyRecord created: %d\n", r);

          response.data.addRecord(stage);
          response.data.addRecord(pubKeyRecord);
          response.data.addRecord(encryptRecord);

          delete [] plainMsg;
      }
          break;
      case State_Pair_Verify_M3: {
          Serial.printf("Pair Verify M3\n");
          char *encryptedData = msg.data.dataPtrForIndex(5);
          short packageLen = msg.data.lengthForIndex(5);
          byte decryptedData[packageLen-16];
          int r= wc_ChaCha20Poly1305_Decrypt(
              (const byte *) sessionKeyData,
              (const byte *)"\x0\x0\x0\x0PV-Msg03",
              NULL, 0,
              (const byte *)encryptedData, packageLen-16,
              (const byte *)encryptedData+packageLen-16, decryptedData
          );
          Serial.printf("wc_ChaCha20Poly1305_Decrypt: r:%d\n",r);

          HKNetworkMessageData subData = HKNetworkMessageData((char *)decryptedData, packageLen-16);

          HKKeyRecord rec = server->persistor->getKey(subData.dataPtrForIndex(1));

          int controllerInfoSize = CURVE25519_KEYSIZE+CURVE25519_KEYSIZE+subData.lengthForIndex(1);
          byte controllerInfo[controllerInfoSize];
          memcpy(controllerInfo,controllerKeyData, CURVE25519_KEYSIZE);
          memcpy(&controllerInfo[CURVE25519_KEYSIZE],subData.dataPtrForIndex(1), subData.lengthForIndex(1));
          memcpy(&controllerInfo[CURVE25519_KEYSIZE+subData.lengthForIndex(1)],publicSecretKeyData, CURVE25519_KEYSIZE);

          ed25519_key clKey;
          r = wc_ed25519_init(&clKey);
          Serial.printf("wc_ed25519_init: r:%d\n",r);
          r = wc_ed25519_import_public((const byte*) rec.publicKey, ED25519_PUB_KEY_SIZE, &clKey);
          Serial.printf("wc_ed25519_import_public: r:%d\n",r);
          int verified = 0;
          r = wc_ed25519_verify_msg((byte*) subData.dataPtrForIndex(10), subData.lengthForIndex(10),(const byte*)  controllerInfo,controllerInfoSize, &verified, &clKey);
          Serial.printf("wc_ed25519_verify_msg: r:%d, verified:%d\n",r,verified);
          if(verified) {
            completed = true;

            HKNetworkMessageDataRecord stage;
            stage.activate = true;
            stage.data = new char;
            stage.data[0] = State_Pair_Verify_M4;
            stage.index = 6;
            stage.length = 1;
            response.data.addRecord(stage);

            //hkdf
            const char salt[] = "Control-Salt";
            const char read_info[] = "Control-Read-Encryption-Key";
            const char write_info[] = "Control-Write-Encryption-Key";
            r = wc_HKDF(SHA512,(const byte*) sharedKey, CHACHA20_POLY1305_AEAD_KEYSIZE,(const byte*) salt, strlen(salt),(const byte*) read_info, strlen(read_info),readKey, CHACHA20_POLY1305_AEAD_KEYSIZE);
            Serial.printf("wc_HKDF: r:%d\n",r);
            r = wc_HKDF(SHA512,(const byte*) sharedKey, CHACHA20_POLY1305_AEAD_KEYSIZE,(const byte*) salt, strlen(salt),(const byte*) write_info, strlen(write_info),writeKey, CHACHA20_POLY1305_AEAD_KEYSIZE);
            Serial.printf("wc_HKDF: r:%d\n",r);

            Serial.println("Pair verified, secure connection established");
          }
          else{
            HKNetworkMessageDataRecord error;
            error.activate = true;
            error.data = new char[1];
            error.data[0] = 2;
            error.index = 7;
            error.length = 1;
            response.data.addRecord(error);
            Serial.println("Pair not verified.");
          }
        }
      }

      char *responseBuffer = 0;  int responseLen = 0;
      response.getBinaryPtr(&responseBuffer, &responseLen);
      if (responseBuffer) {
          writeData((byte *)responseBuffer,responseLen);

          delete [] responseBuffer;

      } else {
          Serial.printf("Why empty response\n");
      }
      return completed;
}

void HKConnection::handlePairSetup(const char *buffer) {
    bool completed = false;

    HKNetworkMessageDataRecord stateRecord;
    stateRecord.activate = true;
    stateRecord.data = new char[1];
    stateRecord.length = 1;
    stateRecord.index = 6;

    PairSetupState_t state = State_M1_SRPStartRequest;

    char *responseBuffer = 0; int responseLen = 0;

    HKNetworkMessage msg = HKNetworkMessage(buffer);
    HKNetworkResponse mResponse(200);


    state = (PairSetupState_t)(*msg.data.dataPtrForIndex(6));
    Serial.printf("State: %d\n", state);
    *stateRecord.data = (char)state+1;
    switch (state) {
        case State_M1_SRPStartRequest: {
          Serial.println("State_M1_SRPStartRequest started");
          stateRecord.data[0] = State_M2_SRPStartRespond;
          HKNetworkMessageDataRecord saltRec;
          HKNetworkMessageDataRecord publicKeyRec;

          byte salt[16];
          for (int i = 0; i < 16; i++) {
              salt[i] = rand();
          }

          int r = wc_SrpInit(&srp,SRP_TYPE_SHA512,SRP_CLIENT_SIDE);
          srp.keyGenFunc_cb = wc_SrpSetKeyH;
          if (!r) r = wc_SrpSetUsername(&srp,(const byte *)"Pair-Setup",strlen("Pair-Setup"));
          Serial.printf("wc_SrpSetUsername: %d\n", r);
          if (!r) r = wc_SrpSetParams(&srp,(const byte *)N, sizeof(N),(const byte *)generator, 1,salt,16);
          Serial.printf("wc_SrpSetParams: %d\n", r);
          if (!r) r = wc_SrpSetPassword(&srp,(const byte *)devicePassword,strlen(devicePassword));
          Serial.printf("wc_SrpSetPassword: %d\n", r);
          if (!r) r = wc_SrpGetVerifier(&srp, (byte *)publicKey, &publicKeyLength); //use publicKey to store v
          Serial.printf("wc_SrpGetVerifier: %d\n", r);
          srp.side=SRP_SERVER_SIDE; //switch to server mode
          if (!r) r = wc_SrpSetVerifier(&srp, (byte *)publicKey, publicKeyLength);
          Serial.printf("wc_SrpSetVerifier: %d\n", r);
          if (!r) r = wc_SrpGetPublic(&srp, (byte *)publicKey, &publicKeyLength);
          Serial.printf("wc_SrpGetPublic: %d\n", r);
          saltRec.index = 2;
          saltRec.activate = true;
          saltRec.length = sizeof(salt);
          saltRec.data = new char[saltRec.length];
          memcpy(saltRec.data,salt, saltRec.length);
          publicKeyRec.index = 3;
          publicKeyRec.activate = true;
          publicKeyRec.length = publicKeyLength;
          publicKeyRec.data = new char[publicKeyRec.length];
          memcpy(publicKeyRec.data,publicKey, publicKeyRec.length);

          mResponse.data.addRecord(stateRecord);
          mResponse.data.addRecord(publicKeyRec);
          mResponse.data.addRecord(saltRec);

          Serial.println("State_M1_SRPStartRequest ended.");

          //Serial.println("DUMP pubkey:");
          //print_hex_memory(publicKeyRec.data,publicKeyRec.length);

          //Serial.println("DUMP salt:");
          //print_hex_memory(saltRec.data,saltRec.length);
        }
        break;

        case State_M3_SRPVerifyRequest: {
            Serial.println("State_M3_SRPVerifyRequest started");
            stateRecord.data[0] = State_M4_SRPVerifyRespond;
            const char *keyStr = 0;
            int keyLen = 0;
            const char *proofStr;
            int proofLen = 0;
            keyStr = msg.data.dataPtrForIndex(3);
            keyLen = msg.data.lengthForIndex(3);
            char *temp = msg.data.dataPtrForIndex(4);
            if (temp != NULL) {
                proofStr = temp;
                proofLen = msg.data.lengthForIndex(4);
            }
            Serial.println("State_M3_SRPVerifyRequest data readed");
            Serial.printf("Partner key len: %d\n", keyLen);
            Serial.printf("My key len: %d\n", publicKeyLength);
            Serial.printf("Partner proof len: %d\n", proofLen);

            int r = wc_SrpComputeKey(&srp,(byte*) keyStr,keyLen,(byte*) publicKey,publicKeyLength);
            if (!r) r = wc_SrpVerifyPeersProof(&srp, (byte*) proofStr, proofLen);

            if (r != 0) { //failed
                HKNetworkMessageDataRecord responseRecord;
                responseRecord.activate = true;
                responseRecord.data = new char[1];
                responseRecord.data[0] = 2;
                responseRecord.index = 7;
                responseRecord.length = 1;
                mResponse.data.addRecord(stateRecord);
                mResponse.data.addRecord(responseRecord);

                Serial.println("Oops at M3");
            } else { //success
                wc_SrpGetProof(&srp, (byte *)response,&responseLength);
                //SRP_respond(srp, &response);
                HKNetworkMessageDataRecord responseRecord;
                responseRecord.activate = true;
                responseRecord.index = 4;
                responseRecord.length = responseLength;
                responseRecord.data = new char[responseRecord.length];
                memcpy(responseRecord.data,response, responseRecord.length);

                mResponse.data.addRecord(stateRecord);
                mResponse.data.addRecord(responseRecord);
                Serial.println("Password Correct");
            }
        }
            break;
        case State_M5_ExchangeRequest: {
            Serial.println("State_M5_ExchangeRequest started");
            stateRecord.data[0] = State_M6_ExchangeRespond;
            const char *encryptedPackage = NULL;int packageLen = 0;
            encryptedPackage = msg.data.dataPtrForIndex(5);
            packageLen = msg.data.lengthForIndex(5);
            char *encryptedData = new char[packageLen];
            memcpy(encryptedData,encryptedPackage, packageLen-16);
            char mac[16];
            memcpy(mac,&encryptedPackage[packageLen-16], 16);

            const char salt1[] = "Pair-Setup-Encrypt-Salt";
            const char info1[] = "Pair-Setup-Encrypt-Info";
            uint8_t sharedKey[100];
            int r = wc_HKDF(SHA512,(const byte*) srp.key, srp.keySz,(const byte*) salt1, strlen(salt1),(const byte*) info1, strlen(info1),sharedKey, CHACHA20_POLY1305_AEAD_KEYSIZE);
            Serial.printf("wc_HKDF: r:%d\n",r);
            uint8_t *decryptedData =  new uint8_t[packageLen-16];
            bzero(decryptedData, packageLen-16);
            r= wc_ChaCha20Poly1305_Decrypt(
                (const byte *)sharedKey,
                (const byte *)"\x0\x0\x0\x0PS-Msg05",
                NULL, 0,
                (const byte *)encryptedData, packageLen-16,
                (const byte *)mac, decryptedData
            );
            Serial.printf("wc_ChaCha20Poly1305_Decrypt: r:%d\n",r);

            HKNetworkMessageData *subTLV8 = new HKNetworkMessageData((char *)decryptedData, packageLen-16);
            char *controllerIdentifier = subTLV8->dataPtrForIndex(1);
            char *controllerPublicKey = subTLV8->dataPtrForIndex(3);
            size_t controllerPublicKeySize = subTLV8->lengthForIndex(3);
            char *controllerSignature = subTLV8->dataPtrForIndex(10);
            size_t controllerSignatureSize = subTLV8->lengthForIndex(10);
            char controllerHash[100];

            HKKeyRecord newRecord;
            memcpy(newRecord.controllerID,controllerIdentifier, 36);
            memcpy(newRecord.publicKey,controllerPublicKey, 32);
            server->persistor->addKey(newRecord);


            const char salt2[] = "Pair-Setup-Controller-Sign-Salt";
            const char info2[] = "Pair-Setup-Controller-Sign-Info";
            r = wc_HKDF(SHA512,(const byte*) srp.key, srp.keySz,(const byte*) salt2, strlen(salt2),(const byte*) info2, strlen(info2),(byte*)controllerHash, CHACHA20_POLY1305_AEAD_KEYSIZE);
            Serial.printf("wc_HKDF: r:%d\n",r);
            memcpy(&controllerHash[32],controllerIdentifier, 36);
            memcpy(&controllerHash[68],controllerPublicKey, 32);

            ed25519_key clKey;
            r = wc_ed25519_init(&clKey);
            Serial.printf("wc_ed25519_init: r:%d\n",r);
            r = wc_ed25519_import_public((const byte*) controllerPublicKey, controllerPublicKeySize, &clKey);
            Serial.printf("wc_ed25519_import_public: r:%d\n",r);
            int verified = 0;
            r = wc_ed25519_verify_msg((byte*) controllerSignature, controllerSignatureSize,(const byte*)  controllerHash,100, &verified, &clKey);
            Serial.printf("wc_ed25519_verify_msg: r:%d, verified:%d\n",r,verified);
            if(verified) {
              HKNetworkMessageData *returnTLV8 = new HKNetworkMessageData();

              HKNetworkMessageDataRecord usernameRecord;
              usernameRecord.activate = true;
              usernameRecord.index = 1;
              usernameRecord.length = strlen(deviceIdentity);
              usernameRecord.data = new char[usernameRecord.length];
              memcpy(usernameRecord.data,deviceIdentity,usernameRecord.length);
              returnTLV8->addRecord(usernameRecord);
              Serial.printf("usernameRecord created.\n");

               // Generate Signature
              const char salt3[] = "Pair-Setup-Accessory-Sign-Salt";
              const char info3[] = "Pair-Setup-Accessory-Sign-Info";
              size_t outputSize = 64+strlen(deviceIdentity);
              uint8_t output[outputSize];
              //hkdf((const unsigned char*)salt, strlen(salt), (const unsigned char*)srp.key, srp.keySz, (const unsigned char*)info, strlen(info), output, 32);
              r = wc_HKDF(SHA512,(const byte*) srp.key, srp.keySz,(const byte*) salt3, strlen(salt3),(const byte*) info3, strlen(info3),(byte*)output, CHACHA20_POLY1305_AEAD_KEYSIZE);
              Serial.printf("wc_HKDF: r:%d\n",r);

              //ed25519_key *accessoryKey = (ed25519_key*)ACESSORY_KEY;


              size_t accessoryPubKeySize = ED25519_PUB_KEY_SIZE;
              uint8_t accessoryPubKey[accessoryPubKeySize];
              r = wc_ed25519_export_public(accessoryKey, accessoryPubKey, &accessoryPubKeySize);
              Serial.printf("wc_ed25519_export_public: r:%d\n",r);
              memcpy(&output[32],deviceIdentity,strlen(deviceIdentity));
              memcpy(&output[32+strlen(deviceIdentity)],accessoryPubKey,accessoryPubKeySize);
              Serial.printf("output concatenated: r:%d\n",r);
              size_t signatureSize = 64;
              uint8_t signature[signatureSize];

              r = wc_ed25519_sign_msg(output,outputSize,signature,&signatureSize,accessoryKey);
              Serial.printf("wc_ed25519_sign_msg accessoryKey: r:%d\n",r);

              HKNetworkMessageDataRecord signatureRecord;
              signatureRecord.activate = true;
              signatureRecord.index = 10;
              signatureRecord.length = 64;
              signatureRecord.data = new char[64];
              memcpy(signatureRecord.data,signature,signatureSize);
              returnTLV8->addRecord(signatureRecord);
              Serial.printf("signature record created: r:%d\n",r);

              HKNetworkMessageDataRecord publicKeyRecord;
              publicKeyRecord.activate = true;
              publicKeyRecord.index = 3;
              publicKeyRecord.length = accessoryPubKeySize;
              publicKeyRecord.data = new char[accessoryPubKeySize];
              memcpy(publicKeyRecord.data,accessoryPubKey,accessoryPubKeySize);
              returnTLV8->addRecord(publicKeyRecord);
              Serial.printf("public key record created: r:%d\n",r);

              const char *tlv8Data;unsigned short tlv8Len;
              returnTLV8->rawData(&tlv8Data, &tlv8Len);

              HKNetworkMessageDataRecord tlv8Record;
              tlv8Record.data = new char[tlv8Len+16];
              tlv8Record.length = tlv8Len+16;
              bzero(tlv8Record.data, tlv8Record.length);

              r = wc_ChaCha20Poly1305_Encrypt(
                  (const byte *)sharedKey,
                  (const byte *)"\x0\x0\x0\x0PS-Msg06",
                  NULL, 0,
                  (const byte *)tlv8Data, tlv8Len,
                  (byte*) tlv8Record.data,
                  (byte*) (tlv8Record.data + tlv8Len)
              );
              Serial.printf("wc_ChaCha20Poly1305_Encrypt: r:%d\n",r);


              tlv8Record.activate = true;
              tlv8Record.index = 5;//5

              mResponse.data.addRecord(stateRecord);
              mResponse.data.addRecord(tlv8Record);
              completed = true;
              delete returnTLV8;
            }

            delete subTLV8;
        }
        break;
    }
    mResponse.getBinaryPtr(&responseBuffer, &responseLen);
    if (responseBuffer) {
        writeData((byte*)responseBuffer,responseLen);

        delete [] responseBuffer;
    } else {
        Serial.printf("Why empty response\n");
    }
    if(completed){
      server->setPaired(1);
      //client.stop();
      Serial.println("Pairing completed.");
      wc_SrpTerm(&srp);
    }

}

void HKConnection::handleAccessoryRequest(const char *buffer,size_t size){
  char *resultData = 0; unsigned int resultLen = 0;
  Serial.println("--------BEGIN REQUEST--------");
  Serial.printf("%s\n",buffer);
  Serial.println("--------END REQUEST--------");
  handleAccessory(buffer, size, &resultData, &resultLen, this);
  if(resultLen > 0) {
    Serial.println("--------BEGIN RESPONSE--------");
    Serial.printf("%s\n",resultData);
    Serial.println("--------END RESPONSE--------");
    writeData((byte*)resultData,resultLen);
  }

  //processNotifiableCharacteristics();
}

void HKConnection::processNotifiableCharacteristics() {
  for(int i = 0; i < notifiableCharacteristics.size(); i++) {
    Serial.println("Notifing characteristics value.");
    notifiableCharacteristics.at(i)->notify(this);
  }
  notifiableCharacteristics.clear();
}

void HKConnection::addNotify(characteristics *c){
  notifiableCharacteristics.push_back(c);
}

void HKConnection::removeNotify(characteristics *c){
  int i = notifiableCharacteristics.size() - 1;
  while(i >= 0) {
    characteristics *item = notifiableCharacteristics.at(i);

    if(item == c) {
      Serial.println("removeNotify");
      notifiableCharacteristics.erase(notifiableCharacteristics.begin() + i);
    }
    i--;
  }
}
