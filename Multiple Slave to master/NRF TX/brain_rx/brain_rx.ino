#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(4, 5); 
const uint64_t address = 0xF0F0F0F0E1LL;

#define BUTTONPIN       (12U)
#define NUM_OF_BRAINS   (2U)
#define VOLT_PIN        (39U)
#define RED_LED         (34U)

double volt_measure()
{
  int volt = analogRead(VOLT_PIN);// read the input
  double voltage = map(volt,0, 3000, 0, 7.4);// map 0-1023 to 0-2500 and add correction offset
  return voltage + 0.5;
}

String TeamTank[NUM_OF_BRAINS] = {"DAVID BLUE", "NIKE GREEN"};
 

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
  int brain_id;
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
    Serial.println("*** Sending Data to brains******");
    delay(200);
    Serial.println();


      /* TODO: Iterate the ID's here and send Team Commands */
    for(int i = 0; i < NUM_OF_BRAINS; i++)
    {
    /******* Send Data to the brains ***********/
      Serial.println("*** Sending First data to Brain: " + String( i + 1 )+ " ******");
      TeamData.team_name = TeamTank[i];
      TeamData.health = 100U;
      TeamData.go = 0U;
      TeamData.time = 2U;
      TeamData.id = i + 1U;

      radio.write(&TeamData, sizeof(StructureOfTeam));

      delay(500);
    }
        
    while(1)
    {
      if(digitalRead(BUTTONPIN) == 1)
      {
        
        break;
      }
    } 

    /* TODO: Iterate the ID's here and send Go commands*/
    for(int i = 1; i <= NUM_OF_BRAINS; i++)
    {
      TeamData.go = 1;
      
      /* ID of slaves */
      TeamData.id = i; 

      /* Starting the game */
      Serial.println(" >>>> Starting the Game Now: <<<< ");
      radio.write(&TeamData, sizeof(StructureOfTeam));

      Serial.println("Data Packet Sent");
      Serial.println("");

      delay(1000);
    }
    
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
 
  Serial.print("Brain ID = ");
  Serial.print(BrainData.brain_id);
 
  Serial.println();

  }

}