#include <Arduino.h>
#include <Adafruit_MLX90614.h>
#include <SoftwareSerial.h>
#include <MFRC522.h>
#include <SPI.h>
#include <Ethernet.h>

#define DEBUG true
#define RX 5
#define TX 6
#define SS_PIN 4
#define RST_PIN 8

// Initalise SoftwareSerial and RFID object structures
SoftwareSerial ESP(RX, TX);
MFRC522 mfrc522(SS_PIN, RST_PIN); 

// Set the static IP address to use if the DHCP fails to assign
IPAddress ip(172, 20, 10, 50);
IPAddress myDns(172, 20, 10, 1);

// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
EthernetClient client;

// Set constants for connection 
String AP = "vivo1904";       // CHANGE ME
String PASS = "e3a66d08ef2b"; // CHANGE MEString API = "YOUR_API_KEY";   // CHANGE ME
String HOST = "139.180.186.167";
char server[] = "139.180.186.167";    // name address for Google (using DNS)
String PORT = "51111";

// Mac address of eth card
byte mac[] = { 0xDE, 0xAC, 0xBE, 0xEF, 0xFE, 0xED };

// Temp Sensor Adjust Settings
double temp_sensor_adjust = 1.151; 
int temp_min = 30; 

// At command read and timeout configuration
boolean AT_cmd_result = false;
int AT_cmd_time;
int at_fail_count = 0;

unsigned long beginMicros, endMicros;
unsigned long byteCount = 0;

// ESP-ETH Selection
bool eth = false;

Adafruit_MLX90614 mlx = Adafruit_MLX90614();

int cmd_result;

// Set pin assignments
int inPin = 3;         // the number of the input pin
int outPin = 2;         // the number of the input pin
int outPin2 = 9;         // the number of the input pin
int onboardLED = 13;       // the number of the output pin

void setup() {
  
 // Temp Sensor initalise
  mlx.begin();
  SPI.begin();      // Initiate  SPI bus
  mfrc522.PCD_Init();   // Initiate MFRC522

// LED Pinouts 
  pinMode(inPin, INPUT);
  pinMode(outPin, OUTPUT);
  pinMode(outPin2, OUTPUT);
  pinMode(onboardLED, OUTPUT);

// Ethernet Mode initalisation (DISABLED)
  if (eth == true){

  /*Serial.begin(9600);
    Serial.println("ETH MODE START");
  // Open serial communications and wait for port to open:
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
    // start the Ethernet connection:
  Serial.println("Initialize Ethernet with DHCP:");
  if (Ethernet.begin(mac) == 0) {
    // Check for Ethernet hardware present
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
      while (true) {
        delay(1); // do nothing, no point running without Ethernet hardware
      }
    }
    
        if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("Ethernet cable is not connected.");
    }
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip, myDns);
  } else {
    Serial.print("  DHCP assigned IP ");
    Serial.println(Ethernet.localIP());
  }
  // give the Ethernet shield a second to initialize:
  delay(1000);
  Serial.print("connecting to ");
  Serial.print(server);
  Serial.println("...");

  // if you get a connection, report back via serial:
 if (client.connect(server, 51111)) {
    Serial.print("connected to ");
    Serial.println(client.remoteIP());
    // Make a HTTP request:


 } else {
    // if you didn't get a connection to the server:
 Serial.println("Ethernet connection failed");
 } 

 beginMicros = micros();
 */

  }
  else
  { 
  // Ethernet Mode initalisation (ENABLED)
  Serial.begin(9600);
  Serial.println("ESP MODE START");
  Serial.println("*****************************************************");
  Serial.println("********** Connect to WIFI");
  ESP.begin(9600);
  Serial.println("Initiate AT commands with ESP8266 ");
  sendATcmd("AT", 5, "OK");
  sendATcmd("AT+CWLAPOPT=1,8", 5, "OK");
  sendATcmd("AT+CWMODE=1", 5, "OK");
  Serial.print("Connecting to WiFi:");
  Serial.println(AP);
  sendATcmd("AT+CWJAP=\"" + AP + "\",\"" + PASS + "\"", 20, "OK");
  Serial.println("*****************************************************");
  Serial.println("********** RFID Start : ");
  }
}

void loop() {

static int loopcount = 0;
double temp_result = 0;
String rfid_result = ("0");
String ip_result = ("0");
String mac_result = ("0");
static int wdt_result = 0;

delay(200);

/* //ESP to ETH Mode fallback
 if (eth == false){
 
  if (loopcount >= 20){
    loopcount = 0;
    if (sendATcmd("AT", 5, "OK")){
    }
    else {
         Serial.println("SWITCHING TO ETH MODE");
    eth = true;
     ethConnect();
    delay(5000);
       // softReset();
  
      }
    }
  }*/

// Main loop executon
  rfid_result = getRFIDSensorData();
  if (rfid_result != "0") {
    digitalWrite(outPin, LOW);
    digitalWrite(outPin2, HIGH);
    delay(500);
    Serial.println("Awaiting Temp Scan");
    temp_result = getTempSensorData(); // Get Temperature Sensor Data
    Serial.println(temp_result); // Print Temperture Sensor Result
    ip_result = getGatewayIP(); // Get gatewayIP address
    mac_result = getNearbyMacs(); // Get nearby MAC addresses
    // Flash output once received 
    digitalWrite(outPin, HIGH);
    delay(100);
    digitalWrite(outPin, LOW);
    delay(100);
    digitalWrite(outPin, HIGH);
    delay(100);
    digitalWrite(outPin, LOW);
    delay(100);
    digitalWrite(outPin, HIGH);
    delay(100);
    digitalWrite(outPin, LOW);
    delay(100);
    digitalWrite(outPin, HIGH);
    delay(100);
    digitalWrite(outPin, LOW);
    delay(100);  
  sendDataPacket(rfid_result, temp_result, ip_result, mac_result); // SendDataPacket function
  }
  else {
    digitalWrite(outPin, HIGH);
    digitalWrite(outPin2, LOW);
   Serial.println("NO SCAN DETECTED");

 
 // Eth Reset timer (Disabled for ESP MODE)
    if (eth == true) {
 //   if (!client.connected()) {
  //  endMicros = micros();
 //   Serial.println();
 //   Serial.println("disconnecting.");
 //   client.stop();
//    delay(3000);
//    Serial.println("Rebooting ETH mode to reestablish connection");
//    delay(2000);
//    softReset(); 
//    eth = true;
//    }
  }
 // loopcount++; // Fallback loop
  }
}

// Send packet data 
void sendDataPacket(String result2, double result3, String result4, String result5) {
  
  digitalWrite(outPin2, HIGH); // Red LED on
  
  String result9 = "";
  String result10 = "";
  String result11 = "";

  result9 = "'" + result4 + "'";
  result10 = "'" + result2 + "'";
  result11 = "'" + result5 + "'";

  // Create the URL for the request
  String url = "GET /insert_db.php";

  url += "?rfid=";
  url += result10;
  url += "&temp=";
  url += result3;
  url += "&gateway_ip=";
  url += result9;
  url += "&maclist=";
  url += result10;
  //url += " HTTP/1.1";

// Disabled for ESP MODE
  if (eth == true){
    
 //   Make a ETH HTTP request - Requires ETH Initailisation. Currently disabled.
 //   Serial.println("*****************************************************");
 //   Serial.println("********** Open TCP connection ");
 //   Serial.println(url);
 //   Serial.println("Host: 139.180.186.167");
 //   client.println("Connection: close");
 //   client.println("Host: 139.180.186.167");
 //   client.println("Connection: close");
 //   client.println();

 //   Serial.println("********** Close TCP Connection ");
 //   Serial.println("*****************************************************");

eth == false;
  }

    else{
  // Send ESP Packet Data 
  sendATcmd("AT+CIPMUX=1", 15, "OK"); // Set minimum number of connectons
  digitalWrite(outPin2, LOW);
  sendATcmd("AT+CIPSTART=0,\"TCP\",\"" + HOST +"\"," + PORT, 20, "OK"); // Open TCP connection
  digitalWrite(outPin2, HIGH);
  sendATcmd("AT+CIPSEND=0," + String(url.length() + 4), 20, ">"); // Count the length and prepare to send
  Serial.println("********** requesting URL: ");
  Serial.println(url); // Print complete command to serial monitor screen
  ESP.println(url); // Issue command to the ESP8266
  delay(2000);
  digitalWrite(outPin2, LOW);
  sendATcmd("AT+CIPCLOSE=0", 10, "OK"); // Close the connection
  digitalWrite(outPin2, HIGH);
  Serial.println("********** Close TCP Connection ");
  Serial.println("*****************************************************");
  delay(2500);   // delay

  }
}

// Check RFID card is present at the module
String getRFIDSensorData()
{
  //RFID
  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent())
  {
    return "0";
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial())
  {
    return "0";
  }

  //Show UID on serial monitor
  Serial.print("UID tag :");
  String content = "";
  byte letter;
  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  Serial.println();
  Serial.print("Message : ");
  content.toUpperCase();

  //Remove all whitespace from RFID string
  String words = content.substring(1);
  char c;
  char no = ' '; //character I want removed.

    for (int i=0; i<words.length()-1;++i){
        c = words.charAt(i);
        if(c==no){
            words.remove(i, 1);
        }
    }
 // Check if RFID is valid  
   if (content.substring(1) == "B2 B8 E5 1C")
  {
    digitalWrite(onboardLED, HIGH);
    Serial.println("Authorized access");
    Serial.println(words);
    return words;
  }
  else   {
    digitalWrite(outPin, LOW);
    Serial.println(" Access denied");
    digitalWrite(outPin2, HIGH);
    delay(100);
    digitalWrite(outPin2, LOW);
    delay(100);
    digitalWrite(outPin2, HIGH);
    delay(100);
    digitalWrite(outPin2, LOW);
    delay(100);
    digitalWrite(outPin2, HIGH);
    delay(100);
    digitalWrite(outPin2, LOW);
    delay(100);
    delay(3000);
    return "0";
  }
  return "0";
}

// getTempSensorData
double getTempSensorData() {
  
  int tmp_result4 = 0;
  int tmp_result3 = mlx.readObjectTempC();
  Serial.println("Object Temp (C) ");
  Serial.println(tmp_result3);

  // Sample sensor data within pre-definied limits.
  while  (tmp_result4 <= temp_min){
    digitalWrite(outPin2, HIGH);
    tmp_result4 = mlx.readObjectTempC();
    delay(200);
    Serial.println(tmp_result4);
    digitalWrite(outPin2, LOW);  
  }
  return (tmp_result4*temp_sensor_adjust);
}

// get GatewayIP address
String getGatewayIP()
{
  ESP.println("AT+CIFSR");
  String result9 = waitForResponse("OK\r\n",1000,1);
  return result9; // Return result of waitFunction
} 

// get MAC address address list
String getNearbyMacs(){
  ESP.println("AT+CWLAP");
  String result3 = waitForResponse("OK\r\n",1000,0); 
  return result3; // Return result of waitFunction 
}


String waitForResponse(String target, unsigned long timeout, int flag)
{
  unsigned long startTime = millis();
  String responseBuffer;
  String responseBuffer1;
  String responseBuffer2;
  String responseBuffer3;
  String responseBuffer4 = "  ";
  char charIn;
 
  //keep checking for ESP response until timeout expires
 
    switch (flag){
      case 0: // MAC ADDRESS
    
     while ((millis() - startTime) < timeout)
  {
    responseBuffer = readLine();
    responseBuffer1 = responseBuffer.substring(9, 26);
    responseBuffer2 += responseBuffer1;
    responseBuffer2 += responseBuffer4;
    Serial.println(responseBuffer1);
    if (responseBuffer.equals(target))
    {
      //Serial.println(responseBuffer2);
      return responseBuffer2; // target string found
    }
    }
    break;
      case 1: //Gatway IP
       while ((millis() - startTime) < timeout)
       {
      responseBuffer = readLine();
      responseBuffer1 = responseBuffer.substring(14, 27);
      responseBuffer2 += responseBuffer1;
      Serial.print(responseBuffer);
       if (responseBuffer.equals(target))
       {
      //Serial.println(responseBuffer2);
      return responseBuffer2; // target string found
      }
       }
      break;
      
    default:
        while ((millis() - startTime) < timeout)
       {
      responseBuffer = readLine();
      responseBuffer1 += responseBuffer;
      
       if (responseBuffer.equals(target))
       {
      Serial.println(responseBuffer1);
      return responseBuffer1; // target string found
      }
       }
  
  }
  return; // could not find string before timeout
}

// Read each charcter from AT command response
String readLine()
{
  String lineRcvd = "";
  char espIn[200];
  int charIndex = 0;
  unsigned long readLineTime = millis();

  while (millis() - readLineTime < 10000) //allow up to 10 seconds to read line before timeout
  {
    while (ESP.available() > 0)
    {
      espIn[charIndex] = ESP.read();
      lineRcvd += espIn[charIndex];
      if (charIndex > 0)
      {
        if (espIn[charIndex-1] == '\r' && espIn[charIndex] == '\n') //look for carriage return/new line
       {
          //Serial.print("lineRcvd = ");
          return lineRcvd;
        }
      }
      charIndex++;
    }
  }
  Serial.println("readLine timed out!"); //if line not received within loop, timeout
  return "";
}

// sendATcmd
int sendATcmd(String AT_cmd, int AT_cmd_maxTime, char readReplay[]) {
  bool AT_cmd_result_tmp;
  cmd_result = 0;
  Serial.print("AT command:");
  Serial.println(AT_cmd);

// Issue command to ESP8266 while command time is less than maxTime set.
  while (AT_cmd_time < (AT_cmd_maxTime)) {
    ESP.println(AT_cmd);
    if (ESP.find(readReplay)) {
      AT_cmd_result = true;
      break;
    }
    // Loop until AT_cmd_time = AT_cmd_maxTime
    AT_cmd_time++;
  }
  Serial.print("...Result:");
  
  // Return true result if reply not found in response
  if (AT_cmd_result == true) {
    Serial.println("DONE");
    AT_cmd_time = 0;
    AT_cmd_result_tmp = AT_cmd_result;
    Serial.println(AT_cmd_result_tmp);
    cmd_result = 1;
    AT_cmd_result = false;
    return 1;
  }
  
  // Return false result if reply not found in response
  if (AT_cmd_result == false) {
    Serial.println("FAILED");
    AT_cmd_time = 0;
    AT_cmd_result_tmp = AT_cmd_result;
    Serial.println(AT_cmd_result_tmp);
    cmd_result = 0;
    AT_cmd_result = false;
    return 0;
  }

   AT_cmd_result = false;
}

// Software Reset
void softReset(){
asm volatile ("  jmp 0");
}