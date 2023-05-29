#include <esp_now.h>
#include <WiFi.h>
#include <SPI.h>
#include <Wire.h>
#include <nRF24L01.h>
#include <RF24.h>
 
/* ID of this Brain */
#define ID  (1U)
int GameEndFlag = 0;

// MAC addresses of all slaves
uint8_t slaveMACAddress[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0x66};

/**
 * @brief HIT Brief
 * If 
 * id = 1, front hit
 * id = 2, side hit
 * id = 3, back hit
 */

int GreenLed = 27;
unsigned long StartTime = 0;
unsigned long TotalTime = 0;
unsigned long TimeLeft = 0;

RF24 radio(4, 5); 
const uint64_t address = 0xF0F0F0F0E1LL;
int counter = 0;

String TankName = "";
String TeamName = "";
 
/************* Structure to receive data from Admin *************/
struct StructureOfTeam
{
  String team_name;
  int health;
  unsigned char go = 0;
  unsigned char time = 0;
  unsigned char id = 0;
};

StructureOfTeam TeamData;
 
/************* Structure to send data to Admin *************/
struct StructureOfBrain
{
  int counter;
  int brain_id;
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

/************* Structure to send data to Slave *************/
typedef struct StructureOfSlaves 
{
  int health;       // must be unique for each sender board
} StructureOfSlaves;

StructureOfSlaves slaveData;

// Create peer interface
esp_now_peer_info_t peerInfo;

// Create a structure to hold the readings from each board
StructureOfTargets board1;
StructureOfTargets board2;
StructureOfTargets board3;
StructureOfTargets board4;

// Create an array with all the structures
StructureOfTargets boardsStruct[4] = {board1, board2, board3, board4};


// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len) 
{
  if(GameEndFlag == 0)
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
    SendNextionCommand("health", String(Final_Score));
    SendNextionCommand("health", String(Final_Score));

    /********* Send Hit Status **********/
    switch (myData.id)
    {
    case 1:
      SendNextionCommand("t5", String("FRONT HIT"));
      SendNextionCommand("t5", String("FRONT HIT"));
      break;
    case 2:
      SendNextionCommand("t5", String("SIDE HIT"));
      SendNextionCommand("t5", String("SIDE HIT"));
      break;
    case 3:
      SendNextionCommand("t5", String("BACK HIT"));
      SendNextionCommand("t5", String("BACK HIT"));
      break;
    
    default:
      break;
    }

    
  void SendNextionCommand(String object, String msg);

    
    /********** Sending data to the admin **************/
    BrainData.counter = counter;
    BrainData.health = Final_Score;
    BrainData.brain_id = ID;
    
    Serial.println(" ");
    Serial.print("Packet No. = ");
    Serial.println(BrainData.counter);
    
    Serial.print("Health = ");
    Serial.print(BrainData.health);
  
    Serial.print("Brain ID = ");
    Serial.print(BrainData.brain_id);
  
    Serial.println(" ");
    
    radio.write(&BrainData, sizeof(StructureOfBrain));
    
    Serial.println("Data Packet Sent");
    Serial.println("");
    
    counter++;
      
    Serial.println();

    /************ Sending updated health to salve **************/

    // Register peer
    memcpy(peerInfo.peer_addr, slaveMACAddress, 6);
    peerInfo.channel = 0;  
    peerInfo.encrypt = false;
    
    // Add peer        
    if (esp_now_add_peer(&peerInfo) != ESP_OK)
    {
      Serial.println("Failed to add peer");
    }
    
    delay(50);
    
    // Send message via ESP-NOW
    Serial.println("*** Sending the Score now ****");
    slaveData.health = Final_Score;
    esp_err_t result = esp_now_send(slaveMACAddress, (uint8_t *) &slaveData, sizeof(slaveData));
    
    if (result == ESP_OK) 
    {
      Serial.println("Sent Score");
    }
    else 
    {
      Serial.println("Error sending the data");
    }
  }

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
  Serial.begin(9600);
  
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
  
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);

  radio.begin();                  //Starting the Wireless communication
  Serial.println("Receiver Started....");

  radio.openReadingPipe(0, address);   //Setting the address at which we will receive the data
  radio.setPALevel(RF24_PA_MIN);       //You can set this as minimum or maximum depending on the distance between the transmitter and receiver.
  radio.startListening();              //This sets the module as receiver

  pinMode(GreenLed, OUTPUT);
  digitalWrite(GreenLed, LOW);

  SendNextionCommand("start", String(" "));
  SendNextionCommand("start", String(" "));

}
 
void SendNextionCommand(String object, String msg)
{
  String command = "";
  command = object+".txt=\""+String(msg)+"\"";
  Serial.print(command);

  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);

  delay(50);
}

void loop() {
  while(TeamData.go == 0)
  {
    if(recvData())
    {
      if( (TeamData.go == 0) and (TeamData.id == ID))
      {
        Serial.print("Team Name = ");
        TeamName = TeamData.team_name.substring(0,TeamData.team_name.indexOf(" "));
        Serial.println(TeamName);

        Serial.print("Tank Name = ");
        TankName = TeamData.team_name.substring(TeamData.team_name.indexOf(" "),TeamData.team_name.length());
        Serial.println(TankName);
        
        Serial.print("Health Given = ");
        Serial.println(TeamData.health);

        /* Setting Final score equal to health */
        Final_Score = TeamData.health;

        Serial.print("GO = ");
        Serial.println(TeamData.go);

        Serial.print("Time in Seconds = ");
        TotalTime = TeamData.time * 60;
        Serial.println(TeamData.time);

        Serial.println();
    
        Serial.println("Turning ON GREEN");
        

        for (int i = 0; i <= 2; i++)
        { 
          digitalWrite(GreenLed, HIGH);
          delay(100);
          digitalWrite(GreenLed, LOW);
          delay(100);
        }

        /* Updating data on Nextion HMI */
        SendNextionCommand("time", String(TeamData.time)+" : "+"00");
        SendNextionCommand("time", String(TeamData.time)+" : "+"00");
        SendNextionCommand("health", String(TeamData.health));
        SendNextionCommand("team", String(TeamName));
        SendNextionCommand("tank", String(TankName));
        SendNextionCommand("t5", String("Neutral"));
        SendNextionCommand("start", String(" "));
        SendNextionCommand("start", String(" "));

                // Register peer
        memcpy(peerInfo.peer_addr, slaveMACAddress, 6);
        peerInfo.channel = 0;  
        peerInfo.encrypt = false;
        
        // Add peer        
        if (esp_now_add_peer(&peerInfo) != ESP_OK)
        {
          Serial.println("Failed to add peer");
        }
        
        delay(50);
        
        // Send message via ESP-NOW
        Serial.println("*** Sending the Score now ****");
        slaveData.health = Final_Score;
        esp_err_t result = esp_now_send(slaveMACAddress, (uint8_t *) &slaveData, sizeof(slaveData));
        
        if (result == ESP_OK) 
        {
          Serial.println("Sent Score");
        }
        else 
        {
          Serial.println("Error sending the data");
        }
      }

      if((TeamData.go) and (TeamData.id == ID))
      {
        SendNextionCommand("start", String("3"));
        delay(1000);
        SendNextionCommand("start", String("2"));
        delay(1000);
        SendNextionCommand("start", String("1"));
        delay(1000);       

        SendNextionCommand("start", String("START"));
        delay(2000);
        
        SendNextionCommand("start", String(" "));
        delay(1000);          

        digitalWrite(GreenLed, HIGH);

        radio.openWritingPipe(address); //Setting the address where we will send the data
        radio.setPALevel(RF24_PA_MIN);  //You can set it as minimum or maximum depending on the distance between the transmitter and receiver.
        radio.stopListening();          //This sets the module as transmitter

        /*TODO: Add millisecond part and then update it under void Loop */
        StartTime = millis();
        Serial.println("Start Time : "+ String(StartTime));

        break;
      }
    }
  }


  if( ((millis() - StartTime) / 1000) <= TotalTime )
  {
    int minute = 0;
    int seconds = 0;
    String countdown = "";

    TimeLeft =  TotalTime - ((millis() - StartTime) / 1000);
    minute = (int) TimeLeft / 60;
    seconds = TimeLeft - (minute * 60);

    if(minute < 10)
    {
      countdown = "0" + String(minute) + " : "+ String(seconds);
    }
    if(seconds < 10)
    {
      countdown = String(minute) + " : 0"+ String(seconds);
    }
    
    
    Serial.println("");
    SendNextionCommand("time", String(countdown));
    SendNextionCommand("time", String(countdown));
    delay(100);
  }
  else
  {
    Serial.printf("\nTime left : %d", 0);

    SendNextionCommand("time", String(0)); 
    SendNextionCommand("time", String(0)); 
    SendNextionCommand("start", String("Game Ended")); 

    /* TODO: Add the ending of game logic here */
    GameEndFlag = 1;
    while(1);
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
