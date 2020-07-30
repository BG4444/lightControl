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
  Serial.begin(115200); 

  pinMode(3, OUTPUT);


  Serial.println();
  Serial.print("connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.hostname("ESP-light");
  WiFi.begin(ssid, password);
  size_t nError=0;
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(1000);
    Serial.print(".");
    if(++nError==20)
    {
       ESP.restart();
    }
  }
  
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address:");
  Serial.println(WiFi.localIP());

  // Use WiFiClientSecure class to create TLS connection
 
  Serial.print("connecting to ");
  Serial.println(host);
  client.setFingerprint(fingerprint);
  
}

void loop() 
{
  static size_t nError=0;
  for(;;)
  {
    Serial.println("connecting...");
    if (!client.connect(host, httpsPort)) 
    {
      Serial.println("connection failed");
      if(++nError==20)
      {
         ESP.restart();
      } 
      break;
    }

    nError=0;    
    
    Serial.print("requesting URL  ");
    Serial.println(url);
    const auto hex= getHex(uptime);
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "User-Agent: ESP8266\r\n" +
                 "Accept: *//*\r\n" +
                 "Content-Length:144\r\n" +
                 "Connection: close\r\n\r\n" + 
                 token + hex +
                    "\r\n\r\n");
    Serial.println(hex);
    Serial.println(uptime);
    Serial.println("request sent");
    while (client.connected()) 
    {
      String line = client.readStringUntil('\n');
      if (line == "\r") 
      {
        Serial.println("headers received");
        break;
      }
    }
    String line = client.readStringUntil('\n');
    Serial.println("reply was ");
    Serial.println("==========");
    Serial.println(line);
    Serial.println("==========");
    Serial.println("closing connection");    
    const auto v=line.toInt();

    digitalWrite(3, v & 0b01);

    break;    
  }  
  delay(2000);
  ++uptime;
}
