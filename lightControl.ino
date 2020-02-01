#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>

#include "cred.h"

const char* ssid = STASSID;
const char* password = STAPSK;

const char subst[2] = {'~','#'};

WiFiClientSecure client;
    
void setup() 
{
  
  Serial.begin(115200);
  pinMode(3, INPUT);
  Serial.println();
  Serial.print("connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.hostname("ESP-water");
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

  Serial.printf("Using fingerprint '%s'\n", fingerprint);
  client.setFingerprint(fingerprint);

  
}

void loop() 
{
  static size_t nError=0;
  for(;;)
  {
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

    const auto v=digitalRead(3);

    const char status = subst[ size_t (v==HIGH) ];

    Serial.println(status);

    
    Serial.print("requesting URL  ");
    Serial.println(url);
  
    client.print(String("PUT ") + url + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "User-Agent: ESP8266\r\n" +
                 "Accept: *//*\r\n" +
                 "Content-Length:129\r\n" +                    
                 "Connection: close\r\n\r\n" + 
                 token +
                 status +
                    "\r\n\r\n");
  
    Serial.println("request sent");
    String line = client.readStringUntil('\n');
    while (client.connected()) 
    {
      
      if (line == "\r") 
      {
        Serial.println("headers received");
        break;
      }
    }
    Serial.println("reply was ");
    Serial.println("==========");
    Serial.println(line);
    Serial.println("==========");
    Serial.println("closing connection");
    break;    
  }
  delay(10000);
}
