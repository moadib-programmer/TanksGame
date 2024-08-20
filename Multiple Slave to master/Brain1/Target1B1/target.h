/*+************* Includes *****************/
#include <esp_now.h>
#include <WiFi.h>
#include <Adafruit_NeoPixel.h>
#include <esp_wifi.h>

/*+*************** Macros ******************/
#define PIN                15
#define NUM_OF_LEDS        9
#define VOLT_PIN           34
#define RED_LED            33
#define BUTTON_PIN         21
#define NUMBER_OF_BLINKS   3
#define GREEN_LED          35
#define GREEN_ON_TIME_MS   2000
#define RED_FLASH_TIME_MS  600
#define DELAY_HIT_FLASH_MS 2000 

/**
 * @brief HIT Brief
 * If 
 * id = 1, front hit
 * id = 2, side hit
 * id = 3, back hit
 */
#define TARGET_ID     1
#define MIN_SCORE     5
#define MAX_SCORE     10