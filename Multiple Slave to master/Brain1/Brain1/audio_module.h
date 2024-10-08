// will play many audio file formats, mp3,aac,flac etc.
// See github page : https://github.com/schreibfaul1/ESP32-audioI2S

/*********** Includes *********/
#include "Arduino.h"
#include "Audio.h"
#include "SD.h"
#include "FS.h"
#include <SPI.h>
/*********** MACRO *************/

//hspi pinouts
#define SD_CS 15
#define SPI_MOSI 13
#define SPI_MISO 22
#define SPI_SCK 14

//speakers pinouts
#define I2S_DOUT      25
#define I2S_BCLK      27    // I2S
#define I2S_LRC       26

 
/*********** Data Types *************/
typedef enum
{
  AUDIO_E_OK,
  AUDIO_INIT_FAIL,
  AUDIO_SET_VOLUME_FAIL,
  AUDIO_FILE_NAME_FAIL
}AUDIO_ERR_TYPE;

/******* Function Prototypes *********/
void audioInit(void);