#include <esp_now.h>
#include <WiFi.h>
#include <SPI.h>
#include <Wire.h>
#include <nRF24L01.h>
#include <RF24.h>
 
int GreenLed = 27;

RF24 radio(4, 5); 
const uint64_t address = 0xF0F0F0F0E1LL;
int counter = 0;
 
float temperature;
float humidity;
float altitude;
float pressure;
 
/************* Structure to receive data from Admin *************/
struct StructureOfTeam
{
  String team_name;
  int health;
  unsigned char go = 0;
  unsigned char time = 0;
};

StructureOfTeam TeamData;
 
/************* Structure to send data to Admin *************/
struct StructureOfBrain
{
  int counter;
  String slave_id;
  int health;
};

StructureOfBrain BrainData;


/************* Structure to receive data from Slave *************/
typedef struct StructureOfTargets 
{
  int id;
  int flag;
  int Score;
}StructureOfTargets;

/* Initial Score value */
int Final_Score = 100;

// Create a StructureOfTargets called myData
StructureOfTargets myData;

// Create a structure to hold the readings from each board
StructureOfTargets board1;
StructureOfTargets board2;
StructureOfTargets board3;
StructureOfTargets board4;

// Create an array with all the structures
StructureOfTargets boardsStruct[4] = {board1, board2, board3, board4};

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len) 
{
  char macStr[18];
  Serial.print("Packet received from: ");
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.println(macStr);
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.printf("Board ID %u: %u bytes\n", myData.id, len);
  // Update the structures with the new incoming data
  boardsStruct[myData.id-1].flag = myData.flag;
  boardsStruct[myData.id-1].Score = myData.Score;
  Serial.printf("flag value: %d \n", boardsStruct[myData.id-1].flag);
  Serial.printf("Score value Received: %d \n", boardsStruct[myData.id-1].Score);

  if( (Final_Score - boardsStruct[myData.id-1].Score ) >= 0 ) 
  {
    Final_Score = Final_Score - boardsStruct[myData.id-1].Score;
  }
  else
  {
    Final_Score = 0;
  }

  Serial.printf("Updated Score Value: %d \n", Final_Score);
  
  /* Send new data to the main Admin*/
  BrainData.counter = counter;
  BrainData.health = Final_Score;
  BrainData.slave_id = myData.id;
  
  Serial.println(" ");
  Serial.print("Packet No. = ");
  Serial.println(BrainData.counter);
  
  Serial.print("Health = ");
  Serial.print(BrainData.health);
 
  Serial.print("SlaveID = ");
  Serial.print(BrainData.slave_id);
 
  Serial.println(" ");
  
  radio.write(&BrainData, sizeof(StructureOfBrain));
  
  Serial.println("Data Packet Sent");
  Serial.println("");
  
  counter++;
    
  Serial.println();
}
 
 
int recvData()
{
  if ( radio.available() ) 
  {
    radio.read(&TeamData, sizeof(StructureOfTeam));
    return 1;
  }

    return 0;
}

void setup() {
  //Initialize Serial Monitor
  Serial.begin(115200);
  
  //Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  //Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(OnDataRecv);

  radio.begin();                  //Starting the Wireless communication
  Serial.println("Receiver Started....");

  radio.openReadingPipe(0, address);   //Setting the address at which we will receive the data
  radio.setPALevel(RF24_PA_MIN);       //You can set this as minimum or maximum depending on the distance between the transmitter and receiver.
  radio.startListening();              //This sets the module as receiver

  pinMode(GreenLed, OUTPUT);
  digitalWrite(GreenLed, LOW);

}
 
void loop() {
  while(1)
  {
    if(recvData())
    {
      Serial.print("Team Name = ");
      Serial.println(TeamData.team_name.substring(0,TeamData.team_name.indexOf(" ")));

      Serial.print("Tank Name = ");
      Serial.println(TeamData.team_name.substring(TeamData.team_name.indexOf(" "),TeamData.team_name.length()));
      
      Serial.print("Health Given = ");
      Serial.println(TeamData.health);

      /* Setting Final score equal to health */
      Final_Score = TeamData.health;

      Serial.print("GO = ");
      Serial.println(TeamData.go);

      Serial.print("Time in Minutes = ");
      Serial.println(TeamData.time);

      Serial.println();
      
      Serial.println("Turning ON GREEN");
      digitalWrite(GreenLed, HIGH);

      radio.openWritingPipe(address); //Setting the address where we will send the data
      radio.setPALevel(RF24_PA_MIN);  //You can set it as minimum or maximum depending on the distance between the transmitter and receiver.
      radio.stopListening();          //This sets the module as transmitter
      break;
    }
  }

  // Acess the variables for each board

  /*Board1 Data */
  int board1X = boardsStruct[0].flag;
  int board1Y = boardsStruct[0].Score;
  
  /*Board2 Data*/
  int board2X = boardsStruct[1].flag;
  int board2Y = boardsStruct[1].Score;
  
  /*Board3 Data*/
  int board3X = boardsStruct[2].flag;
  int board3Y = boardsStruct[2].Score;

  /*Board4 Data*/
  int board4X = boardsStruct[3].flag;
  int board4Y = boardsStruct[3].Score;

  delay(500);  
}
