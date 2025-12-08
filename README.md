# ESPAssistant

A small portable AI Assistant computer built with ESP32.

## Description

This compact computer is equipped with an ESP32 Seed Studio microcontroller, featuring three buttons, a small OLED display, and an INMP441 microphone. It connects to a smartphone's hotspot. Upon pressing and holding one of the buttons, it records audio, which is then transcribed using the Groq Whisper API and sent to the Groq Llama 3.1 API for a chat response. The response is displayed on the OLED screen.

## Hardware Design

The custom PCB was designed using EasyEDA. Below are the 3D renders and PCB layout:

### PCB Layout

![PCB view](https://github.com/Laurentsandler/ESPAssistant/blob/main/PCB/PCB%20design%20images%20update%20(1).png)


### 3D Renders

![PCB 3D View - Back Angle](https://github.com/Laurentsandler/ESPAssistant/blob/main/PCB/PCB%20design%20images%20update%20(2).png)

![PCB 3D View - Side](https://github.com/Laurentsandler/ESPAssistant/blob/main/PCB/PCB%20design%20images%20update%20(3).png)

![PCB 3D View - Front](https://github.com/Laurentsandler/ESPAssistant/blob/main/PCB/PCB%20design%20images%20update.png)

## Features

- **Voice Recording**: Press and hold to record audio via INMP441 I2S microphone
- **Speech-to-Text**: Transcription via Groq Whisper API
- **AI Chat**: Responses powered by Llama 3.1 via Groq API
- **OLED Display**: 128x64 SH1106/SSD1306 display with modern UI
- **Weather App**: Real-time weather with geolocation
- **Reaction Game**: Fun reaction time game
- **Assignment Tracker**: Voice-record assignments and save to Supabase
- **Screensaver**: Analog clock with digital time display
- **Web Server**: Access recorded audio via browser

## Hardware Requirements

| Component | Specification | Pin Connection |
|-----------|---------------|----------------|
| Microcontroller | ESP32 (Seed Studio) | - |
| Microphone | INMP441 I2S | WS=4, SD=2, SCK=3 |
| Display | SH1106/SSD1306 128x64 OLED | SDA=5, SCL=6 |
| Button 1 (Main) | Momentary push button | GPIO 7 |
| Button 2 (Scroll) | Momentary push button | GPIO 8 |
| Button 3 (Assignment) | Momentary push button | GPIO 9 |
| Button 4 (Extra) | Momentary push button | GPIO 1 |

## Software Dependencies

Install these libraries via Arduino IDE Library Manager:
- WiFi (built-in)
- WiFiClientSecure (built-in)
- HTTPClient (built-in)
- WebServer (built-in)
- ESPmDNS (built-in)
- U8g2lib
- Wire (built-in)

## Configuration

Before uploading, edit the following in `Firmware/sketch.ino`:

```cpp
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASS = "YOUR_WIFI_PASSWORD";
const char* GROQ_API_KEY = "YOUR_GROQ_API_KEY";

const char* SUPABASE_URL = "YOUR_SUPABASE_URL";
const char* SUPABASE_ANON_KEY = "YOUR_SUPABASE_ANON_KEY";
const char* SUPABASE_USER_ID = "YOUR_SUPABASE_USER_ID";
const char* SUPABASE_SERVICE_KEY = "YOUR_SUPABASE_SERVICE_KEY";
```

## Usage

### Menu Navigation
- **Short press Button 1 (GPIO 7)**: Navigate left in menu
- **Short press Button 2 (GPIO 8)**: Navigate right in menu
- **Long press Button 1**: Select/Enter app

### AI Mode
- **Hold Button 1**: Record audio
- **Release**: Transcribe and get AI response
- **Button 2**: Scroll through long responses
- **Long press Button 2**: Return to menu

### Weather Mode
- Automatically fetches weather based on IP geolocation
- **Short press Button 2**: Force refresh
- **Long press Button 1**: Return to menu

### Assignment Mode
- **Hold Button 3 (GPIO 9)**: Record assignment details
- Voice input is parsed by AI and saved to Supabase

## Web Interface

After connecting to WiFi, access the web interface at:
- `http://<ESP32_IP>/` - Home page
- `http://<ESP32_IP>/audio.wav` - Download last recording
- `http://esp32.local/audio.wav` - Via mDNS (if supported)

## API Services Used

- **Groq API**: Speech-to-text (Whisper) and chat (Llama 3.1)
- **Open-Meteo API**: Weather data
- **ipapi.co**: IP-based geolocation
- **Supabase**: Database for assignment storage

## Case
![Case mockup bottom view.jpg](https://github.com/Laurentsandler/ESPAssistant/blob/main/Images/Case%20mockup%20bottom%20view.jpg)
![Case mockup side veiw.jpg](https://github.com/Laurentsandler/ESPAssistant/blob/main/Images/Case%20mockup%20side%20veiw.jpg)
![Case mockup top alternate view.jpg](https://github.com/Laurentsandler/ESPAssistant/blob/main/Images/Case%20mockup%20top%20alternate%20view.jpg)
![Case mockup top view.jpg](https://github.com/Laurentsandler/ESPAssistant/blob/main/Images/Case%20mockup%20top%20view.jpg)

## Bom: 
Component,Footprint,Quantity,Supplier Part Number,price,link
OLED SD130,OLED-TH12864,1,C9900014198,10€,https://amzn.eu/d/837vRA1
INMP441-MEMS-I2S-MIC-MODULE,INMP441-MEMS-I2S-MIC-MODULE,1,N/A,7€,https://amzn.eu/d/0Qtf1CE
SEEED XIAO ESP32S3,SEEED STUDIO XIAO_S3 BY LAURENT,1,N/A,20€,https://amzn.eu/d/1FE54ig
TS-1187A-B-A-B,SW-SMD_4P-L5.1-W5.1-P3.70-LS6.5-TL_H1.5,4,C318884,0.32€,https://www.lcsc.com/product-detail/C318884.html?s_z=n_TS-1187A-B-A-B
500175140 PogoPin Connector, N/A,1,B09STS5YZX,7.90€,https://amzn.eu/d/2DIAPW8

## License

MIT License
