#include <esp_now.h>
#include <WiFi.h>
#include <Adafruit_NeoPixel.h>
#include <esp_wifi.h>

/* LED's on each target would be 8 connected to PIN 15 */
#define PIN 15
#define NUM 9
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUM,PIN, NEO_GRB + NEO_KHZ800);

// Set your new MAC Address
uint8_t newMACAddress[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0x67};

#define  BUTTON_PIN    23

/**
 * @brief HIT Brief
 * If 
 * id = 1, front hit
 * id = 2, side hit
 * id = 3, back hit
 */
#define  ID            (3U)

/* Max and min score,
  When ball will hit, 
    The minimun score which should be deducted when ball hits
    at MIN SPEED.
    The maximum score which should be deducted when ball will 
    hit at FULL SPEED.
*/
#define MIN_SCORE     5
#define MAX_SCORE     30

/* Replace with Receiver's MAC Address 
brain 1:
78:21:84:C7:05:38

brain 2:
70:B8:F6:5B:F8:B8
*/
uint8_t broadcastAddress[] = {0x70, 0xB8, 0xF6, 0x5B, 0xF8, 0xB8};
int Health = 0U;

/******* Structure to send data to the brain ********/
/*
  Explaining Structures :
      flag -> 1 positive
      flag -> 2 Negative
*/
typedef struct struct_message {
    int id;       // must be unique for each sender board
    int flag;     // 1 -> ADD, 2 -> Subtract
    int Score;    // Score to be sent
} struct_message;

// Create a struct_message called myData
struct_message myData;

/************* Structure to receive data from brain *************/
typedef struct StructureOfSlaves {
    int health;       // must be unique for each sender board
} StructureOfSlaves;

StructureOfSlaves slaveData;

// Create peer interface
esp_now_peer_info_t peerInfo;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&slaveData, incomingData, sizeof(slaveData));
  Serial.print("Bytes received: ");
  Serial.println(len);
  Serial.print("Health: ");
  Serial.println(slaveData.health);

  /****************** Setting Neopixel depending on HEALTH *******************/
  if( (slaveData.health >= 70) and (slaveData.health <= 100) )
  {
    /* Setting neopixel to green, if score is between 70 and 100 */
    setneopixel(0, 255, 0);
  }
  else if ( (slaveData.health >= 40) and (slaveData.health <= 69) )
  {
    /* Setting neopixel to yellow, if score is between 40 and 69 */
    setneopixel(255,255,0);
  }
  else if ( (slaveData.health >= 0) and (slaveData.health <= 39) )
  {
    /* Setting neopixel to yellow, if score is between 40 and 69 */
    setneopixel(255, 0, 0);
  }

  Serial.println();
  delay(50);
}
 
void setup() {
  // Init Serial Monitor
  Serial.begin(9600);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  esp_wifi_set_mac(WIFI_IF_STA, &newMACAddress[0]);

  Serial.println(" ***** New mac address set ********");
  delay(500);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);
  
  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    Serial.println("Failed to add peer");
    return;
  }

  Serial.println("******* Setting the LED to GREEN ********");
  pinMode(BUTTON_PIN,INPUT);
  
  pixels.begin();

  /* Turning ON GREEN */
  setneopixel(0, 255, 0);
}


void Target_hit()
{
  Serial.println("***** BUTTON PRESSED ****** ");
  delay(100);

  for (int i = 0; i <= 2; i++)
  {
    setneopixel(255, 0, 0);
    delay(600);
    setneopixel(0, 0, 0);
    delay(600);
  }
}

void setneopixel(int r, int g, int b)
{
  for(int i=0; i<=NUM; i++)
  {
    pixels.setPixelColor(i, pixels.Color(r,g,b));
    pixels.show();
  }
}

void loop() 
{
  // ID 2 for target 2
  myData.id = ID;
  myData.flag = 1;
  myData.Score = MAX_SCORE;

  /****** When target is hit ******/
  if(digitalRead(BUTTON_PIN) == 1)
  {
    // Blinking RED LED 3 times
    Target_hit();

    // Send message via ESP-NOW
    Serial.println("*** Sending the Score now ****");
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
    
    if (result == ESP_OK) 
    {
      Serial.println("Sent Score");
      Serial.println(" ");
      Serial.println("Awaiting Score results ");
    }
    else 
    {
      Serial.println("Error sending the data");
    }
  }
}