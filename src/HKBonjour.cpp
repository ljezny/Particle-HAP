//  Copyright (C) 2010 Georg Kaindl
//  http://gkaindl.com
//
//  Particle.io Port Copyright (c) 2017 Mobile FLow LLC
//  http://github/moflome
//
//  This file is part of Arduino EthernetBonjour.
//
//  EthernetBonjour is free software: you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public License
//  as published by the Free Software Foundation, either version 3 of
//  the License, or (at your option) any later version.
//
//  EthernetBonjour is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with EthernetBonjour. If not, see
//  <http://www.gnu.org/licenses/>.
//

#define  HAS_SERVICE_REGISTRATION      1  // disabling saves about 1.25 kilobytes
#define  HAS_NAME_BROWSING             1  // disable together with above, additionally saves about 4.3 kilobytes

#include <string.h>
#include <stdlib.h>

#include "HKBonjour.h"
#include "HKLog.h"
#define  MDNS_DEFAULT_NAME       "arduino"
#define  MDNS_TLD                ".local"
#define  DNS_SD_SERVICE          "_services._dns-sd._udp.local"
#define  MDNS_SERVER_PORT        5353
#define  MDNS_NQUERY_RESEND_TIME 1000   // 1 second, name query resend timeout
#define  MDNS_SQUERY_RESEND_TIME 1000  // 10 seconds, service query resend timeout
#define  MDNS_SRV_TTL       120
#define  MDNS_TXT_TTL       1400
#define  MDNS_PTR_TTL       1400

#define  MDNS_MAX_SERVICES_PER_PACKET  6
#define  MSNS_ANNOUNCE_TIME_SEC  1 //Send announce packet every 1 seconds

// This is from Arduino Ethernet library types.h, and is included here to remove compiler warnings...
#define	_ENDIAN_LITTLE_	0	/**<  This must be defined if system is little-endian alignment */
#define	_ENDIAN_BIG_		1
#define 	SYSTEM_ENDIAN		_ENDIAN_LITTLE_

static uint8_t mdnsMulticastIPAddr[] = { 224, 0, 0, 251 };
//static uint8_t mdnsHWAddr[] = { 0x01, 0x00, 0x5e, 0x00, 0x00, 0xfb };



typedef enum _MDNSPacketType_t {
   MDNSPacketTypeMyIPAnswer,
   MDNSPacketTypeNoIPv6AddrAvailable,
   MDNSPacketTypeServiceRecord,
   MDNSPacketTypeServiceRecordRelease,
   MDNSPacketTypeNameQuery,
   MDNSPacketTypeServiceQuery,
} MDNSPacketType_t;

typedef struct _DNSHeader_t {
   uint16_t    xid;
   uint8_t     recursionDesired:1;
   uint8_t     truncated:1;
   uint8_t     authoritiveAnswer:1;
   uint8_t     opCode:4;
   uint8_t     queryResponse:1;
   uint8_t     responseCode:4;
   uint8_t     checkingDisabled:1;
   uint8_t     authenticatedData:1;
   uint8_t     zReserved:1;
   uint8_t     recursionAvailable:1;
   uint16_t    queryCount;
   uint16_t    answerCount;
   uint16_t    authorityCount;
   uint16_t    additionalCount;
} __attribute__((__packed__)) DNSHeader_t;

typedef enum _DNSOpCode_t {
   DNSOpQuery     = 0,
   DNSOpIQuery    = 1,
   DNSOpStatus    = 2,
   DNSOpNotify    = 4,
   DNSOpUpdate    = 5
} DNSOpCode_t;


HKBonjour::HKBonjour()
{
   memset(&this->_mdnsData, 0, sizeof(MDNSDataInternal_t));
   memset(&this->_serviceRecord, 0, sizeof(this->_serviceRecord));

   this->_state = MDNSStateIdle;

   this->_resolveNames[0] = NULL;
   this->_resolveNames[1] = NULL;

   this->_lastAnnounceMillis = 0;
}

HKBonjour::~HKBonjour()
{
	this->stop();
}


int HKBonjour::setUDP( UDP * localUDP )
{
      this->_localUDP = localUDP;
      return 0;
}
int HKBonjour::stop()
{
    UDP *udp = this->_localUDP;

	udp->stop();

      return 0;
}
int HKBonjour::beginMulticast(uint8_t *IPAddr, uint16_t port)
{
    UDP *udp = this->_localUDP;

	udp->begin(5353);    // was... 4097
	udp->beginPacket(IPAddr, 5353);

      return 0;
}
int HKBonjour::write(uint8_t *buf, int size)
{
    UDP *udp = this->_localUDP;

	udp->write(buf, size);

      return 0;
}
int HKBonjour::read(uint8_t *buf, int size)
{
    UDP *udp = this->_localUDP;

      udp->setBuffer(size, buf); // application provided buffer

      return udp->read(buf, size);

}
int HKBonjour::beginPacket(uint8_t * IPAddr, uint16_t port)
{
    UDP *udp = this->_localUDP;
    udp->beginPacket(IPAddr, 5353);
    return 0;
}
int HKBonjour::endPacket()
{
    UDP *udp = this->_localUDP;

	udp->endPacket();

      return 0;
}
int HKBonjour::parsePacket()
{
    UDP *udp = this->_localUDP;

    return udp->parsePacket();
}
int HKBonjour::flush()
{
    UDP *udp = this->_localUDP;

    udp->flush();

      return 0;
}
int HKBonjour::remotePort()
{
    UDP *udp = this->_localUDP;

    return udp->remotePort();
}
int HKBonjour::remoteIP()
{
    UDP *udp = this->_localUDP;

    return udp->remoteIP();
}
/*unsigned long HKBonjour::localIP()
{
  IPAddress localIP = WiFi.localIP();

  return (unsigned long)localIP;
}*/

void HKBonjour::begin(const char* bonjourName)
{
	this->setBonjourName(bonjourName);
    this->beginMulticast(mdnsMulticastIPAddr, MDNS_SERVER_PORT);
}

void HKBonjour::begin()
{
    this->begin(MDNS_DEFAULT_NAME);
}

// return value:
// A DNSError_t (DNSSuccess on success, something else otherwise)
// in "int" mode: positive on success, negative on error
MDNSError_t HKBonjour::_sendMDNSMessage(uint32_t peerAddress, uint32_t xid, int type)
{
   MDNSError_t statusCode = MDNSSuccess;
   uint16_t ptr = 0;
   DNSHeader_t dnsHeader;
   uint8_t* buf;

   memset(&dnsHeader, 0, sizeof(DNSHeader_t));

   dnsHeader.xid = ethutil_htons(xid);
   dnsHeader.opCode = DNSOpQuery;

   switch (type) {
      case MDNSPacketTypeServiceRecordRelease:
      case MDNSPacketTypeMyIPAnswer:
         dnsHeader.answerCount = ethutil_htons(1);
         dnsHeader.queryResponse = 1;
         dnsHeader.authoritiveAnswer = 1;
         break;
      case MDNSPacketTypeServiceRecord:
         dnsHeader.answerCount = ethutil_htons(4);
         dnsHeader.additionalCount = ethutil_htons(1);
         dnsHeader.queryResponse = 1;
         dnsHeader.authoritiveAnswer = 1;
         break;
      case MDNSPacketTypeNameQuery:
      case MDNSPacketTypeServiceQuery:
         dnsHeader.queryCount = ethutil_htons(1);
         break;
      case MDNSPacketTypeNoIPv6AddrAvailable:
         dnsHeader.queryCount = ethutil_htons(1);
         dnsHeader.additionalCount = ethutil_htons(1);
         dnsHeader.responseCode = 0x03;
         dnsHeader.authoritiveAnswer = 1;
         dnsHeader.queryResponse = 1;
         break;
   }




   this->beginPacket(mdnsMulticastIPAddr,MDNS_SERVER_PORT);
   this->write((uint8_t*)&dnsHeader,sizeof(DNSHeader_t));

   ptr += sizeof(DNSHeader_t);
   buf = (uint8_t*)&dnsHeader;

   // construct the answer section
   switch (type) {
      case MDNSPacketTypeMyIPAnswer: {
         this->_writeMyIPAnswerRecord(&ptr, buf, sizeof(DNSHeader_t));
         break;
      }

#if defined(HAS_SERVICE_REGISTRATION) && HAS_SERVICE_REGISTRATION

      case MDNSPacketTypeServiceRecord: {

         // SRV location record
         this->_writeServiceRecordName(&ptr, buf, sizeof(DNSHeader_t), 0);

         buf[0] = 0x00;
         buf[1] = 0x21;    // SRV record
         buf[2] = 0x80;    // cache flush
         buf[3] = 0x01;    // class IN

         // ttl
         *((uint32_t*)&buf[4]) = ethutil_htonl(MDNS_SRV_TTL);

         // data length
         *((uint16_t*)&buf[8]) = ethutil_htons(8 + strlen((char*)this->_bonjourName));

         this->write((uint8_t*)buf,10);
         ptr += 10;
         // priority and weight
         buf[0] = buf[1] = buf[2] = buf[3] = 0;

         // port
         *((uint16_t*)&buf[4]) = ethutil_htons(this->_serviceRecord.port);

         this->write((uint8_t*)buf,6);
         ptr += 6;
         // target
         this->_writeDNSName(this->_bonjourName, &ptr, buf, sizeof(DNSHeader_t), 1);

         // TXT record
         this->_writeServiceRecordName(&ptr, buf, sizeof(DNSHeader_t), 0);

         buf[0] = 0x00;
         buf[1] = 0x10;    // TXT record
         buf[2] = 0x80;    // cache flush
         buf[3] = 0x01;    // class IN

         // ttl
         *((uint32_t*)&buf[4]) = ethutil_htonl(MDNS_TXT_TTL);

         this->write((uint8_t*)buf,8);
         ptr += 8;

         // data length && text
         if (NULL == this->_serviceRecord.textContent) {
            buf[0] = 0x00;
            buf[1] = 0x01;
            buf[2] = 0x00;

            this->write((uint8_t*)buf,3);
            ptr += 3;
         } else {
            int slen = strlen((char*)this->_serviceRecord.textContent);
            *((uint16_t*)buf) = ethutil_htons(slen);
            this->write((uint8_t*)buf,2);
            ptr += 2;

            this->write((uint8_t*)this->_serviceRecord.textContent,slen);
            ptr += slen;
         }

         // PTR record (for the dns-sd service in general)
         this->_writeDNSName((const uint8_t*)DNS_SD_SERVICE, &ptr, buf,
                                          sizeof(DNSHeader_t), 1);

         buf[0] = 0x00;
         buf[1] = 0x0c;    // PTR record
         buf[2] = 0x00;    // no cache flush
         buf[3] = 0x01;    // class IN

         // ttl
         *((uint32_t*)&buf[4]) = ethutil_htonl(MDNS_PTR_TTL);

         // data length.
         uint16_t dlen = strlen((char*)this->_serviceRecord.servName) + 2;
         *((uint16_t*)&buf[8]) = ethutil_htons(dlen);

         this->write((uint8_t*)buf, 10);
         ptr += 10;

         this->_writeServiceRecordName(&ptr, buf, sizeof(DNSHeader_t), 1);

         // PTR record (our service)
         this->_writeServiceRecordPTR(&ptr, buf, sizeof(DNSHeader_t),
                                      MDNS_PTR_TTL);

         // finally, our IP address as additional record
         this->_writeMyIPAnswerRecord(&ptr, buf, sizeof(DNSHeader_t));

         break;
      }

      case MDNSPacketTypeServiceRecordRelease: {
         // just send our service PTR with a TTL of zero
         this->_writeServiceRecordPTR(&ptr, buf, sizeof(DNSHeader_t), 0);
         break;
      }

#endif // defined(HAS_SERVICE_REGISTRATION) && HAS_SERVICE_REGISTRATION

#if defined(HAS_NAME_BROWSING) && HAS_NAME_BROWSING

      case MDNSPacketTypeNameQuery:
      case MDNSPacketTypeServiceQuery:
      {
         // construct a query for the currently set _resolveNames[0]
         this->_writeDNSName(
               (type == MDNSPacketTypeServiceQuery) ? this->_resolveNames[1] :
                                                      this->_resolveNames[0],
               &ptr, buf, sizeof(DNSHeader_t), 1);

         buf[0] = buf[2] = 0x0;
         buf[1] = (type == MDNSPacketTypeServiceQuery) ? 0x0c : 0x01;
         buf[3] = 0x1;

         this->write((uint8_t*)buf, sizeof(DNSHeader_t));
         ptr += sizeof(DNSHeader_t);

         this->_resolveLastSendMillis[(type == MDNSPacketTypeServiceQuery) ? 1 : 0] = millis();

         break;
      }

#endif // defined(HAS_NAME_BROWSING) && HAS_NAME_BROWSING

      case MDNSPacketTypeNoIPv6AddrAvailable: {
         // since the WIZnet doesn't have IPv6, we will respond with a Not Found message
         this->_writeDNSName(this->_bonjourName, &ptr, buf, sizeof(DNSHeader_t), 1);

         buf[0] = buf[2] = 0x0;
         buf[1] = 0x1c; // AAAA record
         buf[3] = 0x01;

         this->write((uint8_t*)buf, 4);
         ptr += 4;

         // send our IPv4 address record as additional record, in case the peer wants it.
         this->_writeMyIPAnswerRecord(&ptr, buf, sizeof(DNSHeader_t));

         break;
      }
   }


   this->endPacket();

   hkLog.info("Bonjour advertised");
   return statusCode;
}

bool HKBonjour::run()
{
   bool result = false;
   unsigned long now = millis();
   // now, should we re-announce our services again?
   if ((now - this->_lastAnnounceMillis) > 1000*MSNS_ANNOUNCE_TIME_SEC) {
      (void)this->_sendMDNSMessage(0, 0, (int)MDNSPacketTypeServiceRecord);

      this->_lastAnnounceMillis = now;
      result = true;
   }
   return result;
}

void HKBonjour::setBonjourName(const char* bonjourName)
{
   memset(this->_bonjourName, 0, BONJOUR_NAME_MAX_LENGTH);
   strcpy((char*)this->_bonjourName, bonjourName);
   strcpy((char*)this->_bonjourName+strlen(bonjourName), MDNS_TLD);
}

void HKBonjour::setServiceRecord(const char* name, uint16_t port,
                                           MDNSServiceProtocol_t proto, const char* textContent)
{

   memset(this->_serviceRecord.name, 0, MNDS_NAME_MAX_LENGTH);
   strcpy((char*)this->_serviceRecord.name, name);
   if (NULL != textContent) {
       memset(_serviceRecord.textContent, 0, MNDS_TEXT_CONTENT_MAX_LENGTH);
       strcpy((char*)_serviceRecord.textContent, textContent);
   }

   _serviceRecord.port = port;
   _serviceRecord.proto = proto;

   uint8_t* s = this->_findFirstDotFromRight(_serviceRecord.name);
   memset(_serviceRecord.servName, 0,MNDS_SRV_NAME_MAX_LENGTH);
   strcpy((char*)_serviceRecord.servName, (const char*)s);

   const uint8_t* srv_type = this->_postfixForProtocol(proto);
   if (srv_type)
       strcat((char*)_serviceRecord.servName, (const char*)srv_type);

   this->_sendMDNSMessage(0, 0, (int)MDNSPacketTypeServiceRecord);

   hasServiceRecord = true;
}


void HKBonjour::removeServiceRecord()
{
    hasServiceRecord = false;
}

void HKBonjour::_writeDNSName(const uint8_t* name, uint16_t* pPtr,
                                         uint8_t* buf, int bufSize, int zeroTerminate)
{
   uint16_t ptr = *pPtr;
   uint8_t* p1 = (uint8_t*)name, *p2, *p3;
   int i, c, len;

   while(*p1) {
      c = 1;
      p2 = p1;
      while (0 != *p2 && '.' != *p2) { p2++; c++; };

      p3 = buf;
      i = c;
      len = bufSize-1;
      *p3++ = (uint8_t)--i;
      while (i-- > 0) {
         *p3++ = *p1++;

         if (--len <= 0) {
            this->write((uint8_t*)buf, bufSize);
            ptr += bufSize;
            len = bufSize;
            p3 = buf;
         }
      }

      while ('.' == *p1)
         ++p1;

      if (len != bufSize) {
    	  this->write((uint8_t*)buf, bufSize-len);
         ptr += bufSize-len;
      }
   }

   if (zeroTerminate) {
      buf[0] = 0;
      this->write((uint8_t*)buf, 1);
      ptr += 1;
   }

   *pPtr = ptr;
}

void HKBonjour::_writeMyIPAnswerRecord(uint16_t* pPtr, uint8_t* buf, int bufSize)
{
   uint16_t ptr = *pPtr;

   this->_writeDNSName(this->_bonjourName, &ptr, buf, bufSize, 1);

   buf[0] = 0x00;
   buf[1] = 0x01;
   buf[2] = 0x80; // cache flush: true
   buf[3] = 0x01;
   this->write((uint8_t*)buf, 4);
   ptr += 4;

   *((uint32_t*)buf) = ethutil_htonl(MDNS_PTR_TTL);
   *((uint16_t*)&buf[4]) = ethutil_htons(4);      // data length

   uint8_t myIp[4];
   IPAddress myIpBuf;
#if !defined(PARTICLE)
   myIpBuf = Ethernet.localIP();
#else
   myIpBuf = WiFi.localIP();
#endif
   myIp[0] = myIpBuf [0];
   myIp[1] = myIpBuf [1];
   myIp[2] = myIpBuf [2];
   myIp[3] = myIpBuf [3];

   memcpy(&buf[6], &myIp, 4);              // our IP address

   this->write((uint8_t*)buf, 10);
   ptr += 10;

   *pPtr = ptr;
}

void HKBonjour::_writeServiceRecordName(uint16_t* pPtr, uint8_t* buf,
                                                   int bufSize, int tld)
{
   uint16_t ptr = *pPtr;

   uint8_t* name = tld ? this->_serviceRecord.servName :
                         this->_serviceRecord.name;

   this->_writeDNSName(name, &ptr, buf, bufSize, tld);

   if (0 == tld) {
      const uint8_t* srv_type =
         this->_postfixForProtocol(_serviceRecord.proto);

      if (NULL != srv_type) {
         srv_type++; // eat the dot at the beginning
         this->_writeDNSName(srv_type, &ptr, buf, bufSize, 1);
      }
   }

   *pPtr = ptr;
}

void HKBonjour::_writeServiceRecordPTR(uint16_t* pPtr, uint8_t* buf,
                                                  int bufSize, uint32_t ttl)
{
   uint16_t ptr = *pPtr;

   this->_writeServiceRecordName(&ptr, buf, bufSize, 1);

   buf[0] = 0x00;
   buf[1] = 0x0c;    // PTR record
   buf[2] = 0x00;    // no cache flush
   buf[3] = 0x01;    // class IN

   // ttl
   *((uint32_t*)&buf[4]) = ethutil_htonl(ttl);

   // data length (+13 = "._tcp.local" or "._udp.local" + 1  byte zero termination)
   *((uint16_t*)&buf[8]) =
         ethutil_htons(strlen((char*)this->_serviceRecord.name) + 13);

   this->write((uint8_t*)buf, 10);
   ptr += 10;

   this->_writeServiceRecordName(&ptr, buf, bufSize, 0);

   *pPtr = ptr;
}

uint8_t* HKBonjour::_findFirstDotFromRight(const uint8_t* str)
{
   const uint8_t* p = str + strlen((char*)str);
   while (p > str && '.' != *p--);
   return (uint8_t*)&p[2];
}

int HKBonjour::_matchStringPart(const uint8_t** pCmpStr, int* pCmpLen, const uint8_t* buf,
                                           int dataLen)
{
   int matches = 1;

   if (*pCmpLen >= dataLen)
      matches &= (0 == memcmp(*pCmpStr, buf, dataLen));
   else
      matches = 0;

   *pCmpStr += dataLen;
   *pCmpLen -= dataLen;
   if ('.' == **pCmpStr)
      (*pCmpStr)++, (*pCmpLen)--;

   return matches;
}

const uint8_t* HKBonjour::_postfixForProtocol(MDNSServiceProtocol_t proto)
{
   const uint8_t* srv_type = NULL;
   switch(proto) {
      case MDNSServiceTCP:
         srv_type = (uint8_t*)"._tcp" MDNS_TLD;
         break;
      case MDNSServiceUDP:
         srv_type = (uint8_t*)"._udp" MDNS_TLD;
         break;
   }

   return srv_type;
}


uint16_t ethutil_swaps(uint16_t i);
uint32_t ethutil_swapl(uint32_t l);

extern uint16_t ethutil_htons(unsigned short hostshort)
{
#if ( SYSTEM_ENDIAN == _ENDIAN_LITTLE_ )
	return ethutil_swaps(hostshort);
#else
	return hostshort;
#endif
}

extern uint32_t ethutil_htonl(unsigned long hostlong)
{
#if ( SYSTEM_ENDIAN == _ENDIAN_LITTLE_ )
	return ethutil_swapl(hostlong);
#else
	return hostlong;
#endif
}

extern uint16_t ethutil_ntohs(unsigned short netshort)
{
#if ( SYSTEM_ENDIAN == _ENDIAN_LITTLE_ )
	return ethutil_swaps(netshort);
#else
	return netshort;
#endif
}

extern uint32_t ethutil_ntohl(unsigned long netlong)
{
#if ( SYSTEM_ENDIAN == _ENDIAN_LITTLE_ )
	return ethutil_swapl(netlong);
#else
	return netlong;
#endif
}

uint16_t ethutil_swaps(uint16_t i)
{
	uint16_t ret=0;
	ret = (i & 0xFF) << 8;
	ret |= ((i >> 8)& 0xFF);
	return ret;
}

uint32_t ethutil_swapl(uint32_t l)
{
	uint32_t ret=0;
	ret = (l & 0xFF) << 24;
	ret |= ((l >> 8) & 0xFF) << 16;
	ret |= ((l >> 16) & 0xFF) << 8;
	ret |= ((l >> 24) & 0xFF);
	return ret;
}
