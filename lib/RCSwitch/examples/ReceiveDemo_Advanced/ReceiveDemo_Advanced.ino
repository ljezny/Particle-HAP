/*
  Example for receiving

  https://github.com/suda/RCSwitch

  If you want to visualize a telegram copy the raw data and
  paste it into http://test.sui.li/oszi/

  Connect receiver to D3 pin and open serial connection @ 9600 bauds or listen
  Spark Cloud for "tristate-received" event.
*/

#include "RCSwitch.h"

RCSwitch mySwitch = RCSwitch();
int ledPin = D7;
int inputPin = D3;

static char *bin2tristate(char *bin) {
  char returnValue[50];
  for (int i=0; i<50; i++) {
    returnValue[i] = '\0';
  }
  int pos = 0;
  int pos2 = 0;
  while (bin[pos]!='\0' && bin[pos+1]!='\0') {
    if (bin[pos]=='0' && bin[pos+1]=='0') {
      returnValue[pos2] = '0';
    } else if (bin[pos]=='1' && bin[pos+1]=='1') {
      returnValue[pos2] = '1';
    } else if (bin[pos]=='0' && bin[pos+1]=='1') {
      returnValue[pos2] = 'F';
    } else {
      return "not applicable";
    }
    pos = pos+2;
    pos2++;
  }
  returnValue[pos2] = '\0';
  return returnValue;
}

void output(unsigned long decimal, unsigned int length, unsigned int delay, unsigned int* raw, unsigned int protocol) {

  if (decimal == 0) {
    Serial.print("Unknown encoding.");
  } else {
    char* b = mySwitch.dec2binWzerofill(decimal, length);
    char* tristate = bin2tristate(b);

    Serial.print("Decimal: ");
    Serial.print(decimal);
    Serial.print(" (");
    Serial.print( length );
    Serial.print("Bit) Binary: ");
    Serial.print( b );
    Serial.print(" Tri-State: ");
    Serial.print( tristate );
    Serial.print(" PulseLength: ");
    Serial.print(delay);
    Serial.print(" microseconds");
    Serial.print(" Protocol: ");
    Serial.println(protocol);

    Spark.publish("tristate-received", String(delay) + " " + String(tristate));
  }

  Serial.print("Raw data: ");
  for (int i=0; i<= length*2; i++) {
    Serial.print(raw[i]);
    Serial.print(",");
  }
  Serial.println();
  Serial.println();
}

void setup() {
  Serial.begin(9600);
  Serial.println("Listening");

  pinMode(inputPin, INPUT_PULLDOWN);
  mySwitch.enableReceive(inputPin);

  pinMode(ledPin, OUTPUT);
}

void loop() {
  int inputPinState = digitalRead(inputPin);
  digitalWrite(ledPin, inputPinState);

  if (mySwitch.available()) {
    output(mySwitch.getReceivedValue(), mySwitch.getReceivedBitlength(), mySwitch.getReceivedDelay(), mySwitch.getReceivedRawdata(), mySwitch.getReceivedProtocol());
    mySwitch.resetAvailable();
  }
}
