#include "HKConnection.h"
#include "HKServer.h"
#include "HKAccessory.h"
#include "HKLog.h"

uint8_t SHARED_REQUEST_BUFFER[SHARED_REQUEST_BUFFER_LEN] = {0};
uint8_t SHARED_RESPONSE_BUFFER[SHARED_RESPONSE_BUFFER_LEN] = {0};
uint8_t SHARED_TEMP_CRYPTO_BUFFER[SHARED_TEMP_CRYPTO_BUFFER_LEN] = {0};

LEDStatus RGB_STATUS_YELLOW(RGB_COLOR_YELLOW, LED_PATTERN_FADE, LED_PRIORITY_IMPORTANT);

void generateAccessoryKey(ed25519_key *key)
{
    int r = wc_ed25519_init(key);
    hkLog.info("wc_ed25519_init key: r:%d", r);
    WC_RNG rng;
    r = wc_ed25519_make_key(&rng, ED25519_KEY_SIZE, key);
    hkLog.info("wc_ed25519_make_key key: r:%d", r);
    print_hex_memory(key, sizeof(ed25519_key));
}

HKConnection::HKConnection(HKServer *s, TCPClient c)
{
    client = c;
    server = s;
    //generateAccessoryKey(&accessoryKey);
    snprintf(c_ID, CLIENT_ID_MAX_LENGTH, "%d.%d.%d.%d", client.remoteIP()[0], client.remoteIP()[1], client.remoteIP()[2], client.remoteIP()[3]);
}

HKConnection::~HKConnection()
{
    for (uint i = 0; i < notifiableCharacteristics.size(); i++)
    {
        notifiableCharacteristics.at(i)->removeNotifiedConnection(this);
    }
    if(hasSrp){
      wc_SrpTerm(&srp);
    }
}

HKConnection::operator bool()
{
    return client;
}

void HKConnection::doWriteData(uint8_t *data, int size) {
  if (isConnected())
  {
      int bytes = client.write(data, size, 100); //100 ms timeout should be enough
      int err = client.getWriteError();
      if (err != 0 || bytes != size)
      {
          hkLog.warn("doWriteData: failed (error = %d), number of bytes written: %d", err, bytes);
          close();
      }
  }
}

void HKConnection::writeEncryptedData(uint8_t *payload, size_t size)
{
    hkLog.info("writeEncryptedData responseLen:%d", size);
    byte nonce[12] = {0};
    delay(100);
    memset(SHARED_TEMP_CRYPTO_BUFFER, 0, SHARED_TEMP_CRYPTO_BUFFER_LEN);

    uint payload_offset = 0;
    int part = 0;
    while (payload_offset < size)
    {

        size_t chunk_size = size - payload_offset;
        if (chunk_size > 1024)
            chunk_size = 1024;
        byte aead[2] = {(byte)(chunk_size % 256), (byte)(chunk_size / 256)};

        memcpy(SHARED_TEMP_CRYPTO_BUFFER, aead, 2);

        byte i = 4;
        int x = readsCount++;
        while (x)
        {
            nonce[i++] = x % 256;
            x /= 256;
        }
        int r = wc_ChaCha20Poly1305_Encrypt(
            (const byte *)readKey,
            nonce,
            aead, 2,
            (const byte *)payload + payload_offset, chunk_size,
            (byte *)SHARED_TEMP_CRYPTO_BUFFER + 2,
            (byte *)(SHARED_TEMP_CRYPTO_BUFFER + chunk_size + 2));
        if (r)
        {
            hkLog.info("Failed to chacha encrypt payload (code %d)", r);
            client.stop();
            return;
        }
        payload_offset += chunk_size;

        part++;

        doWriteData(SHARED_TEMP_CRYPTO_BUFFER, chunk_size + 16 + 2);
    }
}

void HKConnection::decryptData(uint8_t *payload, size_t *size)
{
    memset(SHARED_TEMP_CRYPTO_BUFFER, 0, SHARED_REQUEST_BUFFER_LEN);
    uint8_t *decryptedData = SHARED_TEMP_CRYPTO_BUFFER;
    size_t decryptedTotalSize = 0;
    size_t payload_size = *size;
    size_t *decrypted_size = size;

    const size_t block_size = 1024 + 16 + 2;
    size_t required_decrypted_size =
        payload_size / block_size * 1024 + payload_size % block_size - 16 - 2;
    if (*decrypted_size < required_decrypted_size)
    {
        *decrypted_size = required_decrypted_size;
    }

    decryptedTotalSize = required_decrypted_size;

    byte nonce[12];
    memset(nonce, 0, sizeof(nonce));

    uint payload_offset = 0;
    int decrypted_offset = 0;
    while (payload_offset < payload_size)
    {
        size_t chunk_size = payload[payload_offset] + payload[payload_offset + 1] * 256;
        if (chunk_size + 18 > payload_size - payload_offset)
        {
            // Unfinished chunk
            break;
        }

        byte i = 4;
        int x = writesCount++;
        while (x)
        {
            nonce[i++] = x % 256;
            x /= 256;
        }

        int r = wc_ChaCha20Poly1305_Decrypt(
            (const byte *)writeKey,
            nonce,
            payload + payload_offset, 2,
            (const byte *)payload + payload_offset + 2, chunk_size,
            (const byte *)payload + payload_offset + 2 + chunk_size, decryptedData + decrypted_offset);
        if (r)
        {
            hkLog.warn("Failed to chacha decrypt payload (code %d)", r);
            *size = 0;
            return;
        }

        decrypted_offset += chunk_size;
        payload_offset += chunk_size + 0x12; // 0x10 is for some auth bytes
    }
    memset(payload, 0, *size);
    memcpy(payload, decryptedData, decryptedTotalSize);
    *size = decryptedTotalSize;
}

void HKConnection::readData(uint8_t *buffer, size_t *size)
{
    int total = 0;
    int bufferSize = 0;

    if (isConnected())
    {
        while (int availableBytes = client.available())
        {
            bufferSize += availableBytes;
            int len = client.read(buffer + total, availableBytes);
            total += len;
        }
    }

    *size = total;

    if (isEncrypted && total > 0)
    {
        decryptData(buffer, size);
    }
}

void HKConnection::writeData(uint8_t *responseBuffer, size_t responseLen)
{

    if (isConnected())
    {
        if (isEncrypted)
        {
            writeEncryptedData((uint8_t *)responseBuffer, responseLen);
        }
        else
        {
            hkLog.info("writeData responseLen:%d", responseLen);
            doWriteData((uint8_t *)responseBuffer, (size_t)responseLen);
        }
    }
}

bool HKConnection::handleConnection(bool maxConnectionsVictim)
{
    size_t len = 0;
    memset(SHARED_REQUEST_BUFFER, 0, SHARED_REQUEST_BUFFER_LEN);
    readData(SHARED_REQUEST_BUFFER, &len);
    bool result = false;
    if (len > 0)
    {

        RGB_STATUS_YELLOW.setActive(true);
        hkLog.info("Request Message read length: %d ", len);
        HKNetworkMessage msg((const char *)SHARED_REQUEST_BUFFER);
        if (!strcmp(msg.directory, "pair-setup"))
        {
            hkLog.info("Handling Pair Setup...");
#ifdef DEBUG_PARTICLE_EVENTS
            Particle.publish("homekit/pair-setup", clientID(), PRIVATE);
#endif
            handlePairSetup((const char *)SHARED_REQUEST_BUFFER);
        }
        else if (!strcmp(msg.directory, "pair-verify"))
        {

            if (!maxConnectionsVictim)
            {
              hkLog.info("Handling Pair Verify...");
#ifdef DEBUG_PARTICLE_EVENTS
              Particle.publish("homekit/pair-verify", clientID(), PRIVATE);
#endif
              if (handlePairVerify((const char *)SHARED_REQUEST_BUFFER))
              {
                  isEncrypted = true;
                  server->setPaired(true);
              }
            } else {
#ifdef DEBUG_PARTICLE_EVENTS
                  Particle.publish("homekit/connection-limit", clientID(), PRIVATE);
#endif
                  //max connections has been reached.
                  memset(SHARED_RESPONSE_BUFFER, 0, SHARED_RESPONSE_BUFFER_LEN);
                  int len = snprintf((char *)SHARED_RESPONSE_BUFFER, SHARED_RESPONSE_BUFFER_LEN, "HTTP/1.1 503 Service Unavailable\r\n\r\n");
                  hkLog.info("Max connections reached, sending response to %s data: %s", clientID(), SHARED_RESPONSE_BUFFER);
                  writeData((byte *)SHARED_RESPONSE_BUFFER, len);
                  close();
            }
        }
        else if (!strcmp(msg.directory, "identify"))
        {
            client.stop();
        }
        else if (isEncrypted)
        {
          //connection is secured
          hkLog.info("Handling message request: %s", msg.directory);
#ifdef DEBUG_PARTICLE_EVENTS
          Particle.publish("homekit/accessory", clientID(), PRIVATE);
#endif
          handleAccessoryRequest((const char *)SHARED_REQUEST_BUFFER, len);
        }

        result = true;
        RGB_STATUS_YELLOW.setActive(false);
    }

    if (isEncrypted){
      if((millis()-lastKeepAliveMs) > 1000) { //send nothing every 1second to check if client is still connected
        lastKeepAliveMs = millis();
        announce("");
      }


      processPostedCharacteristics();
    }

    return result;
}

void HKConnection::announce(char *desc)
{
    memset(SHARED_RESPONSE_BUFFER, 0, SHARED_RESPONSE_BUFFER_LEN);
    int len = snprintf((char*)SHARED_RESPONSE_BUFFER, SHARED_RESPONSE_BUFFER_LEN, "EVENT/1.0 200 OK\r\nContent-Type: application/hap+json\r\nContent-Length: %u\r\n\r\n%s", strlen(desc), desc);
    hkLog.info("Announce: %s, data: %s", clientID(), SHARED_RESPONSE_BUFFER);
    writeData((byte *)SHARED_RESPONSE_BUFFER, len);
}

void HKConnection::processPostedCharacteristics()
{
    for (uint i = 0; i < postedCharacteristics.size(); i++)
    {
        characteristics *c = postedCharacteristics.at(i);
        int len = snprintf(NULL, 0, "{\"characteristics\":[{\"aid\": %d, \"iid\": %d, \"value\": %s}]}", c->accessory->aid, c->iid, c->value(NULL).c_str());
        char buffer[len + 1] = {0};
        snprintf(buffer, len + 1, "{\"characteristics\":[{\"aid\": %d, \"iid\": %d, \"value\": %s}]}", c->accessory->aid, c->iid, c->value(NULL).c_str());
        announce(buffer);
    }
    postedCharacteristics.clear();
}

int wc_SrpSetKeyH(Srp *srp, byte *secret, word32 size)
{
    SrpHash hash;
    int r = BAD_FUNC_ARG;

    srp->key = (byte *)XMALLOC(SHA512_DIGEST_SIZE, NULL, DYNAMIC_TYPE_SRP);
    if (!srp->key)
        return MEMORY_E;

    srp->keySz = SHA512_DIGEST_SIZE;

    r = wc_InitSha512(&hash.data.sha512);
    if (!r)
        r = wc_Sha512Update(&hash.data.sha512, secret, size);
    if (!r)
        r = wc_Sha512Final(&hash.data.sha512, srp->key);

    // clean up hash data from stack for security
    memset(&hash, 0, sizeof(hash));

    return r;
}
bool HKConnection::handlePairVerify(const char *buffer)
{
    bool completed = false;
    char state = State_Pair_Verify_M1;

    HKNetworkMessage msg(buffer);
    HKNetworkResponse response = HKNetworkResponse(200);
    bcopy(msg.data.dataPtrForIndex(6), &state, 1);

    switch (state)
    {
    case State_Pair_Verify_M1:
    {
        server->progressPtr(Progress_Pair_Verify_M1);
        hkLog.info("Pair Verify M1");
        curve25519_key controllerKey;
        int r = wc_curve25519_init(&controllerKey);
        if (r)
            hkLog.warn("wc_curve25519_init key: r:%d", r);
        r = wc_curve25519_import_public_ex((const byte *)msg.data.dataPtrForIndex(3), 32, &controllerKey, EC25519_LITTLE_ENDIAN);
        if (r)
            hkLog.warn("wc_curve25519_import_public_ex: r:%d", r);
        memcpy(&controllerKeyData, msg.data.dataPtrForIndex(3), 32);

        curve25519_key secretKey;
        r = wc_curve25519_init(&secretKey);
        if (r)
            hkLog.warn("wc_curve25519_init: r:%d", r);
        WC_RNG rng;
        wc_curve25519_make_key(&rng, CURVE25519_KEYSIZE, &secretKey);
        if (r)
            hkLog.warn("wc_curve25519_make_key: r:%d", r);

        word32 publicSecretKeySize = CURVE25519_KEYSIZE;
        r = wc_curve25519_export_public_ex(&secretKey, publicSecretKeyData, &publicSecretKeySize, EC25519_LITTLE_ENDIAN);
        if (r)
            hkLog.warn("wc_curve25519_export_public_ex: r:%d", r);
        word32 sharedKeySize = CURVE25519_KEYSIZE;

        r = wc_curve25519_shared_secret_ex(&secretKey, &controllerKey, sharedKey, &sharedKeySize, EC25519_LITTLE_ENDIAN);
        if (r)
            hkLog.warn("crypto_curve25519_shared_secret: %d", r);

        int accessoryInfoSize = CURVE25519_KEYSIZE + CURVE25519_KEYSIZE + server->getDeviceIdentity().length();
        byte accessoryInfo[accessoryInfoSize];
        memcpy(accessoryInfo, publicSecretKeyData, CURVE25519_KEYSIZE);
        memcpy(&accessoryInfo[CURVE25519_KEYSIZE], server->getDeviceIdentity().c_str(), server->getDeviceIdentity().length());
        memcpy(&accessoryInfo[CURVE25519_KEYSIZE + server->getDeviceIdentity().length()], msg.data.dataPtrForIndex(3), CURVE25519_KEYSIZE);

        word32 accessorySignSize = ED25519_SIG_SIZE;
        byte accesorySign[accessorySignSize];
        r = wc_ed25519_sign_msg(accessoryInfo, accessoryInfoSize, accesorySign, &accessorySignSize, accessoryKey);
        if (r)
            hkLog.warn("wc_ed25519_sign_msg: r:%d", r);

        HKNetworkMessageDataRecord signRecord;
        signRecord.activate = true;
        signRecord.data = new char[accessorySignSize];
        signRecord.index = 10;
        signRecord.length = accessorySignSize;
        memcpy(signRecord.data, accesorySign, accessorySignSize);

        HKNetworkMessageDataRecord idRecord;
        idRecord.index = 1;
        idRecord.activate = true;
        idRecord.length = server->getDeviceIdentity().length();
        idRecord.data = new char[idRecord.length];
        memcpy(idRecord.data, server->getDeviceIdentity().c_str(), idRecord.length);

        HKNetworkMessageData data;
        data.addRecord(signRecord);
        data.addRecord(idRecord);

        char salt[] = "Pair-Verify-Encrypt-Salt";
        char info[] = "Pair-Verify-Encrypt-Info";
        r = wc_HKDF(SHA512, (const byte *)sharedKey, sharedKeySize, (const byte *)salt, strlen(salt), (const byte *)info, strlen(info), sessionKeyData, CHACHA20_POLY1305_AEAD_KEYSIZE);
        if (r)
            hkLog.warn("wc_HKDF: r:%d", r);

        const char *plainMsg = 0;
        unsigned short msgLen = 0;
        data.rawData(&plainMsg, &msgLen);

        byte encryptMsg[msgLen + 16];
        r = wc_ChaCha20Poly1305_Encrypt(
            (const byte *)sessionKeyData,
            (const byte *)"\x0\x0\x0\x0PV-Msg02",
            NULL, 0,
            (const byte *)plainMsg, msgLen,
            (byte *)encryptMsg,
            (byte *)(encryptMsg + msgLen));
        if (r)
            hkLog.warn("wc_ChaCha20Poly1305_Encrypt: r:%d", r);
        HKNetworkMessageDataRecord stage;
        stage.activate = true;
        stage.data = new char;
        stage.data[0] = State_Pair_Verify_M2;
        stage.index = 6;
        stage.length = 1;

        HKNetworkMessageDataRecord encryptRecord;
        encryptRecord.activate = true;
        encryptRecord.index = 5;
        encryptRecord.length = msgLen + 16;
        encryptRecord.data = new char[encryptRecord.length];
        memcpy(encryptRecord.data, encryptMsg, encryptRecord.length);

        HKNetworkMessageDataRecord pubKeyRecord;
        pubKeyRecord.activate = true;
        pubKeyRecord.data = new char[publicSecretKeySize];
        pubKeyRecord.index = 3;
        pubKeyRecord.length = publicSecretKeySize;
        memcpy(pubKeyRecord.data, publicSecretKeyData, publicSecretKeySize);

        response.data.addRecord(stage);
        response.data.addRecord(pubKeyRecord);
        response.data.addRecord(encryptRecord);

        delete[] plainMsg;
        server->progressPtr(Progress_Pair_Verify_M2);

    }
    break;
    case State_Pair_Verify_M3:
    {
        hkLog.info("Pair Verify M3");
        server->progressPtr(Progress_Pair_Verify_M3);
        char *encryptedData = msg.data.dataPtrForIndex(5);
        short packageLen = msg.data.lengthForIndex(5);
        byte decryptedData[packageLen - 16];
        int r = wc_ChaCha20Poly1305_Decrypt(
            (const byte *)sessionKeyData,
            (const byte *)"\x0\x0\x0\x0PV-Msg03",
            NULL, 0,
            (const byte *)encryptedData, packageLen - 16,
            (const byte *)encryptedData + packageLen - 16, decryptedData);
        if (r)
            hkLog.warn("wc_ChaCha20Poly1305_Decrypt: r:%d", r);
        HKNetworkMessageData subData = HKNetworkMessageData((char *)decryptedData, packageLen - 16);
        HKKeyRecord rec = server->persistor->getKey(subData.dataPtrForIndex(1));

        int controllerInfoSize = CURVE25519_KEYSIZE + CURVE25519_KEYSIZE + subData.lengthForIndex(1);
        byte controllerInfo[controllerInfoSize];
        memcpy(controllerInfo, controllerKeyData, CURVE25519_KEYSIZE);
        memcpy(&controllerInfo[CURVE25519_KEYSIZE], subData.dataPtrForIndex(1), subData.lengthForIndex(1));
        memcpy(&controllerInfo[CURVE25519_KEYSIZE + subData.lengthForIndex(1)], publicSecretKeyData, CURVE25519_KEYSIZE);

        ed25519_key clKey;
        r = wc_ed25519_init(&clKey);
        if (r)
            hkLog.warn("wc_ed25519_init: r:%d", r);
        r = wc_ed25519_import_public((const byte *)rec.publicKey, ED25519_PUB_KEY_SIZE, &clKey);
        if (r)
            hkLog.warn("wc_ed25519_import_public: r:%d", r);
        int verified = 0;
        r = wc_ed25519_verify_msg((byte *)subData.dataPtrForIndex(10), subData.lengthForIndex(10), (const byte *)controllerInfo, controllerInfoSize, &verified, &clKey);
        if (r)
            hkLog.warn("wc_ed25519_verify_msg: r:%d", r);
        if (verified)
        {
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
            r = wc_HKDF(SHA512, (const byte *)sharedKey, CHACHA20_POLY1305_AEAD_KEYSIZE, (const byte *)salt, strlen(salt), (const byte *)read_info, strlen(read_info), readKey, CHACHA20_POLY1305_AEAD_KEYSIZE);
            if (r)
                hkLog.warn("wc_HKDF: r:%d", r);
            r = wc_HKDF(SHA512, (const byte *)sharedKey, CHACHA20_POLY1305_AEAD_KEYSIZE, (const byte *)salt, strlen(salt), (const byte *)write_info, strlen(write_info), writeKey, CHACHA20_POLY1305_AEAD_KEYSIZE);
            if (r)
                hkLog.warn("wc_HKDF: r:%d", r);
            hkLog.info("Pair verified, secure connection established");
            server->progressPtr(Progress_Pair_Verify_M4);
        }
        else
        {
            HKNetworkMessageDataRecord error;
            error.activate = true;
            error.data = new char[1];
            error.data[0] = 2;
            error.index = 7;
            error.length = 1;
            response.data.addRecord(error);
            hkLog.warn("Pair NOT verified.");
            server->progressPtr(Progress_Error);
        }
    }
    }

    char *responseBuffer = 0;
    int responseLen = 0;
    response.getBinaryPtr(&responseBuffer, &responseLen);
    if (responseBuffer)
    {
        writeData((byte *)responseBuffer, responseLen);

        delete[] responseBuffer;
    }
    else
    {
        hkLog.info("Why empty response");
    }
    return completed;
}

void HKConnection::handlePairSetup(const char *buffer)
{
    bool completed = false;

    HKNetworkMessageDataRecord stateRecord;
    stateRecord.activate = true;
    stateRecord.data = new char[1];
    stateRecord.length = 1;
    stateRecord.index = 6;

    PairSetupState_t state = State_M1_SRPStartRequest;

    char *responseBuffer = 0;
    int responseLen = 0;

    HKNetworkMessage msg = HKNetworkMessage(buffer);
    HKNetworkResponse mResponse(200);

    state = (PairSetupState_t)(*msg.data.dataPtrForIndex(6));
    hkLog.info("State: %d", state);
    *stateRecord.data = (char)state + 1;
    switch (state)
    {
    case State_M1_SRPStartRequest:
    {
      server->progressPtr(Progress_M1_SRPStartRequest);
      hkLog.info("State_M1_SRPStartRequest");
      stateRecord.data[0] = State_M2_SRPStartRespond;
      HKNetworkMessageDataRecord saltRec;
      HKNetworkMessageDataRecord publicKeyRec;

      byte salt[16];
      for (int i = 0; i < 16; i++)
      {
          salt[i] = rand();
      }

      int r = wc_SrpInit(&srp, SRP_TYPE_SHA512, SRP_CLIENT_SIDE);
      hasSrp = true;
      srp.keyGenFunc_cb = wc_SrpSetKeyH;
      if (!r)
          r = wc_SrpSetUsername(&srp, (const byte *)"Pair-Setup", strlen("Pair-Setup"));
      if (r)
          hkLog.warn("wc_SrpSetUsername: r:%d", r);
      if (!r)
          r = wc_SrpSetParams(&srp, (const byte *)N, sizeof(N), (const byte *)generator, 1, salt, 16);
      if (r)
          hkLog.warn("wc_SrpSetParams: r:%d", r);
      if (!r)
          r = wc_SrpSetPassword(&srp, (const byte *)server->getPasscode().c_str(), server->getPasscode().length());
      if (r)
          hkLog.warn("wc_SrpSetPassword: r:%d", r);
      if (!r)
          r = wc_SrpGetVerifier(&srp, (byte *)publicKey, &publicKeyLength); //use publicKey to store v
      if (r)
          hkLog.warn("wc_SrpGetVerifier: r:%d", r);
      srp.side = SRP_SERVER_SIDE; //switch to server mode
      if (!r)
          r = wc_SrpSetVerifier(&srp, (byte *)publicKey, publicKeyLength);
      if (r)
          hkLog.warn("wc_SrpSetVerifier: r:%d", r);
      if (!r)
          r = wc_SrpGetPublic(&srp, (byte *)publicKey, &publicKeyLength);
      if (r)
          hkLog.warn("wc_SrpGetPublic: r:%d", r);
      saltRec.index = 2;
      saltRec.activate = true;
      saltRec.length = sizeof(salt);
      saltRec.data = new char[saltRec.length];
      memcpy(saltRec.data, salt, saltRec.length);
      publicKeyRec.index = 3;
      publicKeyRec.activate = true;
      publicKeyRec.length = publicKeyLength;
      publicKeyRec.data = new char[publicKeyRec.length];
      memcpy(publicKeyRec.data, publicKey, publicKeyRec.length);

      mResponse.data.addRecord(stateRecord);
      mResponse.data.addRecord(publicKeyRec);
      mResponse.data.addRecord(saltRec);
      server->progressPtr(Progress_M2_SRPStartRespond);
    }
    break;

    case State_M3_SRPVerifyRequest:
    {
        hkLog.info("State_M3_SRPVerifyRequest");
        server->progressPtr(Progress_M3_SRPVerifyRequest);
        stateRecord.data[0] = State_M4_SRPVerifyRespond;
        const char *keyStr = 0;
        int keyLen = 0;
        char *proofStr = NULL;
        int proofLen = 0;
        keyStr = msg.data.dataPtrForIndex(3);
        keyLen = msg.data.lengthForIndex(3);
        char *temp = msg.data.dataPtrForIndex(4);
        if (temp != NULL)
        {
            proofStr = temp;
            proofLen = msg.data.lengthForIndex(4);
        }
        else
        {
            hkLog.info("no proof sent!");
        }
        int r = wc_SrpComputeKey(&srp, (byte *)keyStr, keyLen, (byte *)publicKey, publicKeyLength);
        if (r)
            hkLog.warn("wc_SrpComputeKey: r:%d", r);
        r = wc_SrpVerifyPeersProof(&srp, (byte *)proofStr, proofLen);
        if (r)
            hkLog.warn("wc_SrpVerifyPeersProof: r:%d", r);
        if (r != 0)
        { //failed
            HKNetworkMessageDataRecord responseRecord;
            responseRecord.activate = true;
            responseRecord.data = new char[1];
            responseRecord.data[0] = 2;
            responseRecord.index = 7;
            responseRecord.length = 1;
            mResponse.data.addRecord(stateRecord);
            mResponse.data.addRecord(responseRecord);

            hkLog.warn("INCORRECT PASSWORD");

            wc_SrpTerm(&srp);
            server->progressPtr(Progress_Error);
        }
        else
        { //success
            unsigned int srpResponseLength = SRP_RESPONSE_LENGTH;
            char response[SRP_RESPONSE_LENGTH] = {0};
            wc_SrpGetProof(&srp, (byte *)response, &srpResponseLength);
            //SRP_respond(srp, &response);
            HKNetworkMessageDataRecord responseRecord;
            responseRecord.activate = true;
            responseRecord.index = 4;
            responseRecord.length = srpResponseLength;
            responseRecord.data = new char[responseRecord.length];
            memcpy(responseRecord.data, response, responseRecord.length);

            mResponse.data.addRecord(stateRecord);
            mResponse.data.addRecord(responseRecord);
            hkLog.info("Password is correct.");
            server->progressPtr(Progress_M4_SRPVerifyRespond);
        }
    }
    break;
    case State_M5_ExchangeRequest:
    {
        hkLog.info("State_M5_ExchangeRequest");
        server->progressPtr(Progress_M5_ExchangeRequest);
        stateRecord.data[0] = State_M6_ExchangeRespond;
        const char *encryptedPackage = NULL;
        int packageLen = 0;
        encryptedPackage = msg.data.dataPtrForIndex(5);
        packageLen = msg.data.lengthForIndex(5);
        char encryptedData[packageLen];
        memcpy(encryptedData, encryptedPackage, packageLen - 16);
        char mac[16];
        memcpy(mac, &encryptedPackage[packageLen - 16], 16);

        const char salt1[] = "Pair-Setup-Encrypt-Salt";
        const char info1[] = "Pair-Setup-Encrypt-Info";
        uint8_t sharedKey[100];
        int r = wc_HKDF(SHA512, (const byte *)srp.key, srp.keySz, (const byte *)salt1, strlen(salt1), (const byte *)info1, strlen(info1), sharedKey, CHACHA20_POLY1305_AEAD_KEYSIZE);
        if (r)
            hkLog.warn("wc_HKDF: r:%d", r);
        uint8_t decryptedData[packageLen - 16];
        bzero(decryptedData, packageLen - 16);
        r = wc_ChaCha20Poly1305_Decrypt(
            (const byte *)sharedKey,
            (const byte *)"\x0\x0\x0\x0PS-Msg05",
            NULL, 0,
            (const byte *)encryptedData, packageLen - 16,
            (const byte *)mac, decryptedData);
        hkLog.info("wc_ChaCha20Poly1305_Decrypt: r:%d", r);
        HKNetworkMessageData *subTLV8 = new HKNetworkMessageData((char *)decryptedData, packageLen - 16);
        char *controllerIdentifier = subTLV8->dataPtrForIndex(1);
        char *controllerPublicKey = subTLV8->dataPtrForIndex(3);
        size_t controllerPublicKeySize = subTLV8->lengthForIndex(3);
        char *controllerSignature = subTLV8->dataPtrForIndex(10);
        size_t controllerSignatureSize = subTLV8->lengthForIndex(10);
        char controllerHash[100];

        server->persistor->resetPairings();

        HKKeyRecord newRecord;
        memcpy(newRecord.controllerID, controllerIdentifier, 36);
        memcpy(newRecord.publicKey, controllerPublicKey, 32);
        server->persistor->addKey(newRecord);

        const char salt2[] = "Pair-Setup-Controller-Sign-Salt";
        const char info2[] = "Pair-Setup-Controller-Sign-Info";
        r = wc_HKDF(SHA512, (const byte *)srp.key, srp.keySz, (const byte *)salt2, strlen(salt2), (const byte *)info2, strlen(info2), (byte *)controllerHash, CHACHA20_POLY1305_AEAD_KEYSIZE);
        if (r)
            hkLog.warn("wc_HKDF: r:%d", r);
        memcpy(&controllerHash[32], controllerIdentifier, 36);
        memcpy(&controllerHash[68], controllerPublicKey, 32);

        ed25519_key clKey;
        r = wc_ed25519_init(&clKey);
        if (r)
            hkLog.warn("wc_ed25519_init: r:%d", r);
        r = wc_ed25519_import_public((const byte *)controllerPublicKey, controllerPublicKeySize, &clKey);
        if (r)
            hkLog.warn("wc_ed25519_import_public: r:%d", r);
        int verified = 0;
        r = wc_ed25519_verify_msg((byte *)controllerSignature, controllerSignatureSize, (const byte *)controllerHash, 100, &verified, &clKey);
        if (r)
            hkLog.warn("wc_ed25519_verify_msg: r:%d", r);
        if (verified)
        {
            HKNetworkMessageData *returnTLV8 = new HKNetworkMessageData();

            HKNetworkMessageDataRecord usernameRecord;
            usernameRecord.activate = true;
            usernameRecord.index = 1;
            usernameRecord.length = server->getDeviceIdentity().length();
            usernameRecord.data = new char[usernameRecord.length];
            memcpy(usernameRecord.data, server->getDeviceIdentity().c_str(), usernameRecord.length);
            returnTLV8->addRecord(usernameRecord);

            // Generate Signature
            const char salt3[] = "Pair-Setup-Accessory-Sign-Salt";
            const char info3[] = "Pair-Setup-Accessory-Sign-Info";
            size_t outputSize = 64 + server->getDeviceIdentity().length();
            uint8_t output[outputSize];
            r = wc_HKDF(SHA512, (const byte *)srp.key, srp.keySz, (const byte *)salt3, strlen(salt3), (const byte *)info3, strlen(info3), (byte *)output, CHACHA20_POLY1305_AEAD_KEYSIZE);
            if (r)
                hkLog.warn("wc_HKDF: r:%d", r);
            word32 accessoryPubKeySize = ED25519_PUB_KEY_SIZE;
            uint8_t accessoryPubKey[accessoryPubKeySize];
            r = wc_ed25519_export_public(accessoryKey, accessoryPubKey, &accessoryPubKeySize);
            memcpy(&output[32], server->getDeviceIdentity().c_str(), server->getDeviceIdentity().length());
            memcpy(&output[32 + server->getDeviceIdentity().length()], accessoryPubKey, accessoryPubKeySize);
            word32 signatureSize = 64;
            uint8_t signature[signatureSize];

            r = wc_ed25519_sign_msg(output, outputSize, signature, &signatureSize, accessoryKey);
            if (r)
                hkLog.warn("wc_ed25519_sign_msg: r:%d", r);

            HKNetworkMessageDataRecord signatureRecord;
            signatureRecord.activate = true;
            signatureRecord.index = 10;
            signatureRecord.length = 64;
            signatureRecord.data = new char[64];
            memcpy(signatureRecord.data, signature, signatureSize);
            returnTLV8->addRecord(signatureRecord);

            HKNetworkMessageDataRecord publicKeyRecord;
            publicKeyRecord.activate = true;
            publicKeyRecord.index = 3;
            publicKeyRecord.length = accessoryPubKeySize;
            publicKeyRecord.data = new char[accessoryPubKeySize];
            memcpy(publicKeyRecord.data, accessoryPubKey, accessoryPubKeySize);
            returnTLV8->addRecord(publicKeyRecord);
            char *tlv8Data;
            unsigned short tlv8Len;
            returnTLV8->rawData((const char **)&tlv8Data, &tlv8Len);

            HKNetworkMessageDataRecord tlv8Record;
            tlv8Record.data = new char[tlv8Len + 16];
            tlv8Record.length = tlv8Len + 16;
            bzero(tlv8Record.data, tlv8Record.length);

            r = wc_ChaCha20Poly1305_Encrypt(
                (const byte *)sharedKey,
                (const byte *)"\x0\x0\x0\x0PS-Msg06",
                NULL, 0,
                (const byte *)tlv8Data, tlv8Len,
                (byte *)tlv8Record.data,
                (byte *)(tlv8Record.data + tlv8Len));
            if (r)
                hkLog.warn("wc_ChaCha20Poly1305_Encrypt: r:%d", r);

            tlv8Record.activate = true;
            tlv8Record.index = 5; //5

            mResponse.data.addRecord(stateRecord);
            mResponse.data.addRecord(tlv8Record);
            completed = true;
            delete returnTLV8;
            free(tlv8Data);
            server->progressPtr(Progress_M6_ExchangeRespond);
        }
        else
        {
            server->progressPtr(Progress_Error);
        }

        delete subTLV8;
        wc_SrpTerm(&srp);
    }
    break;
    }
    mResponse.getBinaryPtr(&responseBuffer, &responseLen);
    if (responseBuffer)
    {
        writeData((byte *)responseBuffer, responseLen);

        delete[] responseBuffer;
    }
    else
    {
        hkLog.info("Why empty response");
    }
    if (completed)
    {
        hkLog.info("Pairing completed.");
    }
}

void HKConnection::handleAccessoryRequest(const char *buffer, size_t size)
{
    unsigned int resultLen = 0;
    server->progressPtr(Progress_AccessoryRequest);
    handleAccessory(buffer, size, (char *)SHARED_RESPONSE_BUFFER, SHARED_RESPONSE_BUFFER_LEN, &resultLen, this);
    server->progressPtr(Progress_AccessoryRespond);
    if (resultLen > 0)
    {
        writeData(SHARED_RESPONSE_BUFFER, resultLen);
        hkLog.info("Response to: %s, data:%s", clientID(), SHARED_RESPONSE_BUFFER);
        Serial.println((const char*)SHARED_RESPONSE_BUFFER);
    }
}


void HKConnection::postCharacteristicsValue(characteristics *c)
{
    for (uint i = 0; i < postedCharacteristics.size(); i++)
    {
        characteristics *item = postedCharacteristics.at(i);
        if (c == item)
        { //already registered
            return;
        }
    }
    postedCharacteristics.push_back(c);
}

void HKConnection::addNotifiedCharacteristics(characteristics *c)
{
    notifiableCharacteristics.push_back(c);
}
