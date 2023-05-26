#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(4, 5); 
const uint64_t address = 0xF0F0F0F0E1LL;

#define BUTTONPIN 12
 

/************* Structure to send data to Brain *************/
struct StructureOfTeam
{
  String team_name;
  int health;
  unsigned char go = 0;
  unsigned char time = 0;
  unsigned char id = 0;
};

StructureOfTeam TeamData;
 
/************* Structure to receive data from Admin *************/
struct StructureOfBrain
{
  int counter;
  String slave_id;
  int health;
};

StructureOfBrain BrainData;
 
void setup() 
{
  Serial.begin(9600);
  radio.begin();

  pinMode(BUTTONPIN, INPUT);
  
  Serial.println("Transmitter started....");
  radio.openWritingPipe(address); //Setting the address where we will send the data
  radio.setPALevel(RF24_PA_MIN);  //You can set it as minimum or maximum depending on the distance between the transmitter and receiver.
  radio.stopListening();          //This sets the module as transmitter
 
 /*
  Serial.println("Receiver Started....");
 
  radio.openReadingPipe(0, address);   //Setting the address at which we will receive the data
  radio.setPALevel(RF24_PA_MIN);       //You can set this as minimum or maximum depending on the distance between the transmitter and receiver.
  radio.startListening();              //This sets the module as receiver
*/
    Serial.println("*** Sending First data to Brain: 01 ******");
    delay(500);
    Serial.println();
  
  /* Send Team and tank name with space*/
    TeamData.team_name = "DAVID BLUE";
    TeamData.health = 150;
    TeamData.go = 0;
    TeamData.time = 5;

    radio.write(&TeamData, sizeof(StructureOfTeam));
    
    while(1)
    {
      if(digitalRead(BUTTONPIN) == 1)
      {
        
        break;
      }
    } 

    TeamData.go = 1;

    /* Starting the game */
    Serial.println(" >>>> Starting the Game Now: <<<< ");
    radio.write(&TeamData, sizeof(StructureOfTeam));

    
    Serial.println("Data Packet Sent");
    Serial.println("");

    delay(1000);
    
    Serial.println("Receiver Started....");
 
    radio.openReadingPipe(0, address);   //Setting the address at which we will receive the data
    radio.setPALevel(RF24_PA_MIN);       //You can set this as minimum or maximum depending on the distance between the transmitter and receiver.
    radio.startListening();              //This sets the module as receiver

}
 
int recvData()
{
  if ( radio.available() ) 
  {
    radio.read(&BrainData, sizeof(StructureOfBrain));
    return 1;
    }
    return 0;
}
 
 
void loop()
{

  if(recvData())
  {
 
  Serial.print("Packet No. = ");
  Serial.println(BrainData.counter);
  
  Serial.print("Health = ");
  Serial.print(BrainData.health);
 
  Serial.print("SlaveID = ");
  Serial.print(BrainData.slave_id);
 
  Serial.println();

  }

}