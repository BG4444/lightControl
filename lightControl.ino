#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>

#include "cred.h"

const char* ssid = STASSID;
const char* password = STAPSK;

const char subst[2] = {'0','1'};

static uint32_t uptime = 0;

WiFiClientSecure client;

String getHex(uint32_t in)
{
    String ret(in);
    while(ret.length()<16)
    {
        ret=' '+ret;
    }
    return ret;
}

void setup() 
{  
  
  for(uint8_t i=1;i<=3;++i)
  {    
    pinMode(i, OUTPUT);
  }

  bool onOff = true;
    

  WiFi.mode(WIFI_STA);
  WiFi.hostname("ESP-light");
  WiFi.begin(ssid, password);
  size_t nError=0;
  while (WiFi.status() != WL_CONNECTED) 
  {
    digitalWrite(2, onOff); 
    delay( onOff ? 1000 : 500);
    Serial.print(".");
    if(++nError==20)
    {
       ESP.restart();
    }
    onOff = !onOff;
  }
  
  
  // Use WiFiClientSecure class to create TLS connection
 
  client.setFingerprint(fingerprint);
  
}

void loop() 
{
  static uint8_t nError=0;  
  static uint8_t pinStatus = 0;  
  static bool onOff=false;  
  static uint8_t startupCntr[2] = {0};
  constexpr uint8_t nStartSteps = 20;
  constexpr uint8_t pinIndex[2] = {1, 3};
  for(;;)
  {
    if(uptime % 8 == 0)
    {    
      if (!client.connect(host, httpsPort)) 
      {
        if(++nError==20)
        {
           ESP.restart();
        } 
        break;
      }
  
      nError=0;    
      const auto hex= getHex(uptime);
      client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                   "Host: " + host + "\r\n" +
                   "User-Agent: ESP8266\r\n" +
                   "Accept: *//*\r\n" +
                   "Content-Length:144\r\n" +
                   "Connection: close\r\n\r\n" + 
                   token + hex +
                      "\r\n\r\n");
      while (client.connected()) 
      {
        String line = client.readStringUntil('\n');
        if (line == "\r") 
        {
          break;
        }
      }
      String line = client.readStringUntil('\n');
      const auto v=line.toInt();
  
      const uint8_t newPins = (v & 0b11);
  
      const uint8_t pinOn = (~pinStatus) &   newPins;
      const uint8_t pinOff =  pinStatus  & (~newPins);
  
      for(uint8_t i=1;i<=2;++i)
      {
         if(pinOn & i)
         {
            startupCntr[i-1] = nStartSteps;      
         }
         if(pinOff & i)
         {
            startupCntr[i-1] = 0;      
            digitalWrite(pinIndex[i-1], LOW);
         }
      }
      
      pinStatus = newPins;
             
      digitalWrite(2, onOff);
  
      onOff=!onOff;
    }
    break;    
  }  
  for(uint8_t i=0;i<2;++i)
  {
    if(startupCntr[i])
    { 
      --startupCntr[i];     
      analogWrite(pinIndex[i],50*(nStartSteps - startupCntr[i]) );
      if(!startupCntr[i])
      {
        digitalWrite(pinIndex[i-1], HIGH);
      }
    }
  }
  
  
  delay(200);
  ++uptime;
}
