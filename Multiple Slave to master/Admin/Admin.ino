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
void SendTheData(void);

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
  unsigned char counter;
  unsigned char brain_id;
  unsigned char health;
  unsigned char tankID;
};

StructureOfBrain BrainData;
 
 /**
 * @brief Sends the data to the brains
 * 
 */
void sendDataToBrains()
{
  unsigned char oddIdCounter = 0;
  unsigned char evenIdCounter = 0;

  Serial.println("*** Sending Data to tanks of both teams ******");
  delay(200);
  Serial.println();


  for(int i = 1; i <= (tankNum * 2); i++)
  {

      /* 
      * ID corresponds to the team
      * ID is odd: team A
      * ID is even: team B
      */
      if(i % 2 == 0)
      {
        Serial.println("*** Sending Data to the brain of ID : " + String(i)+ " of team " + team2Name  + "******");
        TeamData.team_name = team2Name + " " + team2TanksNamesArr[oddIdCounter];
        TeamData.health = team2TanksScoresArr[oddIdCounter].toInt();
        Serial.println("\nUpdated Health is " + String(TeamData.health) + " "+ TeamData.team_name);
        oddIdCounter++;
      }
      else
      {
        Serial.println("*** Sending Data to the brain of ID : " + String(i)+ " of team " + team1Name  + "******");
        TeamData.team_name = team1Name + " " + team1TanksNamesArr[evenIdCounter];
        TeamData.health = team1TanksScoresArr[evenIdCounter].toInt();
        Serial.println("\nUpdated Health is " + String(TeamData.health) + " "+ TeamData.team_name);
        evenIdCounter++;
      }
      
      TeamData.go = 0U;
      TeamData.time = gameTime;
      TeamData.id = i;
      radio.write(&TeamData, sizeof(StructureOfTeam));

      delay(100);
  }

  Serial.println(" Data has been sent to all tanks ");
}

void setup() 
{
  Serial.begin(9600);

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

/* Fetching the team information */
server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
{
  request->send(200, "text/html", html + html2 + teamPage);
});

/* Handling the team page */
server.on("/teamData", HTTP_POST, [](AsyncWebServerRequest *request)
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

  if(request->hasParam("time", true))
  {
    gameTime = request->getParam("time", true)->value().toInt();
    Serial.print("GameTime received: ");
    Serial.println(gameTime);
  }

  String tankPageData = html + html2 +  tankPage0;

  for(uint8_t i = 1; i <= tankNum; i++)
  {
    tankPageData += tankPage1 + String(i) + tankPagetank1 + String(i) + tankPage11 + String(i) + tankPagetank2 + String(i) + tankPage12 + String(i) + tankPagetank3 + String(i) + tankPage13 + String(i) + tankPagetank4 + String(i) + tankPage14;
  }

  tankPageData += tankPage2;
  request->send(200, "text/html", tankPageData);
});

/* Handling the tank page */
server.on("/tankData", HTTP_POST, [](AsyncWebServerRequest *request)
{
  for(int i = 1; i <= tankNum; i++)
  {
    /* FOR TEAM 1 */
    if(request->hasParam("tankName" +  String(i) + "1", true))
    {
      team1TanksNamesArr[i - 1] = request->getParam("tankName" +  String(i) + "1", true)->value();
      Serial.println("Team 1 Tank name is: ");
      Serial.println(team1TanksNamesArr[i - 1]);
    }

    if(request->hasParam("tankScore" +  String(i) + "1" , true))
    {
      team1TanksScoresArr[i - 1] = request->getParam("tankScore" +  String(i) + "1" , true)->value() ;
      Serial.println("Team 1 Tank score is: ");
      Serial.println(team1TanksScoresArr[i - 1]);
    }

    /* FOR TEAM 2 */
    if(request->hasParam("tankName" +  String(i) + "2" , true))
    {
      team2TanksNamesArr[i - 1] = request->getParam("tankName" +  String(i) + "2" , true)->value();
      Serial.println("Team 2 Tank name is: ");
      Serial.println(team2TanksNamesArr[i - 1]);
    }

    if(request->hasParam("tankScore" +  String(i) + "2" , true))
    {
      team2TanksScoresArr[i - 1] = request->getParam("tankScore" +  String(i) + "2" , true)->value() ;
      Serial.println("Team 2 Tank score is: ");
      Serial.println(team2TanksScoresArr[i - 1]);
    }
  }

  sendDataToBrains();

  delay(1000);
  request->send(200, "text/html", html + html2+  startPage);
});

  /******* User clicks on Start the Game ********/

  server.on("/start", HTTP_POST, [](AsyncWebServerRequest *request)
  {
    static int startFlag = 0;

    String scoreHtml;

    Serial.println("Game is being started");

    scoreHtml += html + refreshTag + html2 +  scoreHead + String(gameTime) + " minutes</h2>";

    /* Appending Team Names */
    scoreHtml += "<div id='TeamBlock'> <span id='TeamA'>Team " + String(team1Name) + "</span><span id='TeamB'>Team " + String(team2Name) + "</span>";

    for (uint8_t i = 0; i < tankNum; i++)
    {
      /* Appending Tank Name of team 1 */
      scoreHtml += "<span id='ScoreA'> Tank " + String(team1TanksNamesArr[i]) + ": " + String(team1TanksScoresArr[i]) + "</span> <span id='ScoreB'> Tank " + String(team2TanksNamesArr[i]) + ": " + String(team2TanksScoresArr[i]) + "</span>";
    }

    delay(200);

    request->send(200, "text/html",  "</div></body></html>" + scoreHtml);

    if(startFlag == 0)
    {
      SendTheData();
      startFlag = 1;
    }

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
    
    Serial.println("Health = ");
    Serial.print(BrainData.health);
  
    Serial.println("Brain ID = ");
    Serial.print(BrainData.brain_id);

        Serial.println("Tank ID = ");
    Serial.print(BrainData.tankID);

    // /*
    // * Brain ID is even for team 2 tanks and odd for team1 tanks.
    if(BrainData.tankID % 2 == 0)
    {
      /*2 as the index is always zero*/
      team2TanksScoresArr[BrainData.tankID - 2] = BrainData.health;
    }
    else
    {
      team1TanksScoresArr[BrainData.tankID - 1] = BrainData.health;
    }

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
void SendTheData(void)
{ 
    TeamData.go = 1;
    
    /* ID of slaves */
    TeamData.id = 1; 

    /* Starting the game */
    Serial.println(" >>>> Starting the Game Now: <<<< ");
    radio.write(&TeamData, sizeof(StructureOfTeam));

    Serial.println("Data Packet Sent");
    Serial.println("");

    delay(50);
  
    Serial.println("Receiver Started....");

    radio.openReadingPipe(0, address);   //Setting the address at which we will receive the data
    radio.setPALevel(RF24_PA_MAX);       //You can set this as minimum or maximum depending on the distance between the transmitter and receiver.
    radio.startListening();              //This sets the module as receiver

    delay(90);
}