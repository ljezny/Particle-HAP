//
//  HKAccessory.c
//  Workbench
//
//  Created by Wai Man Chan on 9/27/14.
//
//

#include "HKAccessory.h"
#include "HKServer.h"
#include "HKLog.h"

const char hapJsonType[] = "application/hap+json";
const char pairingTlv8Type[] = "application/pairing+tlv8";


//Wrap to JSON
inline string wrap(const char *str) { return (string)"\""+str+"\""; }
//Value String
string attribute(unsigned int type, unsigned short acclaim, int p, string value) {
    string result;
    if (p & permission_read) {
        result += wrap("value")+":";
        result += value;
        result += ",";
    }

    result += wrap("perms")+":";
    result += "[";
    if (p & permission_read) result += wrap("pr")+",";
    if (p & permission_write) result += wrap("pw")+",";
    if (p & permission_notify) result += wrap("ev")+",";
    result = result.substr(0, result.size()-1);
    result += "]";
    result += ",";

    char tempStr[4];
    snprintf(tempStr, 4, "%X", type);
    result += wrap("type")+":"+wrap(tempStr);
    result += ",";

    snprintf(tempStr, 4, "%hd", acclaim);
    result += wrap("iid")+":"+tempStr;
    result += ",";

    result += "\"format\":\"bool\"";

    return "{"+result+"}";
}
string attribute(unsigned int type, unsigned short acclaim, int p, string value, int minVal, int maxVal, int step, unit valueUnit) {
    string result;
    char tempStr[16];

    if (p & permission_read) {
        result += wrap("value")+":"+value;
        result += ",";
    }

    snprintf(tempStr, 16, "%d", minVal);
    if (minVal != INT32_MIN)
        result += wrap("minValue")+":"+tempStr+",";

    snprintf(tempStr, 16, "%d", maxVal);
    if (maxVal != INT32_MAX)
        result += wrap("maxValue")+":"+tempStr+",";

    snprintf(tempStr, 16, "%d", step);
    if (step > 0)
        result += wrap("minStep")+":"+tempStr+",";

    result += wrap("perms")+":";
    result += "[";
    if (p & permission_read) result += wrap("pr")+",";
    if (p & permission_write) result += wrap("pw")+",";
    if (p & permission_notify) result += wrap("ev")+",";
    result = result.substr(0, result.size()-1);
    result += "]";
    result += ",";

    snprintf(tempStr, 16, "%X", type);
    result += wrap("type")+":"+wrap(tempStr);
    result += ",";

    snprintf(tempStr, 16, "%hd", acclaim);
    result += wrap("iid")+":"+tempStr;
    result += ",";

    switch (valueUnit) {
        case unit_arcDegree:
            result += wrap("unit")+":"+wrap("arcdegrees")+",";
            break;
        case unit_celsius:
            result += wrap("unit")+":"+wrap("celsius")+",";
            break;
        case unit_percentage:
            result += wrap("unit")+":"+wrap("percentage")+",";
            break;
        case unit_lux:
            result += wrap("unit")+":"+wrap("lux")+",";
            break;
    }

    result += "\"format\":\"int\"";

    return "{"+result+"}";
}
string attribute(unsigned int type, unsigned short acclaim, int p, string value, float minVal, float maxVal, float step, unit valueUnit) {
    string result;
    char tempStr[16];

    if (p & permission_read) {
        result += wrap("value")+":"+value;
        result += ",";
    }

    snprintf(tempStr, 16, "%f", minVal);
    if (minVal != INT32_MIN)
        result += wrap("minValue")+":"+tempStr+",";

    snprintf(tempStr, 16, "%f", maxVal);
    if (maxVal != INT32_MAX)
        result += wrap("maxValue")+":"+tempStr+",";

    snprintf(tempStr, 16, "%f", step);
    if (step > 0)
        result += wrap("minStep")+":"+tempStr+",";

    result += wrap("perms")+":";
    result += "[";
    if (p & permission_read) result += wrap("pr")+",";
    if (p & permission_write) result += wrap("pw")+",";
    if (p & permission_notify) result += wrap("ev")+",";
    result = result.substr(0, result.size()-1);
    result += "]";
    result += ",";

    snprintf(tempStr, 16, "%X", type);
    result += wrap("type")+":"+wrap(tempStr);
    result += ",";

    snprintf(tempStr, 16, "%hd", acclaim);
    result += wrap("iid")+":"+tempStr;
    result += ",";

    switch (valueUnit) {
        case unit_arcDegree:
            result += wrap("unit")+":"+wrap("arcdegrees")+",";
            break;
        case unit_celsius:
            result += wrap("unit")+":"+wrap("celsius")+",";
            break;
        case unit_percentage:
            result += wrap("unit")+":"+wrap("percentage")+",";
            break;
        case unit_lux:
            result += wrap("unit")+":"+wrap("lux")+",";
            break;
    }

    result += "\"format\":\"float\"";

    return "{"+result+"}";
}
//Raw value
string attribute(unsigned int type, unsigned short acclaim, int p, bool value) {
    string result;
    if (p & permission_read) {
        result += wrap("value")+":";
        if (value) result += "true";
        else result += "false";
        result += ",";
    }

    result += wrap("perms")+":";
    result += "[";
    if (p & permission_read) result += wrap("pr")+",";
    if (p & permission_write) result += wrap("pw")+",";
    if (p & permission_notify) result += wrap("ev")+",";
    result = result.substr(0, result.size()-1);
    result += "]";
    result += ",";

    char tempStr[4];
    snprintf(tempStr, 4, "%X", type);
    result += wrap("type")+":"+wrap(tempStr);
    result += ",";

    snprintf(tempStr, 4, "%hd", acclaim);
    result += wrap("iid")+":"+tempStr;
    result += ",";

    result += "\"format\":\"bool\"";

    return "{"+result+"}";
}
string attribute(unsigned int type, unsigned short acclaim, int p, int value, int minVal, int maxVal, int step, unit valueUnit) {
    string result;
    char tempStr[16];

    snprintf(tempStr, 16, "%d", value);

    if (p & permission_read) {
        result += wrap("value")+":"+tempStr;
        result += ",";
    }

    snprintf(tempStr, 16, "%d", minVal);
    if (minVal != INT32_MIN)
        result += wrap("minValue")+":"+tempStr+",";

    snprintf(tempStr, 16, "%d", maxVal);
    if (maxVal != INT32_MAX)
        result += wrap("maxValue")+":"+tempStr+",";

    snprintf(tempStr, 16, "%d", step);
    if (step > 0)
        result += wrap("minStep")+":"+tempStr+",";

    result += wrap("perms")+":";
    result += "[";
    if (p & permission_read) result += wrap("pr")+",";
    if (p & permission_write) result += wrap("pw")+",";
    if (p & permission_notify) result += wrap("ev")+",";
    result = result.substr(0, result.size()-1);
    result += "]";
    result += ",";

    snprintf(tempStr, 16, "%X", type);
    result += wrap("type")+":"+wrap(tempStr);
    result += ",";

    snprintf(tempStr, 16, "%hd", acclaim);
    result += wrap("iid")+":"+tempStr;
    result += ",";

    switch (valueUnit) {
        case unit_arcDegree:
            result += wrap("unit")+":"+wrap("arcdegrees")+",";
            break;
        case unit_celsius:
            result += wrap("unit")+":"+wrap("celsius")+",";
            break;
        case unit_percentage:
            result += wrap("unit")+":"+wrap("percentage")+",";
            break;
        case unit_lux:
            result += wrap("unit")+":"+wrap("lux")+",";
            break;
    }

    result += "\"format\":\"int\"";

    return "{"+result+"}";
}
string attribute(unsigned int type, unsigned short acclaim, int p, float value, float minVal, float maxVal, float step, unit valueUnit) {
    string result;
    char tempStr[16];

    snprintf(tempStr, 16, "%f", value);

    if (p & permission_read) {
        result += wrap("value")+":"+tempStr;
        result += ",";
    }

    snprintf(tempStr, 16, "%f", minVal);
    if (minVal != INT32_MIN)
        result += wrap("minValue")+":"+tempStr+",";

    snprintf(tempStr, 16, "%f", maxVal);
    if (maxVal != INT32_MAX)
        result += wrap("maxValue")+":"+tempStr+",";

    snprintf(tempStr, 16, "%f", step);
    if (step > 0)
        result += wrap("minStep")+":"+tempStr+",";

    result += wrap("perms")+":";
    result += "[";
    if (p & permission_read) result += wrap("pr")+",";
    if (p & permission_write) result += wrap("pw")+",";
    if (p & permission_notify) result += wrap("ev")+",";
    result = result.substr(0, result.size()-1);
    result += "]";
    result += ",";

    snprintf(tempStr, 16, "%X", type);
    result += wrap("type")+":"+wrap(tempStr);
    result += ",";

    snprintf(tempStr, 16, "%hd", acclaim);
    result += wrap("iid")+":"+tempStr;
    result += ",";

    switch (valueUnit) {
        case unit_arcDegree:
            result += wrap("unit")+":"+wrap("arcdegrees")+",";
            break;
        case unit_celsius:
            result += wrap("unit")+":"+wrap("celsius")+",";
            break;
        case unit_percentage:
            result += wrap("unit")+":"+wrap("percentage")+",";
            break;
        case unit_lux:
            result += wrap("unit")+":"+wrap("lux")+",";
            break;
    }

    result += "\"format\":\"float\"";

    return "{"+result+"}";
}

string attribute(unsigned int type, unsigned short acclaim, int p, string value, unsigned short len) {
    string result;
    char tempStr[4];

    if (p & permission_read) {
        result += wrap("value")+":"+value.c_str();
        result += ",";
    }

    result += wrap("perms")+":";
    result += "[";
    if (p & permission_read) result += wrap("pr")+",";
    if (p & permission_write) result += wrap("pw")+",";
    if (p & permission_notify) result += wrap("ev")+",";
    result = result.substr(0, result.size()-1);
    result += "]";
    result += ",";

    snprintf(tempStr, 4, "%X", type);
    result += wrap("type")+":"+wrap(tempStr);
    result += ",";

    snprintf(tempStr, 4, "%hd", acclaim);
    result += wrap("iid")+":"+tempStr;
    result += ",";

    if (len > 0) {
        snprintf(tempStr, 4, "%hd", len);
        result += wrap("maxLen")+":"+tempStr;
        result += ",";
    }

    result += "\"format\":\"string\"";

    return "{"+result+"}";
}
string arrayWrap(string *s, unsigned short len) {
    string result = "";
    result+="[";
    for (int i = 0; i < len; i++) {
        result+=s[i];
        if(i != len - 1) { //ommit last ,
          result+=",";
        }
    }
    result+="]";

    return result;
}

string dictionaryWrap(string *key, string *value, unsigned short len) {
  string result = "";
  result+="{";
  for (int i = 0; i < len; i++) {
      result+=wrap(key[i].c_str())+":"+value[i];
      if(i != len - 1) { //ommit last ,
        result+=",";
      }
  }
  result+="}";

  return result;
}

void characteristics::notify(HKConnection* conn) {
  for(int i = 0; i<notifiedConnections.size();i++){
      if(conn != notifiedConnections.at(i)) {
          notifiedConnections.at(i)->postCharacteristicsValue(this);
      }
  }
}

string boolCharacteristics::describe(HKConnection *sender) {
    string result = attribute(type, iid, permission, value(sender));
    //hkLog.info("boolCharacteristics::describe: %s",result.c_str());
    return result;
}

string floatCharacteristics::describe(HKConnection *sender) {
    string result = attribute(type, iid, permission, value(sender), _minVal, _maxVal, _step, _unit);
    //hkLog.info("boolCharacteristics::describe: %s",result.c_str());
    return result;
}

string intCharacteristics::describe(HKConnection *sender) {
    string result = attribute(type, iid, permission, value(sender), _minVal, _maxVal, _step, _unit);
    //hkLog.info("boolCharacteristics::describe: %s",result.c_str());
    return result;
}

string stringCharacteristics::describe(HKConnection *sender) {
    string result = attribute(type, iid, permission, value(sender), maxLen);
    //hkLog.info("boolCharacteristics::describe: %s",result.c_str());
    return result;
}

void Service::describe(HKConnection *sender, HKStringBuffer &result) {
    result+="{";

    char serviceIDStr[32];
    sprintf(serviceIDStr, "\"iid\":%d,", serviceID);
    result+=serviceIDStr;

    char uuidStr[32];
    sprintf(uuidStr, "\"type\":\"%X\",", uuid);
    result+=uuidStr;

    result+="\"characteristics\":[";

    int len = numberOfCharacteristics();
    for (int i = 0; i < len; i++) {
        result+=_characteristics[i]->describe(sender);
        if(i != len - 1) { //ommit last ,
            result+=",";
        }
    }

    result+="]";

    result+="}";
}

void Accessory::describe(HKConnection *sender, HKStringBuffer &result) {
  result+="{";

  result+="\"services\":[";

  int len = numberOfService();
  for (int i = 0; i < len; i++) {
      _services[i]->describe(sender,result);
      if(i != len - 1) { //ommit last ,
        result+=",";
      }
  }

  result+="]";
  result+=",";

  char temp[32];
  sprintf(temp, "\"aid\":%d", aid);
  result+=temp;

  result+="}";
}

void AccessorySet::describe(HKConnection *sender, HKStringBuffer &result) {
    result+="{";

    result+="\"accessories\":[";

    int numberOfAcc = numberOfAccessory();
    for (int i = 0; i < numberOfAcc; i++) {
        _accessories[i]->describe(sender,result);
        if(i != numberOfAcc - 1) { //ommit last ,
          result+=",";
        }
    }

    result+="]";

    result+="}";

}

const int HEADER_SIZE = 128;
char header[HEADER_SIZE] = {0}; //preallocate header with spaces
void handleAccessory(const char *request, unsigned int requestLen, char *responseBuffer,int responseBufferLen, unsigned int *replyLen, HKConnection *sender) {
    //reply should add '\0', or the printf is incorrect
    memset(responseBuffer, 0, responseBufferLen);
    memset(header, ' ', HEADER_SIZE); //initialize with empty spaces
    memcpy(header + HEADER_SIZE - 4, "\r\n\r\n", 4); //add empty line to end of header
    
    int index = 5;
    char method[5];

    //Read method
    method[4] = 0;
    bcopy(request, method, 4);
    if (method[3] == ' ') {
        method[3] = 0;
        index = 4;
    }

    char path[1024];
    int i;
    for (i = 0; i < 1024 && request[index] != ' '; i++, index++) {
        path[i] = request[index];
    }
    path[i] = 0;
    hkLog.info("Path: %s", path);

    const char *dataPtr = request;
    while (true) {
        dataPtr = &dataPtr[1];
        if (dataPtr[0] == '\r' && dataPtr[1] == '\n' && dataPtr[2] == '\r' && dataPtr[3] == '\n') break;
    }

    dataPtr += 4;

    char *replyData = responseBuffer + HEADER_SIZE;
    unsigned short replyDataLen = 0;

    int statusCode = 0;

    const char *protocol = "HTTP/1.1";
    const char *returnType = hapJsonType;

    if (strcmp(path, "/accessories") == 0) {
        //Publish the characterists of the accessories
        hkLog.info("Ask for accessories info");
        statusCode = 200;

        HKStringBuffer buffer = HKStringBuffer(replyData,responseBufferLen - HEADER_SIZE);
      
        AccessorySet::getInstance().describe(sender,buffer);
        replyDataLen = buffer.size();
        hkLog.info("Accessories description len:%d",replyDataLen);
    } else if (strcmp(path, "/pairings") == 0) {
        HKNetworkMessage msg(request);
        statusCode = 200;
        hkLog.info("%d", *msg.data.dataPtrForIndex(0));
        if (*msg.data.dataPtrForIndex(0) == 3) {
            //Pairing with new user

            hkLog.info("Add new user");

            HKKeyRecord controllerRec;
            bcopy(msg.data.dataPtrForIndex(3), controllerRec.publicKey, 32);
            bcopy(msg.data.dataPtrForIndex(1), controllerRec.controllerID, 36);
            sender->server->persistor->addKey(controllerRec);
            HKNetworkMessageDataRecord drec;
            drec.activate = true; drec.data = new char[1]; *drec.data = 2;
            drec.index = 6; drec.length = 1;
            HKNetworkMessageData data;
            data.addRecord(drec);
            data.rawData((const char **)&replyData, &replyDataLen);
            returnType = pairingTlv8Type;
            statusCode = 200;
        } else {

            hkLog.info("Delete user");

            HKKeyRecord controllerRec;
            bcopy(msg.data.dataPtrForIndex(1), controllerRec.controllerID, 36);
            sender->server->persistor->removeKey(controllerRec);
            HKNetworkMessageDataRecord drec;
            drec.activate = true; drec.data = new char[1]; *drec.data = 2;
            drec.index = 6; drec.length = 1;
            HKNetworkMessageData data;
            data.addRecord(drec);
            data.rawData((const char **)&replyData, &replyDataLen);
            returnType = pairingTlv8Type;
            statusCode = 200;
        }
        //Pairing status change, so update
        //updatePairable();
    } else if (strncmp(path, "/characteristics", 16) == 0){
        hkLog.info("Characteristics");
        if (strncmp(method, "GET", 3) == 0) {
            //Read characteristics
            int aid = 0;    int iid = 0;

            char indexBuffer[1000];
            sscanf(path, "/characteristics?id=%[^\n]", indexBuffer);

            hkLog.info("Get characteristics %s with len %d", indexBuffer, strlen(indexBuffer));

            statusCode = 404;

            HKStringBuffer result = HKStringBuffer(replyData,responseBufferLen - HEADER_SIZE);
            result.append("{\"characteristics\":[");
            int i = 0;
            while (strlen(indexBuffer) > 0) {

                hkLog.info("Get characteristics %s with len %d", indexBuffer, strlen(indexBuffer));

                char temp[1000];
                //Initial the temp
                temp[0] = 0;
                sscanf(indexBuffer, "%d.%d%[^\n]", &aid, &iid, temp);
                hkLog.info("Get temp %s with len %d", temp, strlen(temp));
                strncpy(indexBuffer, temp, 1000);
                hkLog.info("Get characteristics %s with len %d", indexBuffer, strlen(indexBuffer));
                //Trim comma
                if (indexBuffer[0] == ',') {
                    indexBuffer[0] = '0';
                }

                Accessory *a = AccessorySet::getInstance().accessoryAtIndex(aid);
                
                if (a != NULL) {
                    characteristics *c = a->characteristicsAtIndex(iid);
                    if (c != NULL) {
                        hkLog.info("Ask for one characteristics: %d . %d", aid, iid);
                        char c1[3], c2[3];
                        sprintf(c1, "%d", aid);
                        sprintf(c2, "%d", iid);
                        string s[3] = {string(c1), string(c2), c->value(sender)};
                        string k[3] = {"aid", "iid", "value"};
                        if (i++ != 0) {
                            result.append(",");
                        }

                       result.append(dictionaryWrap(k, s, 3));
                    }
                }
            }

            result.append("]}");

            replyDataLen = result.size();
            statusCode = 200;

        } else if (strncmp(method, "PUT", 3) == 0) {
            //Change characteristics
            hkLog.info("PUT characteristics: %s",dataPtr);

            char characteristicsBuffer[1000];
            sscanf(dataPtr, "{\"characteristics\":[{%[^]]s}", characteristicsBuffer);

            char *buffer2 = characteristicsBuffer;
            while (strlen(buffer2) && statusCode != 400) {
                bool reachLast = false; bool updateNotify = false;
                char *buffer1;
                buffer1 = strtok_r(buffer2, "}", &buffer2);
                if (*buffer2 != 0) buffer2+=2;

                int aid = 0;    int iid = 0; char value[16];
                int result = sscanf(buffer1, "\"aid\":%d,\"iid\":%d,\"value\":%s", &aid, &iid, value);
                if (result == 2) {
                    sscanf(buffer1, "\"aid\":%d,\"iid\":%d,\"ev\":%s", &aid, &iid, value);
                    updateNotify = true;
                } else if (result == 0) {
                    sscanf(buffer1, "\"remote\":true,\"value\":%[^,],\"aid\":%d,\"iid\":%d", value, &aid, &iid);
                    if (result == 2) {
                        sscanf(buffer1, "\"remote\":true,\"aid\":%d,\"iid\":%d,\"ev\":%s", &aid, &iid, value);
                        updateNotify = true;
                    }
                    sender->relay = true;
                }
                hkLog.info("%d . %d",aid, iid);

                Accessory *a = AccessorySet::getInstance().accessoryAtIndex(aid);
                if (a==NULL) {
                    statusCode = 400;
                } else {
                    characteristics *c = a->characteristicsAtIndex(iid);

                    if (updateNotify) {
                        hkLog.info("Ask to notify one characteristics: %d . %d -> %s", aid, iid, value);
                        if (c==NULL) {
                            statusCode = 400;
                        } else {
                            if (c->notifiable()) {
                                if (strncmp(value, "1", 1)==0 || strncmp(value, "true", 4) == 0)
                                  c->addNotifiedConnection(sender);
                                else
                                  c->removeNotifiedConnection(sender);
                                statusCode = 204;
                            } else {
                                statusCode = 400;
                            }
                        }
                    } else {
                        hkLog.info("Ask to change one characteristics: %d . %d -> %s", aid, iid, value);
                        if (c==NULL) {
                            statusCode = 400;
                        } else {
                            if (c->writable()) {
                                c->setValue(value, sender);

                                c->notify(sender);

                                statusCode = 204;

                            } else {
                                statusCode = 400;
                            }
                        }
                    }

                }

            }

        } else {
            return;
        }
    } else {
        hkLog.info("Ask for something I don't know");
        hkLog.info("%s", request);
        hkLog.info("%s", path);
        statusCode = 404;
    }
    int len =snprintf(header, HEADER_SIZE, "%s %d OK\r\nContent-Type: %s\r\nContent-Length: %u", protocol, statusCode, returnType, replyDataLen);
    header[len] = ' ';
    memcpy(responseBuffer, header, HEADER_SIZE);
    //memcpy(responseBuffer + len, replyData, replyDataLen);
    (*replyLen) = HEADER_SIZE+replyDataLen;
}

void addInfoServiceToAccessory(Accessory *acc, string accName, string manufactuerName, string modelName, string serialNumber,string firmware, identifyFunction identifyCallback) {
    Service *infoService = new Service(serviceType_accessoryInfo);
    acc->addService(infoService);

    stringCharacteristics *accNameCha = new stringCharacteristics(charType_serviceName, permission_read, 0);
    accNameCha->characteristics::setValue(accName);
    acc->addCharacteristics(infoService, accNameCha);

    stringCharacteristics *manNameCha = new stringCharacteristics(charType_manufactuer, permission_read, 0);
    manNameCha->characteristics::setValue(manufactuerName);
    acc->addCharacteristics(infoService, manNameCha);

    stringCharacteristics *modelNameCha = new stringCharacteristics(charType_modelName, permission_read, 0);
    modelNameCha->characteristics::setValue(modelName);
    acc->addCharacteristics(infoService, modelNameCha);

    stringCharacteristics *serialNameCha = new stringCharacteristics(charType_serialNumber, permission_read, 0);
    serialNameCha->characteristics::setValue(serialNumber);
    acc->addCharacteristics(infoService, serialNameCha);

    stringCharacteristics *fwNameCha = new stringCharacteristics(charType_firmwareRevision, permission_read, 0);
    fwNameCha->characteristics::setValue(firmware);
    acc->addCharacteristics(infoService, fwNameCha);

    boolCharacteristics *identify = new boolCharacteristics(charType_identify, permission_write);
    identify->characteristics::setValue("false");
    identify->valueChangeFunctionCall = identifyCallback;
    acc->addCharacteristics(infoService, identify);
}
