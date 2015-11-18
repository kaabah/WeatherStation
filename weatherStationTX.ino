#include "DHT.h"
#include "SFE_BMP180.h"
#include "Wire.h"
#include "RTClib.h"
#include "VirtualWire.h"

//Setting DHT variables
#define DHTPIN 3    
//#define DHTTYPE DHT11   // DHT 11 
#define DHTTYPE DHT22   // DHT 22  (AM2302)

DHT dht(DHTPIN, DHTTYPE);
RTC_DS1307 rtc;

//Setting BMP180 variables
SFE_BMP180 bmp;

int interval = 10; //minutes

 double tempBmp180, relPressureBmp180, absPressureBmp180, altBmp180;
 float tempDht22, humidity, heatIndexDhtBased, heatIndexBmpBased;

DateTime lastRead;

char SensorCharMsg[150];
char DatetimeSensorCharMsg[20];

#define ALTITUDE 853.0 // Altitude of Belo Horizonte/MG - Brazil

void setup()
{
  setupDs1307();
  setupBmp180();
  setupDht22();
  setupRf433();
  lastRead = getDateTime();
}

void loop ()
{  
  //read datetime
  
   DateTime timeNow = getDateTime();
   
   TimeSpan diff = timeNow - lastRead;
   
   if (diff.minutes() >= interval) {
       //read bmp180 values
       tempBmp180 = getBmp180Temp();
       absPressureBmp180 = getBmp180AbsolutePressure(tempBmp180);
       relPressureBmp180 = getBmp180RelativePressure(absPressureBmp180);
       altBmp180 = getBmp180Alt(absPressureBmp180, relPressureBmp180);
   
       //read dht22 values
       tempDht22 = getDhtTemp();
       humidity = getDhtHumidity();
      
       //heat index calc
       heatIndexDhtBased = getDhtHeatIndex(tempDht22, humidity); 
       heatIndexBmpBased = getDhtHeatIndex(tempBmp180, humidity); 
       
       char strTempBmp180[6], strAbsPressureBmp180[7], strRelPressureBmp180[7], strAltBmp180[7];
       char strTempDht22[6], strHumidity[6], strHeatIndexDhtBased[6], strHeatIndexBmpBased[6];
       
       dtostrf(tempBmp180, 4, 2, strTempBmp180);
       dtostrf(absPressureBmp180, 5, 2, strAbsPressureBmp180);
       dtostrf(relPressureBmp180, 5, 2, strRelPressureBmp180);
       dtostrf(altBmp180, 5, 2, strAltBmp180);
       dtostrf(tempDht22, 4, 2, strTempDht22);
       dtostrf(humidity, 4, 2, strHumidity);
       dtostrf(heatIndexDhtBased, 4, 2, strHeatIndexDhtBased);
       dtostrf(heatIndexBmpBased, 4, 2, strHeatIndexBmpBased);
       
       sprintf(DatetimeSensorCharMsg,"%i-%i-%i %i:%i:%i", timeNow.year(), timeNow.month(), timeNow.day(), 
       timeNow.hour(), timeNow.minute(), timeNow.second());
       
       
       //sprintf(SensorCharMsg,"%s;%s;%s;%s;%s;%s;%s;%s;%s", strTempBmp180, strAbsPressureBmp180, strRelPressureBmp180, 
       //strAltBmp180, strTempDht22, strHumidity, strHeatIndexDhtBased, strHeatIndexBmpBased, DatetimeSensorCharMsg);
     
       sprintf(SensorCharMsg,"1=%s&2=%s&3=%s&4=%s&5=%s&6=%s&7=%s", 
       strTempBmp180, strAbsPressureBmp180, strRelPressureBmp180, strTempDht22, strHumidity, 
       strHeatIndexDhtBased, strHeatIndexBmpBased);
     
       //Serial.println(SensorCharMsg);
     
       sendInfo(SensorCharMsg);
       
       lastRead = getDateTime();
       //delay(5000);
   }
}

//Setup methods

void setupBmp180()
{
  Serial.begin(9600);
  
  if (bmp.begin()) 
  {
    Serial.println("BMP180 init success");
  }
  else
  {
    //Something went wrong

    Serial.println("BMP180 init fail\n\n");
    while(1); // Pause forever.
  }
}

void setupDht22()
{
 Serial.begin(9600); 
 Serial.println("DHT22 init!");
 
  dht.begin();
}

void setupDs1307 () {
  Wire.begin();
  rtc.begin();

  Serial.begin(9600);
  
  if (!rtc.isrunning()) {
    Serial.println("RTC stopped, ajusting now...");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
}

void setupRf433()
{
  Serial.begin(9600);
  vw_set_tx_pin(8); //Define o pino 8 do Arduino como 
//o pino de dados do transmissor
  vw_setup(100);   // Bits per sec
}
 

//Get methods


double getBmp180Temp() {
  
  char status;
  double temp = -99;
  
  status = bmp.startTemperature();
  if (status != 0)
  {
    // Wait for the measurement to complete:
    delay(status);

    // Retrieve the completed temperature measurement:
    // Note that the measurement is stored in the variable T.
    // Function returns 1 if successful, 0 if failure.

    status = bmp.getTemperature(temp);
    if (status != 0)
    {
      // Print out the measurement:
      Serial.print("temperature: ");
      Serial.print(temp,2);
      Serial.print(" C, ");
    }
  }
  return temp;
}

double getBmp180AbsolutePressure (double temp) 
{
  char status;
  double pressure;
  
   status = bmp.startPressure(3);
      if (status != 0)
      {
        // Wait for the measurement to complete:
        delay(status);
  
        status = bmp.getPressure(pressure,temp);
        if (status != 0)
        {
          // Print out the measurement:
          Serial.print("absolute pressure: ");
          Serial.print(pressure,2);
          Serial.print(" mb, ");
          Serial.print(pressure*0.0295333727,2);
          Serial.println(" inHg");
        }
      }
      
  return pressure;
}

double getBmp180RelativePressure (double absPressure) 
{
  char status;
  double pressure;
  
   // The pressure sensor returns abolute pressure, which varies with altitude.
  // To remove the effects of altitude, use the sealevel function and your current altitude.
  // This number is commonly used in weather reports.
  // Parameters: P = absolute pressure in mb, ALTITUDE = current altitude in m.
  // Result: p0 = sea-level compensated pressure in mb
  
  pressure = bmp.sealevel(absPressure, ALTITUDE); // we're at 853 meters (Boulder, CO)
  Serial.print("relative (sea-level) pressure: ");
  Serial.print(pressure,2);
  Serial.print(" mb, ");
  Serial.print(pressure*0.0295333727,2);
  Serial.println(" inHg");
  
  return pressure;
}

double getBmp180Alt(double absPressure, double relPressure) {
 
 double alt; 
 
  alt = bmp.altitude(absPressure, relPressure);
  Serial.print("computed altitude: ");
  Serial.print(alt, 0);
  Serial.print(" meters, ");
  
  return alt;
}

float getDhtTemp() {
  float t = dht.readTemperature();
  // check if returns are valid, if they are NaN (not a number) then something went wrong!
  if (isnan(t)) {
    Serial.println("Failed to read from DHT");
  } else {
    
    Serial.print("Temperature: ");
    Serial.print(t);
    Serial.println(" *C");
    return t;
  }
}

float getDhtHumidity() {
 
  float h = dht.readHumidity();
 
  // check if returns are valid, if they are NaN (not a number) then something went wrong!
  if (isnan(h)) {
    Serial.println("Failed to read from DHT");
  } 
  else {
    Serial.print("Humidity: ");
    Serial.print(h);
    Serial.print(" %\t");
    return h;
  }
}

double getDhtHeatIndex (float temp, float percentHumidity) 
{
  //convert temp to Fahrenheit
  float tempF = dht.convertCtoF(temp);
  
  float heatIndex = dht.convertFtoC(dht.computeHeatIndex(tempF, percentHumidity));
  
  Serial.print("Heat index: ");
  Serial.print(heatIndex);
  Serial.println(" *C");
  
  return heatIndex;
}

DateTime getDateTime() 
{
  DateTime timeNow = rtc.now();
 
/*  Serial.print(timeNow.year(), DEC);
  Serial.print('/');
  Serial.print(timeNow.month(), DEC);
  Serial.print('/');
  Serial.print(timeNow.day(), DEC);
  Serial.print(' ');
  Serial.print(timeNow.hour(), DEC);
  Serial.print(':');
  Serial.print(timeNow.minute(), DEC);
  Serial.print(':');
  Serial.print(timeNow.second(), DEC);
  Serial.println();*/
  
  return timeNow;
}

//Send to Receiver Method 

void sendInfo (char *message)
{
  vw_send((uint8_t *)message, strlen(message));
  vw_wait_tx(); // Aguarda o envio de dados
}



