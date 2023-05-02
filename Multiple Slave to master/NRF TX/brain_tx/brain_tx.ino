#include <SPI.h>
#include <Wire.h>
#include <nRF24L01.h>
#include <RF24.h>
 
RF24 radio(4, 5); 
const uint64_t address = 0xF0F0F0F0E1LL;
int counter = 0;
 
float temperature;
float humidity;
float altitude;
float pressure;
 

struct MyData 
{
  int counter;
  float temperature;
  float humidity;
  float altitude;
  float pressure;
};
MyData data;
 
void setup() 
{
Serial.begin(115200);
radio.begin();                  //Starting the Wireless communication
radio.openWritingPipe(address); //Setting the address where we will send the data
radio.setPALevel(RF24_PA_MIN);  //You can set it as minimum or maximum depending on the distance between the transmitter and receiver.
radio.stopListening();          //This sets the module as transmitter
 
}
void loop()
{
  data.counter = counter;
  data.temperature = 1;
  data.pressure = 2;
  data.altitude = 3;
  data.humidity = 4;
  
  Serial.print("Packet No. = ");
  Serial.println(data.counter);
  
  Serial.print("Temperature = ");
  Serial.print(data.temperature);
  Serial.println("*C");
 
  Serial.print("Pressure = ");
  Serial.print(data.pressure);
  Serial.println("hPa");
 
  Serial.print("Approx. Altitude = ");
  Serial.print(data.altitude);
  Serial.println("m");
 
  Serial.print("Humidity = ");
  Serial.print(data.humidity);
  Serial.println("%");
 
  Serial.println();
  
  radio.write(&data, sizeof(MyData));
  
  Serial.println("Data Packet Sent");
  Serial.println("");
  
  counter++;
  delay(5000);
}