#include "SIM800.h"

SIM800<HardwareSerial> SIM(&Serial);

int statusLED = 0;

void sim800IncomingCall(char *number)
{
    Serial.println(F("ATH"));

    if (statusLED == 0) {
        digitalWrite(5, HIGH);
        statusLED = 1;
    } else {
        digitalWrite(5, LOW);
        statusLED = 0;
    }
}

void sim800IncomingSMS(char *number, char *text)
{
    //char textCopy[strlen(text)];

    //Let's send message back for example (parrot mode)
    //strcpy(textCopy, text);
    //SIM.sendSMS(number, textCopy);

    Serial.println(F("AT+CMGDA=\"DEL ALL\"")); //Delete all SMS to prevent overflow.
}

void sim800IncomingDTMF(char key, int duration)
{
    if (key == '*') {
        Serial.println(F("ATH")); //Hang Up call
    }
    //... some logic here ...
}

void setup() {
   delay(5000);              //Waiting for SIM800 boot up...
   Serial.begin(9600);       //If module not ready yet, connection won't initialize properly
   Serial.println(F("AT"));  //Speed syncing...
   SIM.waitResponse();
   Serial.println(F("AT+DDET=1,0,1")); //Enable DTMF key duration info

   //Serial.println(F("AT+CLIP=1")); //Enable the calling line identity (CLI)
   //Serial.println(F("AT+CMEE=1"));
   //Serial.println(F("AT+CMGF=1"));
   //Serial.println(F("ATV0"));
   //Serial.println(F("ATE0"));

   pinMode(5, OUTPUT);

   SIM.setIncomingCallHandler(sim800IncomingCall);
   SIM.setIncomingSMSHandler(sim800IncomingSMS);
   SIM.setIncomingDTMFHandler(sim800IncomingDTMF);
}


void loop() {
    SIM.handle(); //Checking for unsolicited notification
    //be careful, SIM800 library will read serial buffer
    //make sure you got all data that you might expect from SIM800, *before* handle() call.
}
