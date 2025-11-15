/*+************* Includes *****************/
#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>

/*+*************** Macros *****************/
#define VOLT_PIN             34
#define RED_LED              27
#define BUTTON_PIN           21
#define GREEN_LED            26
#define GREEN_ON_TIME_MS     2000

#define RED_LIGHT_PWR1       17
#define GREEN_LIGHT_PWR4     19
#define YELLOW_LIGHT_PWR2    18
#define FAN_CTRL_PIN_PWR3    5
#define OTA_CTRL_PIN         25
#define TMP36_PIN            32       // Analog input pin
#define TMP36_EN_PIN         33    // Enable pin (active low)

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

float measureTemp(void);