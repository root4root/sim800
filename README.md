## Arduino SIM800 unsolicited notifications handler

Arduino library for sim800 GSM module with event handling (unsolicited notifications) via callbacks.


### Example:


```cpp
#include <sim800.h>
#include <arduino.h>

SoftwareSerial SSERIAL(4, 3);

SIM800 SIM(&SSERIAL);

int status = 0;

void sim800IncomingCall(char *number)
{
   if (status == 0) {
      digitalWrite(A1, HIGH);
      digitalWrite(A2, HIGH);
      status = 1;
   } else {
      digitalWrite(A1, LOW);
      digitalWrite(A2, LOW);
      status = 0;
    }
   //SSERIAL.println(F("ATA")); //Off-hook (answer)
   //SSERIAL.println(F("AT+VTS=\"5,5,5\"")); //Play DTMF to the line
}

void sim800IncomingSMS(char *number, char *text)
{
    char textCopy[strlen(text)];

    //Let's send message back for example (parrot mode)
    strcpy(textCopy, text);
    SIM.sendSMS(number, textCopy);
    
    SSERIAL.println(F("AT+CMGDA=\"DEL ALL\"")); //Delete all SMS to prevent overflow.
}

void sim800IncomingDTMF(char key, int duration)
{
    if (key == '*') {
        SSERIAL.println(F("ATH")); //Hang Up call
    }
    //... some logic here ...
}

void setup() {
   delay(5000);            //Waiting for SIM800 boot up...
   SSERIAL.begin(9600);    //If module not ready yet, connection won't initialize properly
   SSERIAL.println(F("AT"));
   SIM.waitResponse();
   SSERIAL.println(F("AT+DDET=1,0,1")); //Enable DTMF key duration info
   
   //SSERIAL.println(F("AT+CLIP=1")); //Enable the calling line identity (CLI)
   //SSERIAL.println(F("AT+CMEE=1"));
   //SSERIAL.println(F("AT+CMGF=1"));
   //SSERIAL.println(F("ATV0"));
   //SSERIAL.println(F("ATE0"));
   
   
   //Serial.begin(9600);

   pinMode(A1, OUTPUT);
   pinMode(A2, OUTPUT);

   SIM.setIncomingCallHandler(sim800IncomingCall);
   SIM.setIncomingSMSHandler(sim800IncomingSMS);
   SIM.setIncomingDTMFHandler(sim800IncomingDTMF);
}


void loop() {
    SIM.handle(); //Checking for unsolicited notification
    //be careful, SIM800 library will read serial buffer
    //make sure you got all data that you might expect from SIM800, *before* handle() call.

}

```

### License:
MIT
### Author:
Paul aka root4root \<root4root at gmail dot com><br/>
**Any comments/suggestions are welcomed.**
