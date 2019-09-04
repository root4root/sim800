#include <arduino.h>
#include "SoftwareSerial.h"

#ifndef SIMSTRBUFF
#define SIMSTRBUFF 64
#endif

#ifndef H_SIMLIB800
#define H_SIMLIB800

class SIM800
{
    private:
        char buff[SIMSTRBUFF] = {0};
        
        void (*call)(char *) = 0;
        void (*sms)(char *, char *) = 0;
        void (*dtmf)(char, int) = 0;
        SoftwareSerial *serial = 0;
        
        SIM800 ();
        
        void incomingCall();
        void incomingSMS();
        void incomingDTMF();

        void readLine();
        void recognizeEvent();
        

    
    public:
        SIM800 (SoftwareSerial *);
        void setIncomingCallHandler(void (*ptr)(char *));
        void setIncomingSMSHandler(void (*ptr)(char *, char *));
        void setIncomingDTMFHandler(void (*ptr)(char, int));
        
        char * handle();
        char * waitResponse();
        void sendSMS(char *, char *);
        
};

#endif
