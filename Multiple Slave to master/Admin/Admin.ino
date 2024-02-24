/***************************** Includes ****************************/
#include <WiFi.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <ESPAsyncWebServer.h>
#include "HTML.h"

/***************************** Macros ****************************/

#define BUTTON_PIN      (12U)
#define MAX_TEAMS       (20U)
#define VOLT_PIN        (39U)
#define RED_LED         (34U)

/***************************** Variables ****************************/
RF24 radio(4, 5); 
AsyncWebServer server(80);
uint8_t statusSave = 0u;
uint8_t statusScore = 0u;

String team1Name;
String team2Name;
String team1TankNames; 
String team2TankNames;
String team1TankScores; 
String team2TankScores; 

uint16_t gameTime = 0u;
uint8_t tankNum = 0u; /* Variable to store the team number */

/* Array containing team1 and team2 Tanks Names */
String team1TanksNamesArr[MAX_TEAMS] = {};
String team2TanksNamesArr[MAX_TEAMS] = {};

/* Array containing team1 and team2 Tanks Scores */
String team1TanksScoresArr[MAX_TEAMS] = {};
String team2TanksScoresArr[MAX_TEAMS] = {};

const uint64_t address = 0xF0F0F0F0E1LL;

// Replace with your network credentials
const char* ssid = "SSID";
const char* password = "PASSWORD";


/************************* Function Protoypes ************************/
int waitForStart();
void ProcessTheData(void);



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
  Serial.println("*** Sending Data to tanks of both teams ******");
  delay(200);
  Serial.println();

  for(int i = 0; i < tankNum ; i++)
  {
    /* Sending Data of each tank of Team 1 */
    Serial.println("*** Sending Data to the brain of ID : " + String( i + 1 )+ " of team " + team1Name  + "******");

    /* Send the team name, add a space and then add the tank name */
    TeamData.team_name = team1Name + " " + team1TankNames;
    TeamData.health = team1TankScores.toInt();
    TeamData.go = 0U;
    TeamData.time = gameTime;
    TeamData.id = i + 1U;
    radio.write(&TeamData, sizeof(StructureOfTeam));

    delay(500);
  }

  Serial.println(" Data has been sent to all tanks ");
}

void setup() 
{
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

  pinMode(BUTTON_PIN, INPUT);
  
  Serial.println("Transmitter started....");
  radio.openWritingPipe(address); 
  radio.setPALevel(RF24_PA_MIN);  
  radio.stopListening(); 

/************************************************************
 *                                                          *
 *             Handling Servers responses                   *
 *                                                          *
 ************************************************************/

Serial.println(" Starting Server ");

server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
{
  request->send(200, "text/html", html+dataPage);
});

  server.on("/save", HTTP_POST, [](AsyncWebServerRequest *request)
  {
    if(request->hasParam("team1Name", true))
    {
      team1Name = request->getParam("team1Name", true)->value() ;
      Serial.print("Team1 name received: ");
      Serial.println(team1Name);
    }

    if(request->hasParam("team2Name", true))
    {
      team2Name = request->getParam("team2Name", true)->value() ;
      Serial.print("Team2 name received: ");
      Serial.println(team2Name);
    }

    if(request->hasParam("tankNum", true))
    {
      tankNum = request->getParam("tankNum", true)->value().toInt() ;
      Serial.print("Number of Tanks: ");
      Serial.println(tankNum);
    }

    if(request->hasParam("team1TankNames", true))
    {
      team1TankNames = request->getParam("team1TankNames", true)->value() ;
      Serial.print("Team1 tank names received: ");
      Serial.println(team1TankNames);
    }

    if(request->hasParam("team2TankNames", true))
    {
      team2TankNames = request->getParam("team2TankNames", true)->value();
      Serial.print("Team2tank Name received: ");
      Serial.println(team2TankNames);
    }

    if(request->hasParam("team1TankScores", true))
    {
      team1TankScores = request->getParam("team1TankScores", true)->value() ;
      Serial.print("Team1 tank scores received: ");
      Serial.println(team1TankScores);
    }

    if(request->hasParam("team2TankScores", true))
    {
      team2TankScores = request->getParam("team2TankScores", true)->value() ;
      Serial.print("Team2 tank scores received: ");
      Serial.println(team2TankScores);
    }

    if(request->hasParam("time", true))
    {
      gameTime = request->getParam("time", true)->value().toInt();
      Serial.print("GameTime received: ");
      Serial.println(gameTime);
    }

    // ProcessTheData();

    sendDataToBrains();

    delay(1000);

    request->send(200, "text/html", html+startPage);
  });

  /******* User clicks on Start the Game ********/

  server.on("/start", HTTP_POST, [](AsyncWebServerRequest *request)
  {
    String scoreHtml;

    Serial.println("Game is being started");

    scoreHtml += html + scoreHead + String(gameTime) + " minutes</h2>";

    /* Appending Team Names */
    scoreHtml += "<div id='TeamBlock'> <span id='TeamA'>Team " + String(team1Name) + "</span><span id='TeamB'>Team " + String(team2Name) + "</span>";

    /* Appending Tank Name of team 1 */
    scoreHtml += "<span id='ScoreA'> Tank " + String(team1TankNames) + ": " + String(team1TankScores) + "</span> <span id='ScoreB'> Tank " + String(team2TankNames) + ": " + String(team2TankScores) + "</span></div></body></html>";

    delay(1000);

    request->send(200, "text/html", scoreHtml);
    // statusScore = 1;
    ProcessTheData();
    scoreHtml = " ";
    
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

  if(recvData())
  {
 
    Serial.print("Packet No. = ");
    Serial.println(BrainData.counter);
    
    Serial.print("Health = ");
    Serial.print(BrainData.health);

    team1TankScores = String(BrainData.health);
    Serial.println(team1TankScores);
    
  
    Serial.print("Brain ID = ");
    Serial.print(BrainData.brain_id);

    Serial.println();

  }
}


/**
 * @brief This function processes tank names, scores string, splits them
 *        and populate the tanks array.
 * 
 * @param void
 * 
 * @return void
*/
void ProcessTheData(void)
{

      
    for(int i = 1; i <= tankNum; i++)
    {
      TeamData.go = 1;
      
      /* ID of slaves */
      TeamData.id = i; 

      /* Starting the game */
      Serial.println(" >>>> Starting the Game Now: <<<< ");
      radio.write(&TeamData, sizeof(StructureOfTeam));

      Serial.println("Data Packet Sent");
      Serial.println("");

      delay(500);
    }
  
    Serial.println("Receiver Started....");

    radio.openReadingPipe(0, address);   //Setting the address at which we will receive the data
    radio.setPALevel(RF24_PA_MIN);       //You can set this as minimum or maximum depending on the distance between the transmitter and receiver.
    radio.startListening();              //This sets the module as receiver

    delay(90);
}