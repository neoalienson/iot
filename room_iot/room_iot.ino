#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WiFiMulti.h> 
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <DHT.h>
#include <time.h>

const char* ssid = "Neo-Home";
const char* password = "Your password";

uint8_t IrPin  = 4; // D2 for IR
uint8_t LedPin = 14; // D5 for LED flash together with IR
DHT dht(D6, DHT11); // D6 for DHT11 (temperature and humidity) sensor
// D7 for CO2 sensor RX
// D8 for CO2 sensor TX

ESP8266WebServer server(80);
ESP8266WiFiMulti wifiMulti;     // Create an instance of the ESP8266WiFiMulti class, called 'wifiMulti'

unsigned long warmingTimer = 0;
unsigned long timer1 = 0;

typedef enum HvacMode {
  HVAC_HOT,
  HVAC_COLD,
  HVAC_DRY,
  HVAC_FAN, // used for Panasonic only
  HVAC_AUTO
} HvacMode_t; // HVAC  MODE

typedef enum HvacFanMode {
  FAN_SPEED_1,
  FAN_SPEED_2,
  FAN_SPEED_3,
  FAN_SPEED_4,
  FAN_SPEED_5,
  FAN_SPEED_AUTO,
  FAN_SPEED_SILENT
} HvacFanMode_;  // HVAC  FAN MODE

typedef enum HvacVanneMode {
  VANNE_AUTO,
  VANNE_H1,
  VANNE_H2,
  VANNE_H3,
  VANNE_H4,
  VANNE_H5,
  VANNE_AUTO_MOVE
} HvacVanneMode_;  // HVAC  VANNE MODE

typedef enum HvacWideVanneMode {
  WIDE_LEFT_END,
  WIDE_LEFT,
  WIDE_MIDDLE,
  WIDE_RIGHT,
  WIDE_RIGHT_END
} HvacWideVanneMode_t;  // HVAC  WIDE VANNE MODE

typedef enum HvacAreaMode {
  AREA_SWING,
  AREA_LEFT,
  AREA_AUTO,
  AREA_RIGHT
} HvacAreaMode_t;  // HVAC  WIDE VANNE MODE

typedef enum HvacProfileMode {
  NORMAL,
  QUIET,
  BOOST
} HvacProfileMode_t;  // HVAC PANASONIC OPTION MODE


// HVAC TOSHIBA_
#define HVAC_TOSHIBA_HDR_MARK    4400
#define HVAC_TOSHIBA_HDR_SPACE   4300
#define HVAC_TOSHIBA_BIT_MARK    543
#define HVAC_TOSHIBA_ONE_SPACE   1623
#define HVAC_MISTUBISHI_ZERO_SPACE  472
#define HVAC_TOSHIBA_RPT_MARK    440
#define HVAC_TOSHIBA_RPT_SPACE   7048 // Above original iremote limit

void handleVersion() {
  server.send(200, "text/plain", "version 1.0\nCommands\n/version\n/co2\n/temperature\n/humidity\n/serial/swap\n/ir/recv\n/ir/send?cmd=\n");  
}

void setup(void) {
  WiFi.softAPdisconnect (true);

  Serial.begin(9600);  // Setup a serial connection with the sensor
  delay(100);

  warmingTimer = millis();  // initilize warmup timer
  wifiMulti.addAP(ssid, password);   // add Wi-Fi networks you want to connect to

  Serial.print("Connecting to ");
  Serial.print(ssid);
  int i = 0;
  while (wifiMulti.run() != WL_CONNECTED) { // Wait for the Wi-Fi to connect: scan for Wi-Fi networks, and connect to the strongest of the networks above
    delay(250);
    Serial.print(++i); Serial.print('.');    
  }
  Serial.println('\n');
  Serial.println("Connection established!");  
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());         // Send the IP address of the ESP8266 to the computer
      
  server.on("/", handleRoot);               // Call the 'handleRoot' function when a client requests URI "/"
  server.on("/version", handleVersion);
  server.on("/co2", handleCO2);
  server.on("/temperature", handleTemperature);
  server.on("/humidity", handleHumidity);
  server.on("/serial/swap", handleSerialSwap);
  server.on("/toshiba_ac/on", handleToshibaAcOn);
  server.on("/toshiba_ac/off", handleToshibaAcOff);
  server.on("/ir/nec/recv", handleInfraRedNecReceive);
  server.on("/ir/nec/send", handleInfraRedNecSend);
  server.onNotFound(handleNotFound);        // When a client requests an unknown URI (i.e. something other than "/"), call function "handleNotFound"

  Serial.println("Start DHT");  
  dht.begin(); 
  pinMode(IrPin, OUTPUT); 
  pinMode(LedPin, OUTPUT); 
  Serial.println("Start Web Server");
  server.begin();                           // Actually start the server  
}


void handleRoot() {
  server.send(200, "text/plain", "ESP8266");  
}

void handleSerialSwap() {
  Serial.swap();
}

void handleToshibaAcOn() {
  int temp = 25;
  HvacFanMode fan = FAN_SPEED_AUTO;
  if (server.hasArg("temp")) {
    temp = max((int)server.arg("temp").toInt(), 25);
  }
 
  if (server.hasArg("fan")) {
    if (server.arg("fan").equals("auto")) {
        fan = FAN_SPEED_AUTO;
    } else if (server.arg("fan").equals("silent")) {
        fan =   FAN_SPEED_SILENT;
    } else if (server.arg("fan").equals("1")) {
        fan =   FAN_SPEED_1;
    } else if (server.arg("fan").equals("2")) {
        fan =   FAN_SPEED_2;
    } else if (server.arg("fan").equals("3")) {
        fan =   FAN_SPEED_3;
    } else if (server.arg("fan").equals("4")) {
        fan =   FAN_SPEED_4;
    } else if (server.arg("fan").equals("5")) {
        fan =   FAN_SPEED_5;
    }
  }
  sendHvacToshiba(HVAC_COLD, temp, fan, false);
  server.send(200, "text/plain", "on\nTemperature: " + String(temp));
}

void handleToshibaAcOff() {
  sendHvacToshiba(HVAC_COLD, 25, FAN_SPEED_AUTO, true);
  server.send(200, "text/plain", "off");
}

void handleInfraRedNecReceive() {
  Serial.swap();
  while (Serial.available())  // this clears out any garbage in the RX buffer
  {
    int garbage = Serial.read();
  }
  Serial.swap();

  Serial.println("Waiting for IR signal 10s");
  delay(100);
  Serial.swap();
  int i = 0;
  while (!Serial.available())  // this pauses the sketch and waiting for the sensor responce
  {
    delay(10);
    i++;
    if (i == 1000) {
      Serial.swap();
      Serial.println("No signal received."); 
      server.send(200, "text/plain", "No signal received.");  
      return;
    }
  }

  String hexstring;
  while (Serial.available()) {
    char dataString[3] = {0};
    sprintf(dataString,"%02X", Serial.read());
    hexstring += dataString;
  }
  Serial.swap();
  server.send(200, "text/plain", hexstring);  
  Serial.println(hexstring);
  Serial.println("End of signal waiting."); 
}

void handleInfraRedNecSend() {
  byte buff[5];
  String cmd_str = server.arg("cmd");
  cmd_str.toUpperCase();
  char cmd[10];
  cmd_str.toCharArray(cmd, 10);
  hex2bytes(cmd, buff, 5);
  Serial.swap();
  Serial.write(buff, 5);  // Sent out read command to the sensor
  Serial.flush();  // this pauses the sketch and waits for the TX buffer to send all its data to the sensor

  delay(1000);
  String hexstring;
  while (Serial.available()) {
    char dataString[3] = {0};
    sprintf(dataString,"%02X", Serial.read());
    hexstring += dataString;
  }
    
  Serial.swap();
  server.send(200, "text/plain", "Sent: " + hexstring);  
}

void handleCO2() {
  float co2;
  int humidity;
  mhz14aRead(&co2, &humidity);
  Serial.print("CO2:\t");
  Serial.println(String(co2)); 
  Serial.print("Humidity:\t");
  Serial.println(String(humidity)); 
  if (co2 < 0)
    server.send(200, "text/plain", "nan");
  else 
    server.send(200, "text/plain", String(co2));
}

byte hex2byte(char hex){
  if (hex >= 'A' && hex <= 'F') {
    return hex - 'A' + 0xA;
  }
  if (hex >= '1' && hex <= '9') {
    return hex - '1' + 1;
  }
  return 0;
}

void hex2bytes(char* hex, byte* bytes_array, int array_size) {
 int len = strlen(hex);
 
 if (len % 2 != 0) {
   for (int i = 0; i < array_size; i++) {
    bytes_array[i] = 0;
   }
 }
 
 for (int i = 0; i < len * 2 && i < array_size; i += 1)  {  
   bytes_array[i] = hex2byte(hex[i*2]) << 4 | hex2byte(hex[i*2 + 1]);
 }
}

void handleTemperature() {
  float temp = dht.readTemperature();
  Serial.print("Temperature:\t");
  Serial.println(String(temp)); 
  server.send(200, "text/plain", String(temp));
}

void handleHumidity() {
  float hum = dht.readHumidity();
  Serial.print("Humidity:\t");
  Serial.println(String(hum)); 
  server.send(200, "text/plain", String(hum));
}

void handleNotFound(){
  server.send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}

void loop(void) {
   server.handleClient();                    // Listen for HTTP requests from clients
}

/****************************************************************************
/* MHZ-14A CO2 sensor
/***************************************************************************/ 
void mhz14aRead(float* co2, int* humidity)
{
  byte cmd[9] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};  // get gas command
  byte response[9];  // holds the recieved data
  
  *co2 = -1;
  *humidity = -1;
  
  Serial.swap();
  while (Serial.available())  // this clears out any garbage in the RX buffer
  {
    int garbage = Serial.read();
  }

  Serial.write(cmd, 9);  // Sent out read command to the sensor
  Serial.flush();  // this pauses the sketch and waits for the TX buffer to send all its data to the sensor

  int i = 0;
  while (!Serial.available())  // this pauses the sketch and waiting for the sensor responce
  {
    delay(10);
    i++;
    if (i == 100) {
      Serial.swap();
      return;
    }
  }
  Serial.readBytes(response, 9);  // once data is avilable, it reads it to a variable
  Serial.swap();

  if (mhz14aGetCheckSum(response) != response[8]) {
    return;
  }
  
  *co2 = (256 * (int)response[2]) + (int)response[3];
  *humidity = (int)response[4];
}

byte mhz14aGetCheckSum(byte* packet) {
  byte i;
  unsigned char checksum = 0;
  for (i = 1; i < 8; i++) {
    checksum += packet[i];
  }
  checksum = 0xff - checksum;
  checksum += 1;
  return checksum;
}


void mhz14aCalibrate()
{
  byte cmdCal[9] = {0xFF, 0x01, 0x87, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78};  // calibrate command
  Serial.write(cmdCal, 9);
  delay(3000);
}



/****************************************************************************
/* Toshiba AC
/***************************************************************************/ 
// 38khz
int halfPeriodicTime = 500/38; 

/****************************************************************************
/* enableIROut : Set global Variable for Frequency IR Emission
/***************************************************************************/ 
void enableIROut(int khz) {
  // Enables IR output.  The khz value controls the modulation frequency in kilohertz.
  halfPeriodicTime = 500/khz; // T = 1/f but we need T/2 in microsecond and f is in kHz
}

/****************************************************************************
/* mark ( int time) 
/***************************************************************************/ 
void mark(int time) {
  // Sends an IR mark for the specified number of microseconds.
  // The mark output is modulated at the PWM frequency.
  long beginning = micros();
  while(micros() - beginning < time){
    digitalWrite(IrPin, HIGH);
    delayMicroseconds(halfPeriodicTime);
    digitalWrite(IrPin, LOW);
    delayMicroseconds(halfPeriodicTime); //38 kHz -> T = 26.31 microsec (periodic time), half of it is 13
  }
}

/****************************************************************************
/* space ( int time) 
/***************************************************************************/ 
/* Leave pin off for time (given in microseconds) */
void space(int time) {
  // Sends an IR space for the specified number of microseconds.
  // A space is no output, so the PWM output is disabled.
  digitalWrite(IrPin, LOW);
  if (time > 0) delayMicroseconds(time);
}

/****************************************************************************
/* sendRaw (unsigned int buf[], int len, int hz)
/***************************************************************************/ 
void sendRaw (unsigned int buf[], int len, int hz)
{
  enableIROut(hz);
  for (int i = 0; i < len; i++) {
    if (i & 1) {
      space(buf[i]);
    } 
    else {
      mark(buf[i]);
    }
  }
  space(0); // Just to be sure
}

/****************************************************************************
/* Send IR command to Toshiba HVAC - sendHvacToshiba
/***************************************************************************/
void sendHvacToshiba(
  HvacMode                HVAC_Mode,           // Example HVAC_HOT  
  int                     HVAC_Temp,           // Example 21  (°c)
  HvacFanMode             HVAC_FanMode,        // Example FAN_SPEED_AUTO  
  int                     OFF                  // Example false
)
{
#define HVAC_TOSHIBA_DATALEN 9
// #define  HVAC_TOSHIBA_DEBUG;  // Un comment to access DEBUG information through Serial Interface

  byte mask = 1; //our bitmask
  //﻿F20D03FC0150000051
  byte data[HVAC_TOSHIBA_DATALEN] = { 0xF2, 0x0D, 0x03, 0xFC, 0x01, 0x00, 0x00, 0x00, 0x00 };
  // data array is a valid trame, only byte to be chnaged will be updated.

  byte i;

  Serial.print("sendHvacToshiba State: " + String(OFF));
  Serial.println(", Temp: " + String(HVAC_Temp));
  digitalWrite(LedPin, HIGH);

#ifdef HVAC_TOSHIBA_DEBUG
  Serial.println("Packet to send: ");
  for (i = 0; i < HVAC_TOSHIBA_DATALEN; i++) {
    Serial.print("_");
    Serial.print(data[i], HEX);
  }
  Serial.println(".");
#endif

  data[6] = 0x00;
  // Byte 7 - Mode
  switch (HVAC_Mode)
  {
    case HVAC_HOT:   data[6] = (byte) B00000011; break;
    case HVAC_COLD:  data[6] = (byte) B00000001; break;
    case HVAC_DRY:   data[6] = (byte) B00000010; break;
    case HVAC_AUTO:  data[6] = (byte) B00000000; break;
    default: break;
  }


  // Byte 7 - On / Off
  if (OFF) {
    data[6] = (byte) 0x07; // Turn OFF HVAC
  } else {
     // Turn ON HVAC (default)
  }

  // Byte 6 - Temperature
  // Check Min Max For Hot Mode
  byte Temp;
  if (HVAC_Temp > 30) { Temp = 30;}
  else if (HVAC_Temp < 17) { Temp = 17; } 
  else { Temp = HVAC_Temp; };
  data[5] = (byte) Temp - 17<<4;

  // Byte 10 - FAN / VANNE
  switch (HVAC_FanMode)
  {
    case FAN_SPEED_1:       data[6] = data[6] | (byte) B01000000; break;
    case FAN_SPEED_2:       data[6] = data[6] | (byte) B01100000; break;
    case FAN_SPEED_3:       data[6] = data[6] | (byte) B10000000; break;
    case FAN_SPEED_4:       data[6] = data[6] | (byte) B10100000; break;
    case FAN_SPEED_5:       data[6] = data[6] | (byte) B11000000; break; 
    case FAN_SPEED_AUTO:    data[6] = data[6] | (byte) B00000000; break;
    case FAN_SPEED_SILENT:  data[6] = data[6] | (byte) B00000000; break;//No FAN speed SILENT for TOSHIBA so it is consider as Speed AUTO
    default: break;
  }

  // Byte 9 - CRC
  data[8] = 0;
  for (i = 0; i < HVAC_TOSHIBA_DATALEN - 1; i++) {
    data[HVAC_TOSHIBA_DATALEN-1] = (byte) data[i] ^ data[HVAC_TOSHIBA_DATALEN -1];  // CRC is a simple bits addition
  }

#ifdef HVAC_TOSHIBA_DEBUG
  Serial.println("Packet to send: ");
  for (i = 0; i < HVAC_TOSHIBA_DATALEN; i++) {
    Serial.print("_"); Serial.print(data[i], HEX);
  }
  Serial.println(".");
  for (i = 0; i < HVAC_TOSHIBA_DATALEN ; i++) {
    Serial.print(data[i], BIN); Serial.print(" ");
  }
  Serial.println(".");
#endif

  enableIROut(38);  // 38khz
  space(0);
  for (int j = 0; j < 2; j++) {  // For Mitsubishi IR protocol we have to send two time the packet data
    // Header for the Packet
    mark(HVAC_TOSHIBA_HDR_MARK);
    space(HVAC_TOSHIBA_HDR_SPACE);
    for (i = 0; i < HVAC_TOSHIBA_DATALEN; i++) {
      // Send all Bits from Byte Data in Reverse Order
      for (mask = 10000000; mask > 0; mask >>= 1) { //iterate through bit mask
        if (data[i] & mask) { // Bit ONE
          mark(HVAC_TOSHIBA_BIT_MARK);
          space(HVAC_TOSHIBA_ONE_SPACE);
        }
        else { // Bit ZERO
          mark(HVAC_TOSHIBA_BIT_MARK);
          space(HVAC_MISTUBISHI_ZERO_SPACE);
        }
        //Next bits
      }
    }
    // End of Packet and retransmission of the Packet
    mark(HVAC_TOSHIBA_RPT_MARK);
    space(HVAC_TOSHIBA_RPT_SPACE);
    space(0); // Just to be sure
  }
  digitalWrite(LedPin, LOW);
}
