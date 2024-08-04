/**
 * @brief This file contains the API's implementation of the Admin side of 
 * the project. The admin includes the Initiation of WiFI SoftAP and then 
 * hosting the local webpage. The admin is responsible of taking all of the
 * input data of the players and the targets. 
 * 
 * Then it transmits all of the information to the players which in turn transmits
 * all of the information to the targets.
 * 
 * @author Muhammad Moadib Nasir
 * 
*/
/***************************** Includes ****************************/
#include <WiFi.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <WebServer.h>
#include "HTML.h"
#include "WelcomePage.h"
#include "InfoPage.h"

/***************************** Macros ****************************/

#define BUTTON_PIN      12
#define MAX_TEAMS       20
#define VOLT_PIN        39
#define RED_LED         34
#define GREEN_LED       35

#define NRF_ADDRS       0xF0F0F0F0E1LL
#define SSID            "SSID"         //replace with your netwrok SSID
#define PASSWORD        "PASSWORD"     //replace with your netwrok Password

/***************************** Globals ****************************/
RF24 radio(4, 5); 
WebServer server(80);
uint8_t statusSave = 0u;
uint8_t statusScore = 0u;

String team1Name;
String team2Name;

uint16_t gameTime = 0;
uint8_t tankNum = 0;
uint8_t teamNum = 0;  //Number of Teams
uint8_t hitScore = 0; // Score subtracted when target gets a HIT
uint8_t targetNum = 0; // number of targets per player

/* Array containing team1 and team2 Tanks Names */
String team1TanksNamesArr[MAX_TEAMS] = {};
String team2TanksNamesArr[MAX_TEAMS] = {};

/* Array containing team1 and team2 Tanks Scores */
String team1TanksScoresArr[MAX_TEAMS] = {};
String team2TanksScoresArr[MAX_TEAMS] = {};

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
 * @param None
 * @return None
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
        /* for Team B */
        Serial.println("*** Sending Data to the brain of ID : " + String(i)+ " of team " + team2Name  + "******");
        TeamData.team_name = team2Name + " " + team2TanksNamesArr[oddIdCounter];
        TeamData.health = team2TanksScoresArr[oddIdCounter].toInt();
        Serial.println("\nUpdated Health is " + String(TeamData.health) + " "+ TeamData.team_name);
        oddIdCounter++;
      }
      else
      {
        /* for Team A */
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

void handleRoot() 
{
  server.send(200, "text/html", page1);
}

void handleNew()
{
  server.send(200, "text/html", html + html2 + teamPage);
}

void handleTeamData() 
{
  if (server.hasArg("team1Name")) 
  {
    team1Name = server.arg("team1Name");
    Serial.print("Team1 name received: ");
    Serial.println(team1Name);
  }
  if (server.hasArg("targetNum")) 
  {
    targetNum = server.arg("targetNum").toInt();
    Serial.print("Number of Targets: ");
    Serial.println(targetNum);
  }
  if (server.hasArg("team2Name")) 
  {
    team2Name = server.arg("team2Name");
    Serial.print("Team2 name received: ");
    Serial.println(team2Name);
  }  
  if (server.hasArg("tankNum")) 
  {
    tankNum = server.arg("tankNum").toInt();
    Serial.print("Number of Tanks: ");
    Serial.println(tankNum);
  }
  if (server.hasArg("teamNum")) 
  {
    teamNum = server.arg("teamNum").toInt();
    Serial.print("Number of Teams: ");
    Serial.println(teamNum);
  }
  if (server.hasArg("hitScore")) 
  {
    hitScore = server.arg("hitScore").toInt();
    Serial.print("Points Minus when HIT: ");
    Serial.println(hitScore);
  }  
  if (server.hasArg("time")) 
  {
    gameTime = server.arg("time").toInt();
    Serial.print("GameTime received: ");
    Serial.println(gameTime);
  }  

  // String tankPageData = html + html2 +  tankPage0;

  // for(uint8_t i = 1; i <= tankNum; i++)
  // {
  //   tankPageData += tankPage1 + String(i) + tankPagetank1 + String(i) + tankPage11 + String(i) + tankPagetank2 + String(i) + tankPage12 + String(i) + tankPagetank3 + String(i) + tankPage13 + String(i) + tankPagetank4 + String(i) + tankPage14;
  // }

  // tankPageData += tankPage2;


  /* new logic implementation */
      String htmlContent = htmlHeader;

    for (int team = 1; team <= teamNum; team++) 
    {
        htmlContent += teamFormStart + team + teamFormMid + team + teamFormEnd + team + teamFormEnd2;

        for (int player = 1; player <= tankNum; player++) 
        {
            htmlContent += playerFormStart + team + String(player) + playerFormMid + player + playerFormEnd;

            for (int target = 1; target <= targetNum; target++) 
            {
                htmlContent += targetInputStart + team + String(player) + String(target) + targetInputMid + target + targetInputEnd;
            }
        }

        htmlContent += "</div>"; // Close the form-block div
    }

    htmlContent += htmlFooter;

  server.send(200, "text/html", htmlContent);

}

void handleTankData() 
{
  /* Change the logic for the tank Data as well as the extraction of the arguments */
  for(int i = 1; i <= tankNum; i++)
  {
    /* FOR TEAM 1 */
    if(server.hasArg("tankName" +  String(i) + "1"))
    {
      team1TanksNamesArr[i - 1] = server.arg("tankName" +  String(i) + "1");
      Serial.println("Team 1 Tank name is: ");
      Serial.println(team1TanksNamesArr[i - 1]);
    }

    if(server.hasArg("tankScore" +  String(i) + "1"))
    {
      team1TanksScoresArr[i - 1] = server.arg("tankScore" +  String(i) + "1");
      Serial.println("Team 1 Tank score is: ");
      Serial.println(team1TanksScoresArr[i - 1]);
    }

    /* FOR TEAM 2 */
    if(server.hasArg("tankName" +  String(i) + "2"))
    {
      team2TanksNamesArr[i - 1] = server.arg("tankName" +  String(i) + "2");
      Serial.println("Team 2 Tank name is: ");
      Serial.println(team2TanksNamesArr[i - 1]);
    }

    if(server.hasArg("tankScore" +  String(i) + "2"))
    {
      team2TanksScoresArr[i - 1] = server.arg("tankScore" +  String(i) + "2");
      Serial.println("Team 2 Tank score is: ");
      Serial.println(team2TanksScoresArr[i - 1]);
    }
  }

  sendDataToBrains();

  delay(500);
  server.send(200, "text/html", html + html2+  startPage);
}

void handleStart() 
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

    delay(50);

    server.send(200, "text/html",  "</div></body></html>" + scoreHtml);

    if(startFlag == 0)
    {
      SendTheData();
      startFlag = 1;
    }

    scoreHtml = " "; 

}

void setup() 
{
  Serial.begin(9600);
  pinMode(BUTTON_PIN, INPUT);
  pinMode(GREEN_LED, OUTPUT);

  /* Setting up the access point */
  Serial.print("Setting AP (Access Point)…");
  WiFi.softAP(SSID, PASSWORD);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  /* Turning Green LED ON for a time */
  digitalWrite(GREEN_LED, 1);
  delay(2000);
  digitalWrite(GREEN_LED, 0);
   
  radio.begin();
  
  radio.openWritingPipe(NRF_ADDRS); 
  radio.setPALevel(RF24_PA_MIN);
  radio.stopListening();
  Serial.println("Transmitter started....");

/************************************************************
 *                                                          *
 *             Handling Servers responses                   *
 *                                                          *
 ************************************************************/

Serial.println(" Starting Server ");

server.on("/", HTTP_GET, handleRoot);
server.on("/tankData", HTTP_POST, handleTankData);
server.on("/teamData", HTTP_POST, handleTeamData);
server.on("/start", HTTP_GET, handleStart);
server.on("/start", HTTP_POST, handleStart);
server.on("/new", HTTP_GET, handleNew);

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
  delay(5);

  server.handleClient();

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
      /* 2 as the index is always zero*/
      team2TanksScoresArr[BrainData.tankID - 2] = BrainData.health;
    }
    else
    {
      team1TanksScoresArr[BrainData.tankID - 1] = BrainData.health;
    }
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

    radio.openReadingPipe(0, NRF_ADDRS);   //Setting the address at which we will receive the data
    radio.setPALevel(RF24_PA_MAX);       //You can set this as minimum or maximum depending on the distance between the transmitter and receiver.
    radio.startListening();              //This sets the module as receiver

    delay(90);
}