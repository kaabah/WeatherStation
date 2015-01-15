/*Weather Station
Kaabah Di Lacerda
12/01/2015

Sensors 
DHT22
BMP180
*/
#include "DHT.h"
#include <SFE_BMP180.h>
#include <Wire.h>

//Setting DHT variables
#define DHTPIN 2    
//#define DHTTYPE DHT11   // DHT 11 
#define DHTTYPE DHT22   // DHT 22  (AM2302)

DHT dht(DHTPIN, DHTTYPE);

//Setting BMP180 variables
SFE_BMP180 bmp;

#define ALTITUDE 853.0 // Altitude of Belo Horizonte/MG - Brazil

void setup()
{
  setupBmp180();
  setupDht22();
 
  
  
}

void loop ()
{
  double tempBmp180, relPressureBmp180, absPressureBmp180, altBmp180;
  float tempDht22, humidity, heatIndexDhtBased, heatIndexBmpBased;
  
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
      Serial.print(" deg C, ");
    }
  }
  return temp;
}

double getBmp180AbsolutePressure (double temp) 
{
  char status;
  double pressure;
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

float getDhtHeatIndex (float temp, float percentHumidity) 
{
  //convert temp to Fahrenheit
  float tempF = dht.convertCtoF(temp);
  
  float heatIndex = dht.convertFtoC(dht.computeHeatIndex(tempF, percentHumidity));
  
  Serial.print("Heat index: ");
  Serial.print(heatIndex);
  Serial.println(" *C");
  
  return heatIndex;
}

void getDateTime() 
{
  
}

//Send to Receiver Method 

void sendInfo() 
{
}

