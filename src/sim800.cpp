#include "sim800.h"
#include <arduino.h>

SIM800::SIM800(SoftwareSerial *ser)
{
    serial = ser;
}

//Bunch of callback setters
void SIM800::setIncomingCallHandler(void (*ptr)(char*))
{
    call = ptr;
}

void SIM800::setIncomingSMSHandler(void (*ptr)(char*, char*))
{
    sms = ptr;
}

void SIM800::setIncomingDTMFHandler(void (*ptr)(char, int))
{
    dtmf = ptr;
} //setters


//Heartbeat
char * SIM800::handle()
{
    if (serial->available()) {
        this->readLine();
        this->recognizeEvent();
        return buff;
    }
    return NULL;
}

void SIM800::recognizeEvent()
{
    if (*buff == '+') {
        if (strcasestr(buff, "+DTMF:")) { this->incomingDTMF(); }
        if (strcasestr(buff, "+CMTI:")) { this->incomingSMS(); }
        if (strcasestr(buff, "+CLIP:")) { this->incomingCall(); }
    }
}

void SIM800::readLine()
{
    char symbol = '0';
    short int i = 0;

    symbol = serial->read();

    while (symbol != '\n') {
    
        if (symbol != -1) {
            buff[i] = symbol;
            ++i;
        } else {
            --i;
        }
      
        if (i == 0 || (i > sizeof(buff) - 1)) { break; }
    
        delay(2);
    
        symbol = serial->read();
    }
    buff[i] = '\0';
}

char * SIM800::waitResponse()
{
    char i = 0;
    
    while (!serial->available() && i < 250) {
       ++i;
       delay(20);

    }
    
    this->readLine();
    
    return buff;
}

void SIM800::sendSMS(char *number, char *text)
{
    serial->println(F("AT+CMGF=1"));

    this->waitResponse(); 

    serial->print(F("AT+CMGS=\""));
    serial->print(number);           
    serial->print(F("\"\n"));       

    this->waitResponse(); 

    serial->print(text);
    serial->print ("\n");
    serial->print((char)26);

    this->waitResponse(); 
}



void SIM800::incomingCall()
{
    if (!call) return;
    
    char *start = 0, *end = 0; //start/end substring ptr
    int lenght = 0; //substring lenght holder
    
    start = strstr(buff, "\"");
    ++start;
    
    end = start;
    
    while (*end != '"') {
        ++end;
    }
    
    lenght = end - start;
    
    if (lenght == 0) {
        char phoneNumber[] = "0";
        call(phoneNumber);
        return;
    }
    
    char phoneNumber[lenght+1] = {0};
    memcpy(phoneNumber, start, lenght);
    call(phoneNumber);
}

void SIM800::incomingSMS()
{
    if (!sms) return;
    

    char *start = 0, *end = 0;
    int lenght;
    
    //Parsing notification message to get SMS ID
    start = strstr(buff, "\",");
    start += 2;
       
    while (*start < 48 || *start > 57 ) {
       ++start;
    }
    
    end = start;
    
    while (*end > 47 && *end < 58) {
       ++end;
    }
    
    lenght = end - start;

    serial->print(F("AT+CMGR="));
    
    char smsid[lenght+1] = {0};
    memcpy (smsid, start, lenght);

    serial->print(smsid);
    serial->print("\r\n");

    this->waitResponse();

    //Now we have message header in our buffer. Parsing...
    start = strstr(buff, "\",");
    start += 3;
    end = start;
    
    while (*end != '"') {
        ++end;
    }
    
    lenght = end - start;
    
    char phoneNumber[lenght+1] = {0};
    
    memcpy(phoneNumber, start, lenght);
    
    //Reading SMS text to buffer
    this->readLine();
    
    sms(phoneNumber, buff);
    
    this->readLine();
}

void SIM800::incomingDTMF()
{
   if (!dtmf) return; //is callback function set
   
   char *buffptr = buff;
   
   unsigned int duration = 0;
   short i = 1;
   
   while (*buffptr != '\r') {
      ++buffptr;
   }
   
   --buffptr;
   
   while (*buffptr != ',') {
       if (*buffptr == ' ') { break; } //Missconfiguration fail protection
       duration += (*buffptr - '0') * i;
       --buffptr;
       i *= 10;
   }

   dtmf(buff[7], duration);
}
