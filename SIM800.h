/*
   SIM800 - library for Simcom SIM800(L) module.
   Project page: https://github.com/root4root/sim800
*/

/*
MIT License

Copyright (c) 2020 root4root

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <arduino.h>

#ifndef SIMSTRBUFF
#define SIMSTRBUFF 64
#endif

#ifndef H_SIMLIB800
#define H_SIMLIB800

template <typename T>
class SIM800
{
    private:
        char buff[SIMSTRBUFF] = {0};
        
        void (*call)(char *) = 0;
        void (*sms)(char *, char *) = 0;
        void (*dtmf)(char, int) = 0;
        T *serial = 0;
        
        SIM800 ();
        
        void incomingCall();
        void incomingSMS();
        void incomingDTMF();

        void readLine();
        void recognizeEvent();
        

    
    public:
        SIM800 (T *);
        void setIncomingCallHandler(void (*ptr)(char *));
        void setIncomingSMSHandler(void (*ptr)(char *, char *));
        void setIncomingDTMFHandler(void (*ptr)(char, int));
        
        char * handle();
        char * waitResponse();
        void sendSMS(const char *, const char *);
        
};

template <typename T>
SIM800<T>::SIM800(T *ser)
{
    serial = ser;
}

//Bunch of callback setters
template <typename T>
void SIM800<T>::setIncomingCallHandler(void (*ptr)(char*))
{
    call = ptr;
}

template <typename T>
void SIM800<T>::setIncomingSMSHandler(void (*ptr)(char*, char*))
{
    sms = ptr;
}

template <typename T>
void SIM800<T>::setIncomingDTMFHandler(void (*ptr)(char, int))
{
    dtmf = ptr;
} //setters


//Heartbeat
template <typename T>
char * SIM800<T>::handle()
{
    if (serial->available()) {
        this->readLine();
        this->recognizeEvent();
        return buff;
    }
    return NULL;
}

template <typename T>
void SIM800<T>::recognizeEvent()
{
    if (*buff == '+') {
        if (strcasestr(buff, "+DTMF:")) { this->incomingDTMF(); }
        if (strcasestr(buff, "+CMTI:")) { this->incomingSMS(); }
        if (strcasestr(buff, "+CLIP:")) { this->incomingCall(); }
    }
}

template <typename T>
void SIM800<T>::readLine()
{
    char symbol = '0';
    short int i = 0;

    symbol = serial->read();

    if (symbol == '\n') {
        buff[0] = '\0';
        return;
    }

    while (symbol != '\n') {

        if (symbol != -1) {
            buff[i] = symbol;
            ++i;
        } else {
            --i;
        }

        if (i < 1 || (i == sizeof(buff) - 1)) {
            strcpy(buff, "ERR");
            return;
        }

        delay(2);

        symbol = serial->read();
    }
    buff[i] = '\0';
}

template <typename T>
char * SIM800<T>::waitResponse()
{
    char i = 0;

    while (!serial->available() && i < 125) {
       ++i;
       delay(40);

    }

    this->readLine();

    return buff;
}

template <typename T>
void SIM800<T>::sendSMS(const char *number, const char *text)
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


template <typename T>
void SIM800<T>::incomingCall()
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

template <typename T>
void SIM800<T>::incomingSMS()
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

template <typename T>
void SIM800<T>::incomingDTMF()
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

#endif
