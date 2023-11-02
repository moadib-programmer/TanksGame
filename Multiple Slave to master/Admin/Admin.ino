// Load Wi-Fi library
#include <WiFi.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define BUTTONPIN       (12U)
#define NUM_OF_BRAINS   (2U)
#define VOLT_PIN        (39U)
#define RED_LED         (34U)

RF24 radio(4, 5); 

const uint64_t address = 0xF0F0F0F0E1LL;

// Replace with your network credentials
const char* ssid = "SSID";
const char* password = "PASSWORD";

// Set web server port number to 80
WiFiServer server(80);
// Variable to store the HTTP request
String header;
// Auxiliary variables to store the current output state
String StartGame = "";
int startFlag = 0U;

// Define an array of team names and team members
String teamNames[] = {"Team1", "Team2"};
String teamMembers[][4] = 
{
  {"Team Member 1", "Team Member 2", "Team Member 3", "Team Member 4"},
  {"Team Member A", "Team Member B", "Team Member C", "Team Member D"},
};
int teamScores[NUM_OF_BRAINS] = {0,0};
String TeamTank[NUM_OF_BRAINS] = {"DAVID BLUE", "NIKE GREEN"};

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0;
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

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
  server.begin();
   
  radio.begin();

  pinMode(BUTTONPIN, INPUT);
  
  Serial.println("Transmitter started....");
  radio.openWritingPipe(address); //Setting the address where we will send the data
  radio.setPALevel(RF24_PA_MIN);  //You can set it as minimum or maximum depending on the distance between the transmitter and receiver.
  radio.stopListening();          //This sets the module as transmitter

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
    if(waitForStart())
    {
      break;
    }
    else
    {

    }
  } 

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

int waitForStart()
{
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) 
  {                             // If a new client connects,
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected() && currentTime - previousTime <= timeoutTime) 
    { 
      currentTime = millis();
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;

        if (c == '\n') 
        {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:

          if (currentLine.length() == 0) 
          {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            // turns the GPIOs on and off
            if (header.indexOf("GET /12/on") >= 0) 
            {
              Serial.println("\nGAME STARTED\n");
              StartGame = "on";
              return 1;
            } 
            else if (header.indexOf("GET /12/off") >= 0) 
            {
              StartGame = "off";
            } 
  

            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}</style></head>");

            // Web Page Heading
            client.println("<body><h1>Tanks Gamer</h1>");

            // Display 
            client.println("<p>Start Button</p>");

            // If the StartGame is off, it displays the ON button      
            if (StartGame=="off") {
              client.println("<p><a href=\"/12/on\"><button class=\"button\">Start</button></a></p>");
            } 
            else 
            {
              client.println("<p><a href=\"/12/off\"><button class=\"button button2\">Started</button></a></p>");
            }

            client.println("</body></html>");
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } 
          else 
          { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } 
        else if (c != '\r') 
        {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
  
  return 0;
    
}

void loop()
{
  WiFiClient client = server.available();   // Listen for incoming clients

  if(recvData())
  {
 
  Serial.print("Packet No. = ");
  Serial.println(BrainData.counter);
  
  Serial.print("Health = ");
  Serial.print(BrainData.health);
 
  Serial.print("Brain ID = ");
  Serial.print(BrainData.brain_id);

  teamScores[BrainData.brain_id - 1] = BrainData.health;
 
  Serial.println();

  }

 if (client) {                             // If a new client connects,
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client


    while (client.connected() && currentTime - previousTime <= timeoutTime) 
    {  // loop while the client's connected
      currentTime = millis();
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) 
          {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            

            // turns the GPIOs on and off
            if (header.indexOf("GET /GameStart") >= 0) 
            {
              Serial.println("\nGAME STARTED\n");
              StartGame = "on";
              startFlag = 1u;
              teamScores[0] = 10;
            } 

            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");

            // CSS to style the on/off buttons
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #af4cab; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".rectangle { background-color: #3498db; border-radius: 10px; width: 200px; height: 200px; margin: 20px auto;}");
            client.println("</style></head>");

            // Web Page Heading
            client.println("<body><h1>Tanks Games</h1>");

            // Start Button
            client.println("<p>Click to start the game</p>");
            client.println("<p><a href=\"/GameStart\"><button class=\"button\">Start</button></a></p>");

            // Iterate through the teams and generate squares with scores
            for (int i = 0; i < 4; i++) 
            {  // Assuming you have 4 teams
              client.println("<div class=\"rectangle\" style=\"background-color: #32cd32;\">");
              client.print("<h2 style=\"color: white;\">");
              client.print(teamNames[i]);  // Display the team name
              client.println("</h2>");
              client.println("<ul>");

              // Generate the list of team members
              for (int j = 0; j < 4; j++) 
              {  // Assuming each team has 4 members
                client.print("<li style=\"color: white; padding-bottom: 5px;\">");
                client.print(teamMembers[i][j]);  // Display each team member
                client.println("</li>");
              }

              // Display the team's score as a styled list item
              client.print("<li style=\"font-weight: bold; font-size: 22px; color: white;\">");
              client.print("Score: ");
              client.print(teamScores[i]);
              client.println("</li>");

              client.println("</ul>");
              client.println("</div>");
            }


            client.println("</body></html>");

            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } 
          else 
          { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } 
        else if (c != '\r') 
        {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }

    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}