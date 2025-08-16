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
#define MAX_TARGETS     5
#define MAX_PLAYERS     10
#define VOLT_PIN        39
#define RED_LED         34
#define GREEN_LED       35

#define NRF_ADDRS       0xF0F0F0F0E1LL
#define SSID            "Network"         //replace with your netwrok SSID
#define PASSWORD        "PASSWORD"     //replace with your netwrok Password


/***************************** Globals ****************************/
RF24 radio(4, 5); 
WebServer server(80);
uint8_t statusSave = 0u;
uint8_t statusScore = 0u;

uint16_t gameTime = 0;
uint8_t tankNum = 0;
uint8_t teamNum = 0;     //Number of Teams
uint8_t hitScore = 0;    // Score subtracted when target gets a HIT
uint8_t targetNum = 0;   // number of targets per player
int totalScore = 0;  //Total score of the Game
bool target_soft_config = false; // variable to save soft/hard or only hard hit choice.

/* Array to store the data of the teams, players and targets */
String teamsNameArr[MAX_TEAMS] = {};
String playerNamesArr[MAX_TEAMS][MAX_PLAYERS] = {};
String targetScoresArrHard[MAX_TEAMS][MAX_PLAYERS][MAX_TARGETS] = {};
String targetScoresArrSoft[MAX_TEAMS][MAX_PLAYERS][MAX_TARGETS] = {};
int teamsScoresArr[MAX_TEAMS][MAX_PLAYERS] = {0};

/* If players will have different scores as well */
// String teamsPlayerScoresArr[MAX_TEAMS][MAX_PLAYERS] = {};

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
  uint8_t go = 0;
  uint8_t time = 0;
  uint8_t tank_id = 0;
  uint8_t team_id = 0;
  uint8_t target_num = 0;
  uint8_t target_soft_config_flag = 0;
  uint8_t targetSoftScores[MAX_TARGETS] = {0};
  uint8_t targetHardScores[MAX_TARGETS] = {0};
};

StructureOfTeam TeamData;
 
/************* Structure to receive data from Brain *************/
struct StructureOfBrain
{
  unsigned char counter;
  unsigned char tank_id;
  unsigned char team_id;
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

  Serial.println("*** Starting Data transmission ******");
  delay(100);
  Serial.println();

  TeamData.health = totalScore;
  TeamData.go = 0;
  TeamData.time = gameTime;
  TeamData.target_num = targetNum;

  for(int team = 1; team <= teamNum; team++)
  {
    /*TODO: Add the implementation for the Team division based on the IDs */

    for(int player = 1; player <= tankNum; player++)
    {
      TeamData.team_name = teamsNameArr[team - 1] + " " + playerNamesArr[team - 1][player - 1];              // Team Name + Tank Name (Player Name)
      TeamData.tank_id = player;
      TeamData.team_id = team;
  
      Serial.println("*** Sending Data to the brain of team ID: "+ String(TeamData.team_id) + " and Tank ID" + String(TeamData.tank_id ) + " of team " + teamsNameArr[team - 1]  + "******");

      /* TODO: Extracting and populating Target Scores to be minus */

      TeamData.target_soft_config_flag = target_soft_config;
      
      if(target_soft_config)
      {
        for(int target = 1; target <= targetNum; target++)
        {
          /* Extracting target soft scores */
          TeamData.targetSoftScores[target - 1] = targetScoresArrSoft[team - 1][player - 1][target - 1].toInt();
          Serial.println("Target " + String(target) + " score: " + TeamData.targetSoftScores[target - 1]);
          
          /* Extracting target hard scores */
          TeamData.targetHardScores[target - 1] = targetScoresArrHard[team - 1][player - 1][target - 1].toInt();
          Serial.println("Target " + String(target) + " score: " + TeamData.targetHardScores[target - 1]);
        }

      }
      else
      {
        for(int target = 1; target <= targetNum; target++)
        {
          /* Extracting target number */
          TeamData.targetHardScores[target - 1] = targetScoresArrHard[team - 1][player - 1][target - 1].toInt();
          Serial.println("Target " + String(target) + " score: " + TeamData.targetHardScores[target - 1]);
        }
      }

      radio.write(&TeamData, sizeof(StructureOfTeam));

      Serial.println("The data being sent is as follows: ");
      Serial.println("Team Name & Tank Name: " + TeamData.team_name);
      Serial.println("Target Numbers are: " + TeamData.target_num);
      
    }
  }

delay(100);

Serial.println(" Data transmission Successful ");

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
  if (server.hasArg("targetNum")) 
  {
    targetNum = server.arg("targetNum").toInt();
    Serial.print("Number of Targets: ");
    Serial.println(targetNum);
  }
  if (server.hasArg("totalScore")) 
  {
    totalScore = server.arg("totalScore").toInt();
    Serial.print("Total Score: ");
    Serial.println(totalScore);
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
  if (server.hasArg("time")) 
  {
    gameTime = server.arg("time").toInt();
    Serial.print("GameTime received: ");
    Serial.println(gameTime);
  }

  if (server.hasArg("targetConfig")) 
  {
    String targetConfigChoice = server.arg("targetConfig");
    Serial.print("Target Config Option: ");
    Serial.println(targetConfigChoice);

    if (targetConfigChoice == "yes") 
    {
      Serial.println("He said Yes!");
      target_soft_config = true;
    }
  }


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
        if (target_soft_config) 
        {
          // --- Soft + Hard Inputs ---
          htmlContent += targetInputStartSoftHard;  
          htmlContent += String(target);
          htmlContent += targetInputMidSoft;  
          htmlContent += String(team) + String(player) + String(target);  
          htmlContent += targetInputSoft;  
          htmlContent += String(team) + String(player) + String(target);  
          htmlContent += targetInputHard;  
        }
        else 
        {
          // --- Single Input ---
          htmlContent += targetInputStart + team + String(player) + String(target) + targetInputMid + target + targetInputEnd;
        }
      }
    }
  }

    htmlContent += "</div>"; // Close the form-block div

    htmlContent += htmlFooter;

  server.send(200, "text/html", htmlContent);
}

void handleTankData() 
{
  /**
   * Supposing, each team has fix number of players not variable.
  */
  /* Change the logic for the tank Data as well as the extraction of the arguments */

  for(int team = 1; team <= teamNum; team++)
  {
    /* Extracting and populating Team Names */
    if(server.hasArg("team" +  String(team) + "Name"))
    {
      teamsNameArr[team - 1] = server.arg("team" +  String(team) + "Name");
      Serial.println("Team" + String(team) + "name is: ");
      Serial.println("\n******* FOR TEAM " + String(team) + "********");
      Serial.println(teamsNameArr[team - 1]);
    }

    // Player names are stored in the 2D array consist of 0 index(team 1) -> {{"player1", "player2"}}
    for(int player = 1; player <= tankNum; player++)
    {
      /* Extracting and populating player names */
      if(server.hasArg("player" + String(team) + String(player) ) )
      {
        playerNamesArr[team - 1][player - 1] = server.arg("player" + String(team) + String(player));
        Serial.println("Team " + String(team) + " and player" + String(player) + " name is: ");
        Serial.println(playerNamesArr[team - 1][player - 1]);
      }

      /* Extracting and populating Target minus Scores */
      for(int target = 1; target <= targetNum; target++)
      {
        if(target_soft_config)
        {
          if(server.hasArg("target" + String(team) + String(player) + String(target) + "SoftScore") )
          {
            targetScoresArrSoft[team - 1][player - 1][target - 1] = server.arg("target" + String(team) + String(player) + String(target) + "SoftScore");
            Serial.println("Target of team " + String(team) + " and player " + String(player) + "and target  " + String(target) + ", Soft score to be minus is: ");
            Serial.println(targetScoresArrSoft[team - 1][player - 1][target - 1]);
          }
          if(server.hasArg("target" + String(team) + String(player) + String(target) + "HardScore") )
          {
            targetScoresArrHard[team - 1][player - 1][target - 1] = server.arg("target" + String(team) + String(player) + String(target) + "HardScore");
            Serial.println("Target of team " + String(team) + " and player " + String(player) + "and target " + String(target) + ", Hard score to be minus is: ");
            Serial.println(targetScoresArrHard[team - 1][player - 1][target - 1]);
          }
        }
        else
        {
          /* Extracting target scores */
          if(server.hasArg("target" + String(team) + String(player) + String(target) + "Score") )
          {
            targetScoresArrHard[team - 1][player - 1][target - 1] = server.arg("target" + String(team) + String(player) + String(target) + "Score");
            Serial.println("Target of team " + String(team) + " and player " + String(player) + "and target " + String(target) + ", score to be minus is: ");
            Serial.println(targetScoresArrHard[team - 1][player - 1][target - 1]);
          }
        }
      }
    }
  }

  sendDataToBrains();

  delay(500);
  server.send(200, "text/html", html + html2+  startPage);
}

void handleStart() 
{
    static int startFlag = 0;

    Serial.println("Game is being started");

    String htmlContent = htmlHeaderResult;
    htmlContent += "<h2>Game Time: " + String(gameTime) + "</h2>";

    for (int team = 0; team < teamNum; team++) 
    {
        htmlContent += "<div class=\"team\">";
        htmlContent += "<h3>" + teamsNameArr[team] + "</h3>";

        for (int player = 0; player < tankNum; player++) 
        {
          htmlContent += "<div class=\"player\">";
          htmlContent += "<span>" + playerNamesArr[team][player] + "</span>";

          /**
           * TODO: add the logic for multiple scores with teams here.
           * Make an array teamsScore[team][tank], and then update the array in the main loop
           *      when the target is hit and updated score is shared.
           * 
          */
          htmlContent += "<span>Total Score: " + String(teamsScoresArr[team][player]) + "</span>";

          /* TODO: Add the logic for batteries of the targets here */
          htmlContent += "<span>Battery: 100% </span>";
          htmlContent += "</div>";
        }

        htmlContent += "</div>"; // Close team div
    }

    // htmlContent += "<h2>Total Game Score: " + String(totalScore) + "</h2>";
    htmlContent += htmlFooterResult;

    // Send the generated HTML to the client
    server.send(200, "text/html", htmlContent);

    if(startFlag == 0)
    {
      SendTheData();
      startFlag = 1;
    }

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
server.on("/new", HTTP_GET, handleNew);
server.on("/teamData", HTTP_POST, handleTeamData);
server.on("/tankData", HTTP_POST, handleTankData);
server.on("/start", HTTP_GET, handleStart);
server.on("/start", HTTP_POST, handleStart);

/* TODO: Add the RESET command here, TeamData.go = 2 is the reset command */

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
  
    Serial.println("Team ID = ");
    Serial.print(BrainData.team_id);

    Serial.println("Tank ID = ");
    Serial.print(BrainData.tank_id);

    totalScore = BrainData.health;

    teamsScoresArr[BrainData.team_id - 1][BrainData.tank_id - 1] = BrainData.health;

    /**
     * TODO: Add the logic to update the score of the Concerned Brain and Admin data only 
     * Get the ID break it apart into two Int's, and then tanksScore[id[1] - 1][id[2] - 1], 
     * update the score.
     * 
     * TODO: Try sending the html page manually after this command, this will remove the automatic refreshing 
     *       of the page.
    */

    // /*
    // * Brain ID is even for team 2 tanks and odd for team1 tanks.
    // if(BrainData.tankID % 2 == 0)
    // {
    //   /* 2 as the index is always zero*/
    //   team2TanksScoresArr[BrainData.tankID - 2] = BrainData.health;
    // }
    // else
    // {
    //   team1TanksScoresArr[BrainData.tankID - 1] = BrainData.health;
    // }
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