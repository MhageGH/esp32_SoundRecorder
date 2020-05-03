#include "Arduino.h"
#include <FS.h>
#include "Wav.h"
#include "I2S.h"
#include "SPIFFS.h"
#define FORMAT_SPIFFS_IF_FAILED true

//comment the first line and uncomment the second if you use MAX9814
//#define I2S_MODE I2S_MODE_RX
#define I2S_MODE I2S_MODE_ADC_BUILT_IN

const int record_time = 5;  // second
const char filename[] = "/sound.wav";

const int headerSize = 44;
const int waveDataSize = record_time * 88000;
const int numCommunicationData = 8000;
const int numPartWavData = numCommunicationData/4;
byte header[headerSize];
char communicationData[numCommunicationData];
char partWavData[numPartWavData];
File file;

//https://github.com/espressif/arduino-esp32/blob/master/libraries/SPIFFS/examples/SPIFFS_Test/SPIFFS_Test.ino
void listDir(fs::FS& fs, const char* dirname, uint8_t levels) {
  // Serial.printf("Listing directory: %s\r\n", dirname);

  File root = fs.open(dirname);
  if (!root) {
    // Serial.println("- failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    // Serial.println(" - not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if (levels) {
        listDir(fs, file.name(), levels - 1);
      }
    } else {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("\tSIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}

void setup() {
  Serial.begin(115200);
  if (!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)) {
    Serial.println("SPIFFS Mount Failed");
    return;
  }
  // https://gitter.im/espressif/arduino-esp32?at=5dffa2c7260751301cf1455b
  // do NOT remove the following line  SPIFFS.format();
  CreateWavHeader(header, waveDataSize);
  SPIFFS.remove(filename);
  file = SPIFFS.open(filename, FILE_WRITE);
  if (!file) return;
  Serial.println("start recording");
  file.write(header, headerSize);
  I2S_Init(I2S_MODE, I2S_BITS_PER_SAMPLE_32BIT);
  for (int j = 0; j < waveDataSize/numPartWavData; ++j) {
    I2S_Read(communicationData, numCommunicationData);
    for (int i = 0; i < numCommunicationData/8; ++i) {
      partWavData[2*i] = communicationData[8*i + 2];
      partWavData[2*i + 1] = communicationData[8*i + 3];
    }
    file.write((const byte*)partWavData, numPartWavData);
  }
  file.close();
  Serial.println("finish recording");
  Serial.println("test SPIFFS recording file");
  listDir(SPIFFS, "/", 0);
  
}

void loop() {
}
