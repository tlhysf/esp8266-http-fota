#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MCP23017.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>

int led3Pin = 7;
int led2Pin = 6;
int led1Pin = 5;
Adafruit_MCP23017 mcp;

const String FirmwareVer = {"1.0"};
#define URL_fw_Version  "https://raw.githubusercontent.com/tlhysf/esp8266-http-fota/master/firmware/version.txt"
#define URL_fw_Bin      "https://raw.githubusercontent.com/tlhysf/esp8266-http-fota/master/firmware/firmware.ino.nodemcu.bin"

#define URL_fw_Version_Fingerprint "70 94 DE DD E6 C4 69 48 3A 92 70 A1 48 56 78 2D 18 64 E0 B7"
#define URL_fw_Bin_Fingerprint "70 94 DE DD E6 C4 69 48 3A 92 70 A1 48 56 78 2D 18 64 E0 B7"

HTTPClient http;

const char* ssid = "HOME-PTCL";
const char* password = "broadband#672";

void FirmwareUpdate()
{
  http.begin(URL_fw_Version, URL_fw_Version_Fingerprint);
  delay(100);

  // get data from version file
  int httpCode = http.GET();
  delay(100);
  String payload;

  // if version received
  if (httpCode == HTTP_CODE_OK)
  {
    // save received version
    payload = http.getString();
    Serial.println(payload);
  }
  else
  {
    Serial.print("Error in downloading version file: ");
    Serial.println(httpCode);
  }

  http.end();

  // if version received
  if (httpCode == HTTP_CODE_OK)         
  {
    if (payload.equals(FirmwareVer))
    {
      Serial.println("Device already on latest firmware version.");
    }
    else
    {
      Serial.println("New firmware detected.");
      WiFiClient client;

      // The line below is optional. It can be used to blink the LED on the board during flashing
      // The LED will be on during download of one buffer of data from the network. The LED will
      // be off during writing that buffer to flash
      // On a good connection the LED should flash regularly. On a bad connection the LED will be
      // on much longer than it will be off. Other pins than LED_BUILTIN may be used. The second
      // value is used to put the LED on. If the LED is on with HIGH, that value should be passed
      // ESPhttpUpdate.setLedPin(LED_BUILTIN, LOW);
      mcp.digitalWrite(led1Pin, 1);

      t_httpUpdate_return ret = ESPhttpUpdate.update(URL_fw_Bin, "", URL_fw_Bin_Fingerprint);

      switch (ret) {
        case HTTP_UPDATE_FAILED:
          Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
          break;

        case HTTP_UPDATE_NO_UPDATES:
          Serial.println("HTTP_UPDATE_NO_UPDATES");
          break;

        case HTTP_UPDATE_OK:
          Serial.println("HTTP_UPDATE_OK");
          break;
      }
    }
  }
}

unsigned long previousMillis = 0;
const long interval = 30000;

void repeatedCall() {
  unsigned long currentMillis = millis();
  if ((currentMillis - previousMillis) >= interval)
  {
    previousMillis = currentMillis;
    FirmwareUpdate();
  }
}

void setup()
{
  Serial.begin(115200);
  Serial.println("");
  Serial.println("Start");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print("* ");
  }
  Serial.println("Connected to WiFi");
  pinMode(LED_BUILTIN, OUTPUT);

  Wire.begin();
  mcp.begin();
  mcp.pinMode(led1Pin, OUTPUT);
  mcp.pinMode(led2Pin, OUTPUT);
  mcp.pinMode(led3Pin, OUTPUT);
  mcp.digitalWrite(led1Pin, 0);
  mcp.digitalWrite(led2Pin, 0);
  mcp.digitalWrite(led3Pin, 0);
}

void loop()
{
  mcp.digitalWrite(led2Pin, 1);
  delay(200); 
  mcp.digitalWrite(led2Pin, 0);
  delay(200); 
  
  repeatedCall();
}
