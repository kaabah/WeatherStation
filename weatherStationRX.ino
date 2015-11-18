#include "SPI.h"
#include "Ethernet.h"
#include "VirtualWire.h"
 
byte message[VW_MAX_MESSAGE_LEN];   
byte msgLength = VW_MAX_MESSAGE_LEN; 
 
char StringReceived[100]; 

// ThingSpeak Settings
char thingSpeakAddress[] = "api.thingspeak.com";
String writeAPIKey = "EI7V5IMM5MUGBPLO";

const int updateThingSpeakInterval = 16 * 1000;      // Time interval in milliseconds to update ThingSpeak (number of seconds * 1000 = interval)

// Variable Setup
long lastConnectionTime = 0; 
boolean lastConnected = false;
int failedCounter = 0;

byte mac[] = { 
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

EthernetClient client;

void setup()   {
  Serial.begin(9600);
  vw_set_rx_pin(2); //Receptor PIN 
  vw_setup(100); //bits per second
  vw_rx_start(); 
  delay(2000);
  // Start Ethernet on Arduino
  startEthernet();
 
}
 
void loop()
{
    uint8_t message[VW_MAX_MESSAGE_LEN];    
    uint8_t msgLength = VW_MAX_MESSAGE_LEN; 
        
    if (vw_get_message(message, &msgLength)) // Non-blocking
    { 
       for (int i = 0; i < msgLength; i++)
       {
        StringReceived[i] = char(message[i]);
       }
       StringReceived[msgLength] = '\0';
       
       String thingSpeakStr(StringReceived);
       
       Serial.print("Recebido: ");
       Serial.println(thingSpeakStr);
       
        // Print Update Response to Serial Monitor
       while (client.available())
        {
          char c = client.read();
          //Serial.print("TESTE-----");
          Serial.print(c);
        }
      
        // Disconnect from ThingSpeak
       if (!client.connected() && lastConnected)
        {
          Serial.println("...disconnected");
          Serial.println();
          
          client.stop();
        }
        
        // Update ThingSpeak
       if(!client.connected() && (millis() - lastConnectionTime > updateThingSpeakInterval))
        {
          //updateThingSpeak("field1="+analogValue0);
          updateThingSpeak(thingSpeakStr);
        }
        
        // Check if Arduino Ethernet needs to be restarted
        if (failedCounter > 3 )
        {
        startEthernet();
        }
        
        lastConnected = client.connected();
           
        }
    
   
 }

void updateThingSpeak(String tsData)
{  
  if (client.connect(thingSpeakAddress, 80))
  {         
    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: "+writeAPIKey+"\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(tsData.length());
    client.print("\n\n");

    client.print(tsData);
    
    Serial.println("Enviado");
    Serial.println(tsData);
    
    lastConnectionTime = millis();
    
    if (client.connected())
    {
      Serial.println("Connecting to ThingSpeak...");
      Serial.println();
      
      Serial.println("Enviado2");
    Serial.println(tsData);
      
      failedCounter = 0;
    }
    else
    {
      failedCounter++;
  
      Serial.println("Connection to ThingSpeak failed ("+String(failedCounter, DEC)+")");   
      Serial.println();
    }
    
  }
  else
  {
    failedCounter++;
    
    Serial.println("Connection to ThingSpeak Failed ("+String(failedCounter, DEC)+")");   
    Serial.println();
    
    lastConnectionTime = millis(); 
  }
  //client.stop();
  tsData = "";
}

void startEthernet()
{
  
  client.stop();

  Serial.println("Connecting Arduino to network...");
  Serial.println();  

  delay(1000);
  
  // Connect to network amd obtain an IP address using DHCP
  if (Ethernet.begin(mac) == 0)
  {
    Serial.println("DHCP Failed, reset Arduino to try again");
    Serial.println();
  }
  else
  {
    Serial.println("Arduino connected to network using DHCP");
    Serial.println();
    Serial.print("IP = ");
    Serial.println(Ethernet.localIP());
    Serial.print("dns = ");
Serial.println(Ethernet.dnsServerIP());
  }
  
  
  
  delay(1000);
}
