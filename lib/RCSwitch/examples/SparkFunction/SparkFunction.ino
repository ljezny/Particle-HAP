/*
  Example for sending Tri-state code via Spark Cloud.

  Call via CLI: spark call <CORE_NAME> sendtristate "PULSE_LENGTH TRISTATE"

  https://github.com/suda/RCSwitch
*/

#include "RCSwitch.h"

RCSwitch mySwitch = RCSwitch();

int sendTristate(String command) {
  int positionOfSeparator = command.indexOf(' ');
  int pulseLength = command.substring(0, positionOfSeparator).toInt();
  String triState = command.substring(++positionOfSeparator);

  mySwitch.setPulseLength(pulseLength);

  char triStateChars[triState.length() + 1];
  triState.toCharArray(triStateChars, triState.length() + 1);
  triStateChars[triState.length()] = '\0';

  mySwitch.sendTriState(triStateChars);

  return 200;
}

void setup() {
  // Transmitter is connected to Spark Core Pin D0
  mySwitch.enableTransmit(D0);

  // Register function
  Spark.function("sendtristate", sendTristate);
}

void loop() {
}
