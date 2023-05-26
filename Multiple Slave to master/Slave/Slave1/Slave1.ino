#include <esp_now.h>
#include <WiFi.h>
#include <Adafruit_NeoPixel.h>

/* LED's on each target would be 8 connected to PIN 15 */
#define PIN 15
#define NUM 8
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUM,PIN, NEO_GRB + NEO_KHZ800);

#define  BUTTON_PIN    23
#define  RED_LED       22
#define  GREEN_LED     21

/**
 * @brief HIT Brief
 * If 
 * id = 1, front hit
 * id = 2, side hit
 * id = 3, back hit
 */
#define  ID            (1U)

/* Max and min score,
  When ball will hit, 
    The minimun score which should be deducted when ball hits
    at MIN SPEED.
    The maximum score which should be deducted when ball will 
    hit at FULL SPEED.
*/
#define MIN_SCORE     5
#define MAX_SCORE     10

/* Replace with Receiver's MAC Address 
brain 1:
78:21:84:C7:05:38

brain 2:
70:B8:F6:5B:F8:B8
*/
uint8_t broadcastAddress[] = {0x70, 0xB8, 0xF6, 0x5B, 0xF8, 0xB8};


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
  Serial.println();
  delay(50);
}
 
void setup() {
  // Init Serial Monitor
  Serial.begin(115200);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

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

  Serial.println("Setting the directions for the pin");
  pinMode(BUTTON_PIN,INPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);

  /* Setting Green LED ON and RED_LED OFF. */
  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, HIGH);

  /* Turning ON GREEN */
  setneopixel(0, 255, 0);

  pixels.begin();

}


void Target_hit()
{
  Serial.println("***** BUTTON PRESSED ****** ");
  delay(100);

  for (int i = 0; i <= 2; i++)
  {
    digitalWrite(RED_LED, HIGH);
    setneopixel(255, 0, 0);
    delay(600);
    digitalWrite(RED_LED, LOW);
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
  digitalWrite(GREEN_LED, HIGH);
  setneopixel(0, 255, 0);
  
  // ID 1 for target 1
  myData.id = ID;
  myData.flag = 1;
  myData.Score = MAX_SCORE;

  /****** When target is hit ******/
  if(digitalRead(BUTTON_PIN) == 1)
  {
    digitalWrite(GREEN_LED, LOW);
    setneopixel(0, 0, 0);

    // Blinking RED LED 3 times
    Target_hit();

    // Send message via ESP-NOW
    Serial.println("*** Sending the Score now ****");
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
    
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