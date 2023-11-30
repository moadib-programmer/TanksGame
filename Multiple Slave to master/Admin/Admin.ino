// Load Wi-Fi library
#include <WiFi.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <ESPAsyncWebServer.h>
#include "HTML.h"

#define BUTTONPIN       (12U)
#define NUM_OF_BRAINS   (3U)
#define VOLT_PIN        (39U)
#define RED_LED         (34U)

RF24 radio(4, 5); 

AsyncWebServer server(80);

int teamNum = 0; // Variable to store the team number
String teamNames, tankNames;

// Define an array of team names and team members
String teamNames1[] = {"Team1", "Team2","Team 3"};
String teamMembers1[][4] = 
{
  {"Team Member 1", "Team Member 2", "Team Member 3", "Team Member 4"},
  {"Team Member A", "Team Member B", "Team Member C", "Team Member D"},
  {"Team Member E", "Team Member F", "Team Member I", "Team Member J"},
};

const uint64_t address = 0xF0F0F0F0E1LL;

// Replace with your network credentials
const char* ssid = "SSID";
const char* password = "PASSWORD";

// Auxiliary variables to store the current output state
int startFlag = 0U;
int statusSave = 0u;
int statusScore = 0u;


int teamScores1[NUM_OF_BRAINS] = {100,200,300};
String TeamTank1[NUM_OF_BRAINS] = {"DAVID BLUE", "NIKE GREEN", "NEWL GREEN"};

int waitForStart();

double volt_measure()
{
  int volt = analogRead(VOLT_PIN);// read the input
  double voltage = map(volt,0, 3000, 0, 7.4);// map 0-1023 to 0-2500 and add correction offset
  return voltage + 0.5;
}

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
 
/************* Structure to receive data from Brain *************/
struct StructureOfBrain
{
  int counter;
  int brain_id;
  int health;
};

StructureOfBrain BrainData;
 
 /**
 * @brief Sends the data to the brains
 * 
 */
void sendDataToBrains()
{
  Serial.println("*** Sending Data to brains******");
  delay(200);
  Serial.println();


    /* TODO: Iterate the ID's here and send Team Commands */
  for(int i = 0; i < NUM_OF_BRAINS ; i++)
  {
  /******* Send Data to the brains ***********/
    Serial.println("*** Sending First data to Brain: " + String( i + 1 )+ " ******");
    TeamData.team_name = TeamTank1[i];
    TeamData.health = teamScores1[i];
    TeamData.go = 0U;
    TeamData.time = 2U;
    TeamData.id = i + 1U;

    radio.write(&TeamData, sizeof(StructureOfTeam));

    delay(500);
  }
}

void setup() {
  Serial.begin(115200);

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }

  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
   
  radio.begin();

  pinMode(BUTTONPIN, INPUT);
  
  Serial.println("Transmitter started....");
  radio.openWritingPipe(address); 
  radio.setPALevel(RF24_PA_MIN);  
  radio.stopListening(); 

  Serial.println(" Starting Server ");

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send(200, "text/html", html+dataPage);
  });

    server.on("/save", HTTP_POST, [](AsyncWebServerRequest *request)
    {
      // Get the teamNum value from the submitted form
      if(request->hasParam("teamNum", true))
      {
        teamNum = request->getParam("teamNum", true)->value().toInt();
        Serial.print("Team Number received: ");
        Serial.println(teamNum);
      }

      // Get the team Names from the submitted form
      if(request->hasParam("teamNames", true))
      {
        teamNames = request->getParam("teamNames", true)->value();
        Serial.print("Team Names received: ");
        Serial.println(teamNames);
      }

      // Get the tank Names from the submitted form
      if(request->hasParam("tankNames", true))
      {
        tankNames = request->getParam("tankNames", true)->value();
        Serial.print("tank Names received: ");
        Serial.println(tankNames);
        statusSave = 1;
      }
      delay(1000);

      request->send(200, "text/html", html+startPage);
    });

  server.on("/start", HTTP_POST, [](AsyncWebServerRequest *request)
  {
    Serial.println("Game is being started");

    delay(1000);

    request->send(200, "text/html", html+scorePage);
    statusScore = 1;
  });

  server.begin();

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

    while(1)
  {
    if(statusSave == 1)
    {
      Serial.println ("Loop Broken");

      sendDataToBrains();
      statusSave = 0;
      break;
    }
  }


  while(1)
  {
    if(statusScore)
    {
      Serial.println(" Yaho starting");
      delay(1000);
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
      statusScore = 0;
      break;
    }
    else
    {

    }

    delay(90);
  } 


  if(recvData())
  {
 
  Serial.print("Packet No. = ");
  Serial.println(BrainData.counter);
  
  Serial.print("Health = ");
  Serial.print(BrainData.health);
 
  Serial.print("Brain ID = ");
  Serial.print(BrainData.brain_id);
  

  if(BrainData.brain_id >= 1)
  {
    teamScores1[BrainData.brain_id - 1] = BrainData.health;
  }
 
  Serial.println();

  }

  
}