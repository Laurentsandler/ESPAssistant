/* ESP32 I2S Mic -> Groq Whisper -> Groq Chat -> OLED
  Includes web server for audio playback.
  Hardware:
  - I2S mic (INMP441): WS=4, SD=2, SCK=3
  - OLED SH1106/SSD1306 I2C: SDA=5, SCL=6 (128x64)
  - Button on pin 7 (INPUT_PULLUP)
  - Button on pin 8 (scroll)
  - Button on pin 9 (assignment)
  - Button on pin 1 (extra)
*/

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>

#include <driver/i2s.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

// --------- USER CONFIG ----------
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASS = "YOUR_WIFI_PASSWORD";
const char* GROQ_API_KEY = "YOUR_GROQ_API_KEY";

const char* TRANSCRIBE_MODEL = "whisper-large-v3";
const char* CHAT_MODEL = "llama-3.1-8b-instant";

// Supabase config - REPLACE WITH YOUR VALUES!
const char* SUPABASE_URL = "YOUR_SUPABASE_URL";
const char* SUPABASE_ANON_KEY = "YOUR_SUPABASE_ANON_KEY";
const char* SUPABASE_USER_ID = "YOUR_SUPABASE_USER_ID";
const char* SUPABASE_SERVICE_KEY = "YOUR_SUPABASE_SERVICE_KEY";

// Pin definitions
#define I2S_WS 4
#define I2S_SD 2
#define I2S_SCK 3
#define BUTTON_PIN 7
#define BUTTON_SCROLL_PIN 8
#define BUTTON_ASSIGNMENT_PIN 9
#define BUTTON_EXTRA_PIN 1

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_SDA_PIN 5
#define OLED_SCL_PIN 6
#define OLED_ADDR 0x3C

// UI layout constants
static const int UI_STATUS_H = 12;
static const int UI_BOTTOM_TAB_H = 16;
static const int UI_MARGIN = 4;

// NTP/Time configuration (set to Europe/Paris for Marseille: CET/CEST with DST)
const char* NTP_SERVER_1 = "pool.ntp.org";
const char* NTP_SERVER_2 = "time.nist.gov";
const char* TZ_INFO = "CET-1CEST,M3.5.0/2,M10.5.0/3";

// Create display object for SH1106
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, OLED_SCL_PIN, OLED_SDA_PIN);

// I2S configuration
#define I2S_PORT I2S_NUM_0
#define SAMPLE_RATE 16000
#define BITS_PER_SAMPLE I2S_BITS_PER_SAMPLE_32BIT
#define DMA_BUF_COUNT 8
#define DMA_BUF_LEN 1024

// Recording buffer
const int MAX_RECORD_TIME_MS = 10000;
const int MAX_RECORD_SAMPLES = (SAMPLE_RATE * MAX_RECORD_TIME_MS) / 1000;
int16_t* audioBuffer = nullptr;
int audioSampleCount = 0;

// Web server
WebServer server(80);

// App states
enum AppMode {
  MODE_MENU,
  MODE_AI,
  MODE_WEATHER,
  MODE_ASSIGNMENTS,
  MODE_GAME,
  MODE_SCREENSAVER
};

AppMode currentMode = MODE_SCREENSAVER;
int menuSelection = 0;
const int MENU_ITEMS = 5;
const char* menuLabels[] = {"AI", "Weather", "Assign", "Game", "Clock"};

// AI response storage
String aiResponse = "";
int responseScrollOffset = 0;

// Weather data
String weatherLocation = "";
String weatherTemp = "";
String weatherCondition = "";
unsigned long lastWeatherUpdate = 0;

// Game state
unsigned long gameStartTime = 0;
unsigned long gameReactionTime = 0;
bool gameWaitingForPress = false;

// Assignment recording
bool recordingAssignment = false;

// Button debouncing
unsigned long lastButtonPress = 0;
unsigned long lastScrollPress = 0;
unsigned long lastAssignmentPress = 0;
const int DEBOUNCE_DELAY = 50;
const int LONG_PRESS_DELAY = 500;

// Screensaver
unsigned long lastActivityTime = 0;
const unsigned long SCREENSAVER_TIMEOUT = 60000; // 1 minute

// Function declarations
void setupI2S();
void setupWiFi();
void setupWebServer();
void setupDisplay();
void setupNTP();
void recordAudio();
String transcribeAudio();
String getChatResponse(String userMessage);
void updateWeather();
void fetchWeatherData();
void startReactionGame();
void handleReactionPress();
void saveAssignment(String assignmentText);
void drawMenu();
void drawAIMode();
void drawWeatherMode();
void drawAssignmentMode();
void drawGameMode();
void drawScreensaver();
void drawStatusBar();
void checkButtons();
void returnToMenu();
String urlEncode(String str);
String createMultipartBoundary();

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("ESP32 AI Assistant Starting...");

  // Initialize buttons
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUTTON_SCROLL_PIN, INPUT_PULLUP);
  pinMode(BUTTON_ASSIGNMENT_PIN, INPUT_PULLUP);
  pinMode(BUTTON_EXTRA_PIN, INPUT_PULLUP);

  // Initialize display
  setupDisplay();
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tr);
  u8g2.drawStr(0, 20, "Initializing...");
  u8g2.sendBuffer();

  // Allocate audio buffer
  audioBuffer = (int16_t*)malloc(MAX_RECORD_SAMPLES * sizeof(int16_t));
  if (!audioBuffer) {
    Serial.println("Failed to allocate audio buffer!");
    u8g2.clearBuffer();
    u8g2.drawStr(0, 20, "Memory Error!");
    u8g2.sendBuffer();
    while (1) delay(1000);
  }

  // Setup I2S
  setupI2S();

  // Connect to WiFi
  setupWiFi();

  // Setup NTP
  setupNTP();

  // Setup web server
  setupWebServer();

  // Start in screensaver mode
  currentMode = MODE_SCREENSAVER;
  lastActivityTime = millis();

  Serial.println("Setup complete!");
}

void loop() {
  server.handleClient();
  checkButtons();

  // Auto-enter screensaver after timeout
  if (currentMode != MODE_SCREENSAVER && (millis() - lastActivityTime > SCREENSAVER_TIMEOUT)) {
    currentMode = MODE_SCREENSAVER;
  }

  // Draw current mode
  u8g2.clearBuffer();
  
  switch (currentMode) {
    case MODE_MENU:
      drawMenu();
      break;
    case MODE_AI:
      drawAIMode();
      break;
    case MODE_WEATHER:
      drawWeatherMode();
      break;
    case MODE_ASSIGNMENTS:
      drawAssignmentMode();
      break;
    case MODE_GAME:
      drawGameMode();
      break;
    case MODE_SCREENSAVER:
      drawScreensaver();
      break;
  }

  u8g2.sendBuffer();
  delay(10);
}

void setupDisplay() {
  Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN);
  u8g2.begin();
  u8g2.setContrast(255);
}

void setupI2S() {
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = BITS_PER_SAMPLE,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = DMA_BUF_COUNT,
    .dma_buf_len = DMA_BUF_LEN,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0
  };

  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_SCK,
    .ws_io_num = I2S_WS,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = I2S_SD
  };

  i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_PORT, &pin_config);
  Serial.println("I2S initialized");
}

void setupWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Connecting to WiFi");
  
  int attempt = 0;
  while (WiFi.status() != WL_CONNECTED && attempt < 20) {
    delay(500);
    Serial.print(".");
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.drawStr(0, 20, "Connecting WiFi");
    String dots = "";
    for (int i = 0; i < (attempt % 4); i++) dots += ".";
    u8g2.drawStr(0, 35, dots.c_str());
    u8g2.sendBuffer();
    attempt++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nWiFi connection failed!");
  }
}

void setupNTP() {
  configTime(0, 0, NTP_SERVER_1, NTP_SERVER_2);
  setenv("TZ", TZ_INFO, 1);
  tzset();
  Serial.println("NTP configured");
}

void setupWebServer() {
  // mDNS responder
  if (MDNS.begin("esp32")) {
    Serial.println("mDNS responder started: esp32.local");
  }

  server.on("/", HTTP_GET, []() {
    String html = "<html><body><h1>ESP32 AI Assistant</h1>";
    html += "<p>IP: " + WiFi.localIP().toString() + "</p>";
    html += "<p><a href='/audio.wav'>Download last recording</a></p>";
    html += "</body></html>";
    server.send(200, "text/html", html);
  });

  server.on("/audio.wav", HTTP_GET, []() {
    if (audioSampleCount == 0) {
      server.send(404, "text/plain", "No recording available");
      return;
    }

    // Create WAV header
    uint32_t dataSize = audioSampleCount * 2;
    uint32_t fileSize = dataSize + 36;
    uint8_t wavHeader[44];
    
    memcpy(wavHeader, "RIFF", 4);
    *(uint32_t*)(wavHeader + 4) = fileSize;
    memcpy(wavHeader + 8, "WAVE", 4);
    memcpy(wavHeader + 12, "fmt ", 4);
    *(uint32_t*)(wavHeader + 16) = 16;
    *(uint16_t*)(wavHeader + 20) = 1;
    *(uint16_t*)(wavHeader + 22) = 1;
    *(uint32_t*)(wavHeader + 24) = SAMPLE_RATE;
    *(uint32_t*)(wavHeader + 28) = SAMPLE_RATE * 2;
    *(uint16_t*)(wavHeader + 32) = 2;
    *(uint16_t*)(wavHeader + 34) = 16;
    memcpy(wavHeader + 36, "data", 4);
    *(uint32_t*)(wavHeader + 40) = dataSize;

    server.setContentLength(44 + dataSize);
    server.send(200, "audio/wav", "");
    server.sendContent((const char*)wavHeader, 44);
    server.sendContent((const char*)audioBuffer, dataSize);
  });

  server.begin();
  Serial.println("Web server started");
}

void recordAudio() {
  Serial.println("Recording...");
  audioSampleCount = 0;
  
  unsigned long startTime = millis();
  size_t bytesRead;
  int32_t i2s_data[DMA_BUF_LEN];
  
  while (digitalRead(BUTTON_PIN) == LOW && (millis() - startTime < MAX_RECORD_TIME_MS)) {
    i2s_read(I2S_PORT, &i2s_data, DMA_BUF_LEN * 4, &bytesRead, portMAX_DELAY);
    int samplesRead = bytesRead / 4;
    
    for (int i = 0; i < samplesRead && audioSampleCount < MAX_RECORD_SAMPLES; i++) {
      audioBuffer[audioSampleCount++] = (int16_t)(i2s_data[i] >> 14);
    }

    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.drawStr(0, 20, "Recording...");
    int duration = (millis() - startTime) / 1000;
    String timeStr = String(duration) + "s";
    u8g2.drawStr(0, 35, timeStr.c_str());
    u8g2.sendBuffer();
  }
  
  Serial.printf("Recorded %d samples\n", audioSampleCount);
}

String transcribeAudio() {
  if (audioSampleCount == 0) return "";
  
  Serial.println("Transcribing...");
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tr);
  u8g2.drawStr(0, 20, "Transcribing...");
  u8g2.sendBuffer();

  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient https;

  String boundary = createMultipartBoundary();
  
  if (https.begin(client, "https://api.groq.com/openai/v1/audio/transcriptions")) {
    https.addHeader("Authorization", "Bearer " + String(GROQ_API_KEY));
    https.addHeader("Content-Type", "multipart/form-data; boundary=" + boundary);

    String body = "";
    body += "--" + boundary + "\r\n";
    body += "Content-Disposition: form-data; name=\"file\"; filename=\"audio.wav\"\r\n";
    body += "Content-Type: audio/wav\r\n\r\n";

    uint32_t dataSize = audioSampleCount * 2;
    uint32_t fileSize = dataSize + 36;
    uint8_t wavHeader[44];
    
    memcpy(wavHeader, "RIFF", 4);
    *(uint32_t*)(wavHeader + 4) = fileSize;
    memcpy(wavHeader + 8, "WAVE", 4);
    memcpy(wavHeader + 12, "fmt ", 4);
    *(uint32_t*)(wavHeader + 16) = 16;
    *(uint16_t*)(wavHeader + 20) = 1;
    *(uint16_t*)(wavHeader + 22) = 1;
    *(uint32_t*)(wavHeader + 24) = SAMPLE_RATE;
    *(uint32_t*)(wavHeader + 28) = SAMPLE_RATE * 2;
    *(uint16_t*)(wavHeader + 32) = 2;
    *(uint16_t*)(wavHeader + 34) = 16;
    memcpy(wavHeader + 36, "data", 4);
    *(uint32_t*)(wavHeader + 40) = dataSize;

    for (int i = 0; i < 44; i++) body += (char)wavHeader[i];
    for (int i = 0; i < audioSampleCount; i++) {
      body += (char)(audioBuffer[i] & 0xFF);
      body += (char)((audioBuffer[i] >> 8) & 0xFF);
    }

    body += "\r\n--" + boundary + "\r\n";
    body += "Content-Disposition: form-data; name=\"model\"\r\n\r\n";
    body += TRANSCRIBE_MODEL;
    body += "\r\n--" + boundary + "--\r\n";

    int httpCode = https.POST((uint8_t*)body.c_str(), body.length());
    
    if (httpCode == 200) {
      String response = https.getString();
      Serial.println("Transcription response: " + response);
      
      int textStart = response.indexOf("\"text\":\"") + 8;
      int textEnd = response.indexOf("\"", textStart);
      if (textStart > 7 && textEnd > textStart) {
        String text = response.substring(textStart, textEnd);
        https.end();
        return text;
      }
    } else {
      Serial.printf("Transcription failed: %d\n", httpCode);
    }
    https.end();
  }
  
  return "";
}

String getChatResponse(String userMessage) {
  Serial.println("Getting chat response for: " + userMessage);
  
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tr);
  u8g2.drawStr(0, 20, "AI thinking...");
  u8g2.sendBuffer();

  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient https;

  if (https.begin(client, "https://api.groq.com/openai/v1/chat/completions")) {
    https.addHeader("Authorization", "Bearer " + String(GROQ_API_KEY));
    https.addHeader("Content-Type", "application/json");

    String jsonPayload = "{\"model\":\"" + String(CHAT_MODEL) + "\",";
    jsonPayload += "\"messages\":[{\"role\":\"user\",\"content\":\"" + userMessage + "\"}],";
    jsonPayload += "\"max_tokens\":300}";

    int httpCode = https.POST(jsonPayload);
    
    if (httpCode == 200) {
      String response = https.getString();
      Serial.println("Chat response received");
      
      int contentStart = response.indexOf("\"content\":\"") + 11;
      int contentEnd = response.indexOf("\"", contentStart);
      if (contentStart > 10 && contentEnd > contentStart) {
        String content = response.substring(contentStart, contentEnd);
        content.replace("\\n", " ");
        https.end();
        return content;
      }
    } else {
      Serial.printf("Chat request failed: %d\n", httpCode);
    }
    https.end();
  }
  
  return "Error: Could not reach AI";
}

void fetchWeatherData() {
  Serial.println("Fetching weather...");
  
  // First get location from IP
  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient https;
  
  String lat = "";
  String lon = "";
  
  if (https.begin(client, "https://ipapi.co/json/")) {
    int httpCode = https.GET();
    if (httpCode == 200) {
      String response = https.getString();
      
      int cityStart = response.indexOf("\"city\":\"") + 8;
      int cityEnd = response.indexOf("\"", cityStart);
      if (cityStart > 7 && cityEnd > cityStart) {
        weatherLocation = response.substring(cityStart, cityEnd);
      }
      
      int latStart = response.indexOf("\"latitude\":") + 11;
      int latEnd = response.indexOf(",", latStart);
      if (latStart > 10 && latEnd > latStart) {
        lat = response.substring(latStart, latEnd);
      }
      
      int lonStart = response.indexOf("\"longitude\":") + 12;
      int lonEnd = response.indexOf(",", lonStart);
      if (lonStart > 11 && lonEnd > lonStart) {
        lon = response.substring(lonStart, lonEnd);
      }
    }
    https.end();
  }
  
  if (lat.length() > 0 && lon.length() > 0) {
    String weatherUrl = "https://api.open-meteo.com/v1/forecast?latitude=" + lat;
    weatherUrl += "&longitude=" + lon + "&current_weather=true";
    
    if (https.begin(client, weatherUrl)) {
      int httpCode = https.GET();
      if (httpCode == 200) {
        String response = https.getString();
        
        int tempStart = response.indexOf("\"temperature\":") + 14;
        int tempEnd = response.indexOf(",", tempStart);
        if (tempStart > 13 && tempEnd > tempStart) {
          weatherTemp = response.substring(tempStart, tempEnd) + "C";
        }
        
        int codeStart = response.indexOf("\"weathercode\":") + 14;
        int codeEnd = response.indexOf(",", codeStart);
        if (codeStart > 13 && codeEnd > codeStart) {
          int code = response.substring(codeStart, codeEnd).toInt();
          if (code == 0) weatherCondition = "Clear";
          else if (code <= 3) weatherCondition = "Cloudy";
          else if (code <= 67) weatherCondition = "Rainy";
          else if (code <= 77) weatherCondition = "Snowy";
          else weatherCondition = "Stormy";
        }
        
        lastWeatherUpdate = millis();
      }
      https.end();
    }
  }
}

void saveAssignment(String assignmentText) {
  Serial.println("Saving assignment: " + assignmentText);
  
  // First, parse the assignment with AI
  String aiPrompt = "Extract task details from this voice note into JSON format with fields: title, description, due_date (YYYY-MM-DD or null). Voice note: " + assignmentText;
  String aiParsed = getChatResponse(aiPrompt);
  
  // Save to Supabase
  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient https;
  
  String url = String(SUPABASE_URL) + "/rest/v1/assignments";
  
  if (https.begin(client, url)) {
    https.addHeader("apikey", SUPABASE_ANON_KEY);
    https.addHeader("Authorization", "Bearer " + String(SUPABASE_SERVICE_KEY));
    https.addHeader("Content-Type", "application/json");
    https.addHeader("Prefer", "return=minimal");
    
    String payload = "{\"user_id\":\"" + String(SUPABASE_USER_ID) + "\",";
    payload += "\"raw_text\":\"" + assignmentText + "\",";
    payload += "\"parsed_data\":" + aiParsed + "}";
    
    int httpCode = https.POST(payload);
    
    if (httpCode == 201 || httpCode == 200) {
      Serial.println("Assignment saved successfully");
    } else {
      Serial.printf("Failed to save assignment: %d\n", httpCode);
    }
    https.end();
  }
}

void startReactionGame() {
  gameWaitingForPress = false;
  gameReactionTime = 0;
  
  // Random delay 1-3 seconds
  unsigned long delayTime = random(1000, 3000);
  gameStartTime = millis() + delayTime;
  
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tr);
  u8g2.drawStr(0, 30, "Wait for GO!");
  u8g2.sendBuffer();
}

void handleReactionPress() {
  if (gameWaitingForPress) {
    gameReactionTime = millis() - gameStartTime;
    gameWaitingForPress = false;
  }
}

void drawStatusBar() {
  u8g2.setFont(u8g2_font_5x7_tr);
  
  // WiFi status
  if (WiFi.status() == WL_CONNECTED) {
    u8g2.drawStr(0, 8, "WiFi");
  }
  
  // Time
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    char timeStr[6];
    sprintf(timeStr, "%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);
    u8g2.drawStr(90, 8, timeStr);
  }
  
  u8g2.drawLine(0, 10, SCREEN_WIDTH, 10);
}

void drawMenu() {
  drawStatusBar();
  
  u8g2.setFont(u8g2_font_6x10_tr);
  u8g2.drawStr(30, 25, "MENU");
  
  int startY = 35;
  for (int i = 0; i < MENU_ITEMS; i++) {
    if (i == menuSelection) {
      u8g2.drawStr(10, startY + (i * 12), ">");
    }
    u8g2.drawStr(20, startY + (i * 12), menuLabels[i]);
  }
  
  u8g2.drawLine(0, SCREEN_HEIGHT - UI_BOTTOM_TAB_H, SCREEN_WIDTH, SCREEN_HEIGHT - UI_BOTTOM_TAB_H);
  u8g2.setFont(u8g2_font_5x7_tr);
  u8g2.drawStr(5, SCREEN_HEIGHT - 3, "Prev");
  u8g2.drawStr(45, SCREEN_HEIGHT - 3, "Next");
  u8g2.drawStr(85, SCREEN_HEIGHT - 3, "Select");
}

void drawAIMode() {
  drawStatusBar();
  
  u8g2.setFont(u8g2_font_6x10_tr);
  u8g2.drawStr(40, 25, "AI MODE");
  
  if (aiResponse.length() > 0) {
    u8g2.setFont(u8g2_font_5x7_tr);
    int y = 35;
    int charWidth = 5;
    int maxCharsPerLine = (SCREEN_WIDTH - 4) / charWidth;
    
    int startIdx = responseScrollOffset * maxCharsPerLine;
    for (int line = 0; line < 3 && startIdx < aiResponse.length(); line++) {
      String lineText = aiResponse.substring(startIdx, min((int)aiResponse.length(), startIdx + maxCharsPerLine));
      u8g2.drawStr(2, y + (line * 10), lineText.c_str());
      startIdx += maxCharsPerLine;
    }
  } else {
    u8g2.setFont(u8g2_font_5x7_tr);
    u8g2.drawStr(5, 40, "Hold button");
    u8g2.drawStr(5, 50, "to record");
  }
  
  u8g2.drawLine(0, SCREEN_HEIGHT - UI_BOTTOM_TAB_H, SCREEN_WIDTH, SCREEN_HEIGHT - UI_BOTTOM_TAB_H);
  u8g2.setFont(u8g2_font_5x7_tr);
  u8g2.drawStr(5, SCREEN_HEIGHT - 3, "Record");
  u8g2.drawStr(45, SCREEN_HEIGHT - 3, "Scroll");
  u8g2.drawStr(85, SCREEN_HEIGHT - 3, "Menu");
}

void drawWeatherMode() {
  drawStatusBar();
  
  u8g2.setFont(u8g2_font_6x10_tr);
  u8g2.drawStr(30, 25, "WEATHER");
  
  if (weatherLocation.length() > 0) {
    u8g2.setFont(u8g2_font_5x7_tr);
    u8g2.drawStr(5, 35, weatherLocation.c_str());
    
    u8g2.setFont(u8g2_font_10x20_tr);
    u8g2.drawStr(5, 52, weatherTemp.c_str());
    
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.drawStr(60, 52, weatherCondition.c_str());
  } else {
    u8g2.setFont(u8g2_font_5x7_tr);
    u8g2.drawStr(5, 40, "No data");
  }
  
  u8g2.drawLine(0, SCREEN_HEIGHT - UI_BOTTOM_TAB_H, SCREEN_WIDTH, SCREEN_HEIGHT - UI_BOTTOM_TAB_H);
  u8g2.setFont(u8g2_font_5x7_tr);
  u8g2.drawStr(5, SCREEN_HEIGHT - 3, "Menu");
  u8g2.drawStr(45, SCREEN_HEIGHT - 3, "Refresh");
}

void drawAssignmentMode() {
  drawStatusBar();
  
  u8g2.setFont(u8g2_font_6x10_tr);
  u8g2.drawStr(20, 25, "ASSIGNMENT");
  
  u8g2.setFont(u8g2_font_5x7_tr);
  if (recordingAssignment) {
    u8g2.drawStr(5, 40, "Recording...");
  } else {
    u8g2.drawStr(5, 40, "Hold Btn3 to");
    u8g2.drawStr(5, 50, "record task");
  }
  
  u8g2.drawLine(0, SCREEN_HEIGHT - UI_BOTTOM_TAB_H, SCREEN_WIDTH, SCREEN_HEIGHT - UI_BOTTOM_TAB_H);
  u8g2.setFont(u8g2_font_5x7_tr);
  u8g2.drawStr(5, SCREEN_HEIGHT - 3, "Menu");
}

void drawGameMode() {
  drawStatusBar();
  
  u8g2.setFont(u8g2_font_6x10_tr);
  u8g2.drawStr(25, 25, "REACTION");
  
  if (gameReactionTime > 0) {
    u8g2.setFont(u8g2_font_10x20_tr);
    String timeStr = String(gameReactionTime) + "ms";
    u8g2.drawStr(10, 45, timeStr.c_str());
  } else if (gameWaitingForPress) {
    u8g2.setFont(u8g2_font_10x20_tr);
    u8g2.drawStr(30, 45, "GO!");
  } else if (gameStartTime > 0) {
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.drawStr(20, 40, "Wait...");
  } else {
    u8g2.setFont(u8g2_font_5x7_tr);
    u8g2.drawStr(5, 40, "Press to start");
  }
  
  u8g2.drawLine(0, SCREEN_HEIGHT - UI_BOTTOM_TAB_H, SCREEN_WIDTH, SCREEN_HEIGHT - UI_BOTTOM_TAB_H);
  u8g2.setFont(u8g2_font_5x7_tr);
  u8g2.drawStr(5, SCREEN_HEIGHT - 3, "Menu");
  u8g2.drawStr(45, SCREEN_HEIGHT - 3, "Start");
}

void drawScreensaver() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.drawStr(30, 32, "No Time");
    return;
  }
  
  // Draw analog clock
  int centerX = 64;
  int centerY = 32;
  int radius = 28;
  
  // Clock circle
  u8g2.drawCircle(centerX, centerY, radius);
  
  // Hour hand
  float hourAngle = ((timeinfo.tm_hour % 12) + timeinfo.tm_min / 60.0) * 30 - 90;
  int hourX = centerX + (radius * 0.5) * cos(hourAngle * PI / 180);
  int hourY = centerY + (radius * 0.5) * sin(hourAngle * PI / 180);
  u8g2.drawLine(centerX, centerY, hourX, hourY);
  
  // Minute hand
  float minAngle = timeinfo.tm_min * 6 - 90;
  int minX = centerX + (radius * 0.8) * cos(minAngle * PI / 180);
  int minY = centerY + (radius * 0.8) * sin(minAngle * PI / 180);
  u8g2.drawLine(centerX, centerY, minX, minY);
  
  // Digital time at bottom
  char timeStr[9];
  sprintf(timeStr, "%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
  u8g2.setFont(u8g2_font_6x10_tr);
  u8g2.drawStr(38, 62, timeStr);
}

void checkButtons() {
  unsigned long now = millis();
  static unsigned long btnPressStart = 0;
  static unsigned long scrollPressStart = 0;
  static unsigned long assignPressStart = 0;
  static bool btnWasPressed = false;
  static bool scrollWasPressed = false;
  static bool assignWasPressed = false;
  
  // Main button (GPIO 7)
  bool btnPressed = (digitalRead(BUTTON_PIN) == LOW);
  if (btnPressed && !btnWasPressed) {
    btnPressStart = now;
    btnWasPressed = true;
  } else if (!btnPressed && btnWasPressed) {
    unsigned long pressDuration = now - btnPressStart;
    btnWasPressed = false;
    
    if (now - lastButtonPress > DEBOUNCE_DELAY) {
      lastButtonPress = now;
      lastActivityTime = now;
      
      if (currentMode == MODE_SCREENSAVER) {
        currentMode = MODE_MENU;
      } else if (currentMode == MODE_MENU) {
        if (pressDuration < LONG_PRESS_DELAY) {
          menuSelection = (menuSelection - 1 + MENU_ITEMS) % MENU_ITEMS;
        } else {
          // Long press - select menu item
          switch (menuSelection) {
            case 0: currentMode = MODE_AI; aiResponse = ""; responseScrollOffset = 0; break;
            case 1: currentMode = MODE_WEATHER; fetchWeatherData(); break;
            case 2: currentMode = MODE_ASSIGNMENTS; break;
            case 3: currentMode = MODE_GAME; startReactionGame(); break;
            case 4: currentMode = MODE_SCREENSAVER; break;
          }
        }
      } else if (currentMode == MODE_GAME && !gameWaitingForPress) {
        if (gameStartTime == 0 || gameReactionTime > 0) {
          startReactionGame();
        }
      }
    }
  }
  
  // Handle recording in AI mode
  if (currentMode == MODE_AI && btnPressed && (now - btnPressStart > LONG_PRESS_DELAY)) {
    static bool isRecording = false;
    if (!isRecording) {
      isRecording = true;
      recordAudio();
      String transcription = transcribeAudio();
      if (transcription.length() > 0) {
        aiResponse = getChatResponse(transcription);
        responseScrollOffset = 0;
      }
      isRecording = false;
    }
  }
  
  // Scroll button (GPIO 8)
  bool scrollPressed = (digitalRead(BUTTON_SCROLL_PIN) == LOW);
  if (scrollPressed && !scrollWasPressed) {
    scrollPressStart = now;
    scrollWasPressed = true;
  } else if (!scrollPressed && scrollWasPressed) {
    unsigned long pressDuration = now - scrollPressStart;
    scrollWasPressed = false;
    
    if (now - lastScrollPress > DEBOUNCE_DELAY) {
      lastScrollPress = now;
      lastActivityTime = now;
      
      if (currentMode == MODE_MENU) {
        menuSelection = (menuSelection + 1) % MENU_ITEMS;
      } else if (currentMode == MODE_AI) {
        if (pressDuration < LONG_PRESS_DELAY) {
          responseScrollOffset++;
        } else {
          returnToMenu();
        }
      } else if (currentMode == MODE_WEATHER) {
        if (pressDuration < LONG_PRESS_DELAY) {
          fetchWeatherData();
        } else {
          returnToMenu();
        }
      } else if (currentMode == MODE_GAME) {
        if (gameWaitingForPress) {
          handleReactionPress();
        } else if (pressDuration >= LONG_PRESS_DELAY) {
          returnToMenu();
        }
      } else if (currentMode == MODE_ASSIGNMENTS && pressDuration >= LONG_PRESS_DELAY) {
        returnToMenu();
      }
    }
  }
  
  // Assignment button (GPIO 9)
  bool assignPressed = (digitalRead(BUTTON_ASSIGNMENT_PIN) == LOW);
  if (assignPressed && !assignWasPressed) {
    assignPressStart = now;
    assignWasPressed = true;
    
    if (currentMode == MODE_ASSIGNMENTS && (now - lastAssignmentPress > DEBOUNCE_DELAY)) {
      recordingAssignment = true;
      recordAudio();
      String transcription = transcribeAudio();
      if (transcription.length() > 0) {
        saveAssignment(transcription);
      }
      recordingAssignment = false;
    }
  } else if (!assignPressed && assignWasPressed) {
    assignWasPressed = false;
    lastAssignmentPress = now;
    lastActivityTime = now;
  }
  
  // Game logic
  if (currentMode == MODE_GAME && gameStartTime > 0 && !gameWaitingForPress && gameReactionTime == 0) {
    if (millis() >= gameStartTime) {
      gameWaitingForPress = true;
      gameStartTime = millis();
    }
  }
}

void returnToMenu() {
  currentMode = MODE_MENU;
  lastActivityTime = millis();
}

String urlEncode(String str) {
  String encoded = "";
  char c;
  for (int i = 0; i < str.length(); i++) {
    c = str.charAt(i);
    if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
      encoded += c;
    } else {
      encoded += '%';
      encoded += String(c, HEX);
    }
  }
  return encoded;
}

String createMultipartBoundary() {
  return "----WebKitFormBoundary" + String(random(1000000, 9999999));
}
