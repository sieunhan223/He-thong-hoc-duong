#include <Adafruit_ILI9341.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncTCP.h>
#include "LittleFS.h";
#include "LOGO_congty.h";
#include <Adafruit_GFX.h>
#include "Fonts/FreeSansBold12pt7b.h";
#include "Fonts/FreeSansBold9pt7b.h";

#define RST_PIN 3
#define SS_PIN 4
#define ken 8
#define TFT_CS 1
#define TFT_DC 2

int col[8];

#define BLACK 0x0000
#define RED 0xF800
#define WHITE 0xFFFF

MFRC522 mfrc522(SS_PIN, RST_PIN);             // Create MFRC522
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC); // tft
MFRC522::MIFARE_Key key;

// Search for parameter in HTTP POST request
const char *PARAM_INPUT_1 = "ssid";
const char *PARAM_INPUT_2 = "pass";
const char *PARAM_INPUT_3 = "terminal_id";
const char *PARAM_INPUT_4 = "URL_server";
const char *PARAM_INPUT_5 = "AP_name";
const char *PARAM_INPUT_6 = "AP_pass";
// Variables to save values from HTML form
String ssid, pass;
String terminal_id;
String URL_server;
String AP_name, AP_pass;
boolean restart = false;
bool val = true;
// File paths to save input values permanently
const char *ssidPath = "/ssid.txt";
const char *passPath = "/pass.txt";
const char *terminal_idPath = "/terminal_id.txt";
const char *URL_serverPath = "/URL_server.txt";
const char *AP_namePath = "/AP_name.txt";
const char *AP_passPath = "/AP_pass.txt";

// http://192.168.4.1/

AsyncWebServer server(80);

IPAddress localIP;

//*****************************************************************************************//

// Hàm cho tft
void printText(String text, uint16_t color, int x, int y, int textSize)
{
  tft.setCursor(x, y);
  tft.setTextColor(color);
  tft.setTextSize(textSize);
  tft.print(text);
}

// hàm đọc:
void readBlock(byte block, byte len, byte *content, MFRC522::StatusCode &status)
{
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid)); // line 834
  if (status != MFRC522::STATUS_OK)
  {
    Serial.print(F("Authentication failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    val = false;
  }

  status = mfrc522.MIFARE_Read(block, content, &len);
  if (status != MFRC522::STATUS_OK)
  {
    Serial.print(F("Reading failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    val = false;
  }
}

// Read File from LittleFS
String readFile(fs::FS &fs, const char *path)
{
  Serial.printf("Reading file: %s\r\n", path);

  File file = fs.open(path, "r");
  if (!file || file.isDirectory())
  {
    Serial.println("- failed to open file for reading");
    return String();
  }

  String fileContent;
  while (file.available())
  {
    fileContent = file.readStringUntil('\n');
    break;
  }
  file.close();
  return fileContent;
}
// Write file to LittleFS
void writeFile(fs::FS &fs, const char *path, const char *message)
{
  Serial.printf("Writing file: %s\r\n", path);

  File file = fs.open(path, "w");
  if (!file)
  {
    Serial.println("- failed to open file for writing");
    return;
  }
  if (file.print(message))
  {
    Serial.println("- file written");
  }
  else
  {
    Serial.println("- frite failed");
  }
  file.close();
}

// Initialize LittleFS
void initFS()
{
  if (!LittleFS.begin())
  {
    Serial.println("An error has occurred while mounting LittleFS");
  }
  else
  {
    Serial.println("LittleFS mounted successfully");
  }
}

// Initialize WiFi
bool initWiFi()
{
  if (ssid == "")
  {
    Serial.println("Undefined SSID or IP address.");
    return false;
  }

  WiFi.begin(ssid.c_str(), pass.c_str());

  Serial.println("Connecting to WiFi...");
  int dem = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    for (int i = 8; i > 0; i--)
    {
      tft.fillCircle(150 + 15 * (cos(-(i + 0) * PI / 4)), 160 + 15 * (sin(-(i + 0) * PI / 4)), 3, col[0]);
      delay(10);
      tft.fillCircle(150 + 15 * (cos(-(i + 1) * PI / 4)), 160 + 15 * (sin(-(i + 1) * PI / 4)), 3, col[1]);
      delay(10);
      tft.fillCircle(150 + 15 * (cos(-(i + 2) * PI / 4)), 160 + 15 * (sin(-(i + 2) * PI / 4)), 3, col[2]);
      delay(10);
      tft.fillCircle(150 + 15 * (cos(-(i + 3) * PI / 4)), 160 + 15 * (sin(-(i + 3) * PI / 4)), 3, col[3]);
      delay(10);
      tft.fillCircle(150 + 15 * (cos(-(i + 4) * PI / 4)), 160 + 15 * (sin(-(i + 4) * PI / 4)), 3, col[4]);
      delay(10);
      tft.fillCircle(150 + 15 * (cos(-(i + 5) * PI / 4)), 160 + 15 * (sin(-(i + 5) * PI / 4)), 3, col[5]);
      delay(10);
      tft.fillCircle(150 + 15 * (cos(-(i + 6) * PI / 4)), 160 + 15 * (sin(-(i + 6) * PI / 4)), 3, col[6]);
      delay(10);
      tft.fillCircle(150 + 15 * (cos(-(i + 7) * PI / 4)), 160 + 15 * (sin(-(i + 7) * PI / 4)), 3, col[7]);
      delay(10);
    }
    if (dem > 50)
    {
      break;
    }
    Serial.print(".");
    dem = dem + 1;
  }
  tft.fillScreen(BLACK);
  tft.drawRGBBitmap(37, 18, LOGO_congty_den, 245, 200);
  
  digitalWrite(ken, HIGH);
  delay(35);
  digitalWrite(ken, LOW);
  delay(100);
  digitalWrite(ken, HIGH);
  delay(35);
  digitalWrite(ken, LOW);  
  
  Serial.print("\nConnected!, IP address: ");
  Serial.println(WiFi.localIP());
  return true;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------------//
void setup()
{
  Serial.begin(112500); // Initialize serial communications with the PC
  initFS();       // InitFS6b6b6b
  SPI.begin();      // Init SPI bus
  mfrc522.PCD_Init();   // Init MFRC522 card

  pinMode(ken, OUTPUT); // Init ken
  tft.begin();
  tft.fillScreen(WHITE); // Thiet lap mau nen LCD
  tft.setRotation(1);
  tft.drawRGBBitmap(60, 100, loading, 190, 29);
  tft.setFont(&FreeSansBold9pt7b);
  col[0] = tft.color565(35, 35, 35);
  col[1] = tft.color565(53, 53, 53);
  col[2] = tft.color565(71, 71, 71);
  col[3] = tft.color565(89, 89, 89);
  col[4] = tft.color565(107, 107, 107);
  col[5] = tft.color565(124, 124, 124);
  col[6] = tft.color565(142, 142, 142);
  col[7] = tft.color565(160, 160, 160);
  // đọc và lưa giá trị ở tệp
  ssid = readFile(LittleFS, ssidPath);
  pass = readFile(LittleFS, passPath);
  terminal_id = readFile(LittleFS, terminal_idPath);
  URL_server = readFile(LittleFS, URL_serverPath);
  AP_name = readFile(LittleFS, AP_namePath);
  AP_pass = readFile(LittleFS, AP_passPath);

  Serial.println(ssid);
  Serial.println(pass);
  Serial.println(terminal_id);
  Serial.println(URL_server);
  Serial.println(AP_name);
  Serial.println(AP_pass);

  initWiFi();

  // Kết nối web esp
  // Connect to Wi-Fi network with SSID and password
  Serial.println("Setting AP (Access Point)");
  Serial.print("Configuring access point...");
  WiFi.softAP(AP_name, AP_pass);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  // Web Server Root URL
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
        { request->send(LittleFS, "/device_config.html", "text/html"); });

  server.serveStatic("/", LittleFS, "/");

  server.on("/", HTTP_POST, [](AsyncWebServerRequest *request)
        {
      int params = request->params();
      for(int i=0;i<params;i++){
        AsyncWebParameter* p = request->getParam(i);
        if(p->isPost()){
          // HTTP POST ssid value
          if (p->name() == PARAM_INPUT_1) {
            ssid = p->value().c_str();
            Serial.print("SSID set to: ");
            Serial.println(ssid);
            // Write file to save value
            writeFile(LittleFS, ssidPath, ssid.c_str());
          }
          // HTTP POST pass value
          if (p->name() == PARAM_INPUT_2) {
            pass = p->value().c_str();
            Serial.print("Password set to: ");
            Serial.println(pass);
            // Write file to save value
            writeFile(LittleFS, passPath, pass.c_str());
          }
          // HTTP POST terminal_id value
          if (p->name() == PARAM_INPUT_3) {
            terminal_id = p->value().c_str();
            Serial.print("Terminal ID set to: ");
            Serial.println(terminal_id);
            // Write file to save value
            writeFile(LittleFS, terminal_idPath, terminal_id.c_str());
          }
          // HTTP POST server value
          if (p->name() == PARAM_INPUT_4) {
            URL_server = p->value().c_str();
            Serial.print("server address set to: ");
            Serial.println(URL_server);
            // Write file to save value
            writeFile(LittleFS, URL_serverPath, URL_server.c_str());
          }
          // HTTP POST AP name value
          if (p->name() == PARAM_INPUT_5) {
            AP_name = p->value().c_str();
            Serial.print("AP name set to: ");
            Serial.println(AP_name);
            // Write file to save value
            writeFile(LittleFS, AP_namePath, AP_name.c_str());
          }
          // HTTP POST AP pass value
          if (p->name() == PARAM_INPUT_6) {
            AP_pass = p->value().c_str();
            Serial.print("AP pass set to: ");
            Serial.println(AP_pass);
            // Write file to save value
            writeFile(LittleFS, AP_passPath, AP_pass.c_str());
          }
          //Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
        }
      }
      restart = true;
      request->send(200, "text/plain", "Done. ESP will restart, connect to your router and go to IP address: http://192.168.4.1/"); });

  server.begin();
}
//*****************************************************************************************//
void loop()
{
  if (restart)
  {
    delay(5000);
    ESP.restart();
  }

  // khai báo các biến
  byte CardUID[18];
  String s_CardUID = "";
  String TerminalID = terminal_id;
  byte SSCID[18];
  String s_SSCID = "";
  byte Name1[18];
  String s_Name1 = "";
  byte Name2[18];
  String s_Name2 = "";

  // Prepare key - all keys are set to FFFFFFFFFFFFh at chip delivery from the factory.

  for (byte i = 0; i < 6; i++)
    key.keyByte[i] = 0xFF;

  // some variables we need
  byte block;
  byte len;
  String temp;
  MFRC522::StatusCode status;

  //-------------------------------------------

  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if (!mfrc522.PICC_IsNewCardPresent())
    return;

  // Select one of the cards
  if (!mfrc522.PICC_ReadCardSerial())
    return;

  Serial.println(F("**Card Detected:**"));

  // Lưu UID Card vào biến
  for (byte i = 0; i < 4; i++)
  {
    CardUID[i] = mfrc522.uid.uidByte[i];
    if (CardUID[i] < 0x10)
      s_CardUID += "0" + String(CardUID[i]);
    else
      s_CardUID += String(CardUID[i]);
  }

  mfrc522.PICC_DumpDetailsToSerial(&(mfrc522.uid)); // dump some details about the card

  // mfrc522.PICC_DumpToSerial(&(mfrc522.uid));      //uncomment this to see all blocks in hex

  //-------------------------------------------------------------------------------------------------*
  // Xử lý biến SSCID
  Serial.print("block4: ");
  block = 4;
  len = 18;

  readBlock(block, len, SSCID, status);

  temp = String((char *)SSCID);
  for (int i = 0; i < 16; i++)
  {
    s_SSCID += temp[i];
  }
  // PRINT SSID
  for (uint8_t i = 0; i < 16; i++)
  {
    if (SSCID[i] != 32)
    {
      Serial.write(SSCID[i]);
    }
  }
  Serial.println("");

  //-------------------------------------------------------------------------------------------------*
  // Xử lý biến Name1
  Serial.print("block5: ");
  block = 5;
  len = 18;
  readBlock(block, len, Name1, status);

  temp = String((char *)Name1);

  for (int i = 0; i < 16; i++)
  {
    if (Name1[i] != 0x0)
    {
      s_Name1 += temp[i];
    }
  }
  // print Name1
  for (uint8_t i = 0; i < 16; i++)
  {
    Serial.write(Name1[i]);
  }
  Serial.println("");
  //-------------------------------------------------------------------------------------------------*
  // Xử lý biến Name2
  Serial.print("block6: ");
  block = 6;
  len = 18;

  readBlock(block, len, Name2, status);

  temp = String((char *)Name2);

  for (int i = 0; i < 16; i++)
  {
    if (Name2[i] != 0x0)
    {
      s_Name2 += temp[i];
    }
  }
  // PRINT Name2
  for (uint8_t i = 0; i < 16; i++)
  {
    if (Name2[i] != 0x0)
    {
      Serial.write(Name2[i]);
    }
  }
  Serial.println("");
  //-------------------------------------------------------------------------------------------------*
  Serial.println(F("\n**End Reading**\n"));
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();

  //-------------------------------------------------------------------------------------------------*
  // đọc thẻ:
  tft.fillScreen(tft.color565(35, 27, 18));
  String s_Name = s_Name1 + s_Name2;
  //  printText("NAME:", WHITE, 3, 30, 1);
  printText(s_Name, tft.color565(245, 108, 87), 3, 50, 1);
  Serial.println("");
  //  // In lớp lên LCD
  printText("CLASS:", tft.color565(247, 118, 4), 3, 71, 1);
  printText("A1", tft.color565(184, 210, 11), 78, 71, 1);
  //
  //  // in ID lên LCD
  printText("ID: ", tft.color565(247, 118, 4), 3, 92, 1);
  printText(s_SSCID, tft.color565(184, 210, 11), 78, 92, 1);

  // bị lỗi:
  if (!val)
  {
    tft.fillScreen(BLACK);
    tft.setFont(&FreeSansBold12pt7b);
    tft.drawRGBBitmap(50, 40, error, 220, 137);
    printText("PLEASE TRY AGAIN!", RED, 39, 165, 1);
    tft.setFont(&FreeSansBold9pt7b);
  }
  // kèn
  digitalWrite(ken, HIGH);
  delay(200);
  digitalWrite(ken, LOW);

  //-------------------------------------------------------------------------------------------------*
  // POST lên server
  if (val)
  {
    StaticJsonDocument<200> doc; // khai báo chỗ chứa dữ liệu
    String jsonData;
    doc["TerminalID"] = TerminalID;
    doc["CardUID"] = s_CardUID;
    doc["SSCID"] = s_SSCID;
    doc["Name"] = s_Name;
    serializeJsonPretty(doc, jsonData);
    WiFiClient client;
    HTTPClient http;

    Serial.print("[HTTP] begin...\n");
    // configure traged server and url
    http.begin(client, URL_server); // HTTP
    http.addHeader("Content-Type", "application/json");

    Serial.print("[HTTP] POST...\n");
    // start connection and send HTTP header and body
    int httpCode = http.POST(jsonData);

    // httpCode will be negative on err or
    if (httpCode > 0)
    {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] POST... code: %d\n", httpCode);

      // file found at server
      if (httpCode == HTTP_CODE_OK)
      {
        const String &payload = http.getString();
        Serial.println("received payload:\n");
        Serial.println(payload);

        //-------------------------------------------------------------------------------------------------*
        // GET
        StaticJsonDocument<200> doc2; // tạo doc chứa data
        deserializeJson(doc2, payload); // fix payload(dữ liệu từ server trả về) với doc2
        JsonObject obj = doc2.as<JsonObject>();
        String data = obj["Data"];   // lấy dữ liệu tên Data
        deserializeJson(doc2, data); // Vì Data là 1 JSON cho nên phải fix data thêm 1 lần nữa
        JsonObject obj2 = doc2.as<JsonObject>();
        const char *RealTime = obj2["Time"]; // lấy real time
        // in RealTime lên LCD
        String s_RealTime;
        for (int i = 0; i < 19; i++)
        {
          if (RealTime[i] == 'T')
          {
            s_RealTime += " ";
          }
          else
          {
            s_RealTime += String(RealTime[i]);
          }
        }
        printText("TIME:", tft.color565(247, 118, 4), 3, 113, 1);
        printText(s_RealTime, tft.color565(184, 210, 11), 78, 113, 1);}
      else
        Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();    
  }
  else
    val = true;
  delay(50);
//-------------------------------------------
}
