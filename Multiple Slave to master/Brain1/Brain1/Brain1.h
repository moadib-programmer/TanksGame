/************** Includes *****************/
#include <esp_now.h>
#include <WiFi.h>
#include <SPI.h>
#include <Wire.h>
#include <nRF24L01.h>
#include <esp_wifi.h>
#include <RF24.h>

/************** Macros *****************/
/* ID of this Brain */
#define ID          1
#define TANK_ID     1   //1 for brain/tank 1
#define TEAM_ID     1   //1 for Team A or whatever

#define VOLT_PIN    (34U)
#define RED_LED     (33U)
#define GREEN_LED    32
#define MAX_TARGETS  5

/*********** Function prototypes **********/
void SendNextionCommand(String object, String msg);