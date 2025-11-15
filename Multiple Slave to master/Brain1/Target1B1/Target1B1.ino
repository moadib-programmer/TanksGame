/*+************* Includes *****************/
#include "target.h"

/*+*************** Globals ******************/
volatile float Voltage = 0U;
// Set your new MAC Address
uint8_t newMACAddress[] = {0x32, 0xAE, 0xA4, 0x07, 0xB1, 0x01};

uint8_t broadcastAddress[] = {0x42, 0xAE, 0xA4, 0x07, 0x0D, 0x01};

uint8_t targetLedAddress[] = {0x32, 0xAE, 0xA4, 0x07, 0xA1, 0x01};

/******* Structure to send data to the brain ********/
typedef struct StructureOfTargets 
{
  int id;
  bool soft_hard_flag; // soft -> 0, hard -> 1.
} StructureOfTargets;


StructureOfTargets myData;

int healthFrmBrain = 0;

esp_now_peer_info_t peerInfo;

/*+*************** Function definations ******************/

/**
 * @brief OnDataSent: callback funcntion when data is sent via ESPNow
 * 
 * @param mac_addr: mac address of the receiver
 * @param status: Status of the sent message
 * 
 * @return void
 * 
*/
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) 
{
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

/**
 * @brief OnDataRecv: callback funcntion when data is received via ESPNow
 * 
 * @param mac: mac address of the sender
 * @param incomingData: pointer to the variable of incoming data from the sender
 * @param len: length of the received message
 * 
 * @return void
 * 
*/
void OnDataRecv(const uint8_t *mac_addr, const uint8_t *incomingData, int data_len)
{
  memcpy(&healthFrmBrain, incomingData, sizeof(healthFrmBrain));

  if(healthFrmBrain == -129)
  {
    Serial.printf("Reset Command Received");

    ESP.restart();
  }
  
  Serial.print("health Received from the brain: ");
  Serial.println(healthFrmBrain);

  Serial.println("Sending the command to the Target.");

  /* Setting Neopixel depending on healthFrmBrain */
  if( (healthFrmBrain >= 70) and (healthFrmBrain <= 300) )
  {
    /* Setting neopixel to green */
    Serial.println("\n Sending Green color switching \n");
    digitalWrite(RED_LIGHT_PWR1, 0);
    digitalWrite(GREEN_LIGHT_PWR4, 1);
    digitalWrite(YELLOW_LIGHT_PWR2, 0);
  }
  else if ( (healthFrmBrain >= 40) and (healthFrmBrain <= 69) )
  {
    /* Setting neopixel to yellow */
    Serial.println("\n Sending yellow color switching \n");
    digitalWrite(RED_LIGHT_PWR1, 0);
    digitalWrite(GREEN_LIGHT_PWR4, 0);
    digitalWrite(YELLOW_LIGHT_PWR2, 1);
  }
  else if ( (healthFrmBrain >= 0) and (healthFrmBrain <= 39) )
  {
    /* Setting neopixel to Red */
    Serial.println("\n Sending yellow Red switching \n");
    digitalWrite(RED_LIGHT_PWR1, 1);
    digitalWrite(GREEN_LIGHT_PWR4, 0);
    digitalWrite(YELLOW_LIGHT_PWR2, 0);
  }

  Serial.println();
  delay(10);
}

float measureTemp(void)
{
  int adcValue = analogRead(TMP36_PIN);

  // Convert to voltage (ESP32 ADC = 12-bit, 3.3V reference)
  float voltage = (adcValue / 4095.0) * 3.3;

  // TMP36 output is 10 mV/°C with 500 mV offset at 0°C
  float temperatureC = ((voltage - 0.5) * 100.0) + 6.0;

  // Serial.print(temperatureC, 1);
  // Serial.println(" °C");

  return temperatureC;
}


/*+*************** Initializations ******************/
void setup() 
{
  Serial.begin(9600);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  /* Assigning a custom MAC address */
  if( esp_wifi_set_mac(WIFI_IF_STA, &newMACAddress[0]) == ESP_OK)
  {
    Serial.println(" ***** New mac address set ********");
  }
  else
  {
    Serial.println("******* MAC ADDRESS NOT SET *********");
  }
  
  delay(100);

  /* Init ESP-NOW */
  if (esp_now_init() != ESP_OK) 
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(OTA_CTRL_PIN, INPUT);
  pinMode(FAN_CTRL_PIN_PWR3, OUTPUT);
  pinMode(RED_LIGHT_PWR1, OUTPUT);
  pinMode(GREEN_LIGHT_PWR4, OUTPUT);
  pinMode(YELLOW_LIGHT_PWR2, OUTPUT);
  pinMode(VOLT_PIN, INPUT);

  pinMode(TMP36_EN_PIN, OUTPUT);
  digitalWrite(TMP36_EN_PIN, HIGH);

  /* Turning Green LED ON for 2 seconds */
  digitalWrite(GREEN_LED, 0);
  digitalWrite(RED_LED, 0);

  // digitalWrite(RED_LIGHT_PWR1, 1);
  // digitalWrite(GREEN_LIGHT_PWR4, 1);
  // digitalWrite(YELLOW_LIGHT_PWR2, 1);
  // digitalWrite(FAN_CTRL_PIN_PWR3, 1);

  delay(GREEN_ON_TIME_MS);
  
  digitalWrite(GREEN_LED, 1);
  digitalWrite(RED_LED, 1);

  Serial.println("Turning Lights On");
  digitalWrite(RED_LIGHT_PWR1, 1);
  digitalWrite(GREEN_LIGHT_PWR4, 1);
  digitalWrite(YELLOW_LIGHT_PWR2, 1);
  digitalWrite(FAN_CTRL_PIN_PWR3, 1);
  
  delay(3000);
Serial.println("Turning Lights OFF");
  digitalWrite(RED_LIGHT_PWR1, 0);
  digitalWrite(GREEN_LIGHT_PWR4, 0);
  digitalWrite(YELLOW_LIGHT_PWR2, 0);
  digitalWrite(FAN_CTRL_PIN_PWR3, 0);

  /* Register Datasend and Datarcv callback functions */
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);
  
  /* Register peer */
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    Serial.println("Failed to add peer");
    return;
  }

/**
 * This is the funciton which is needed to send the status of the lED
 * The function takes the 2 arguments which is the one and the one 
 * argument is the led number, which is actually its color and later it calls for the 
 * delay at which the led blinks
*/
    /* Register peer */
  memcpy(peerInfo.peer_addr, targetLedAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    Serial.println("Failed to add peer");
    return;
  }

  Serial.println("******* Initiating the Hit detection ********");
  pinMode(BUTTON_PIN, INPUT_PULLDOWN);
}


void targetHitCallback()
{

  Serial.println("***** TARGET HIT ****** ");
  digitalWrite(RED_LIGHT_PWR1, 0);
  digitalWrite(GREEN_LIGHT_PWR4, 0);
  digitalWrite(YELLOW_LIGHT_PWR2, 0);

  /* Flashing the Red LED */
  for (int i = 0; i <= 2; i++)
  {
    digitalWrite(RED_LIGHT_PWR1, 1);
    delay(500);
    digitalWrite(RED_LIGHT_PWR1, 0);
    delay(500);
  }

  myData.soft_hard_flag = 1; // default hard

  /* Send message via ESP-NOW */
  Serial.println("*** Sending the Score now ****");
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
  
  if (result == ESP_OK) 
  {
    Serial.println("Score Sent to the Brain");
    Serial.println(" ");
    Serial.println("Awaiting Score results ");
  }
}

void loop() 
{
  // TARGET_ID 1 for target 1
  myData.id = TARGET_ID;

  /****** When target is hit ******/
  if(digitalRead(BUTTON_PIN) == 1) 
  {
    /* Callback function for HIT */
    targetHitCallback();
  }
  if(digitalRead(OTA_CTRL_PIN) == 0)
  {
    Serial.println("OTA button Psed");
    delay(300);
  }

  if(measureTemp() > 50)
  {
    Serial.println("Turning on the FAN");
  }

  delay(10);
}