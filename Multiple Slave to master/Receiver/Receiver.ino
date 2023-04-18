#include <esp_now.h>
#include <WiFi.h>

// Structure example to receive data
// Must match the sender structure
typedef struct struct_message {
  int id;
  int flag;
  int Score;
}struct_message;

// Create a struct_message called myData
struct_message myData;

// Create a structure to hold the readings from each board
struct_message board1;
struct_message board2;
struct_message board3;
struct_message board4;

// Create an array with all the structures
struct_message boardsStruct[4] = {board1, board2, board3, board4};

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len) {
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
  Serial.printf("Score value: %d \n", boardsStruct[myData.id-1].Score);
  Serial.println();
}
 
void setup() {
  //Initialize Serial Monitor
  Serial.begin(115200);
  
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
}
 
void loop() {
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
