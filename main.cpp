#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MLX90614.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <MFRC522.h>

#define DEBUG true

#define RX 5
#define TX 6

#define SS_PIN 4
#define RST_PIN 2

SoftwareSerial ESP(RX, TX);
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

int onboard = 13;

//String AP = "gogo-wifi2";       // CHANGE ME
//String PASS = ""; // CHANGE MEString API = "YOUR_API_KEY";   // CHANGE ME
String AP = "vivo1904";       // CHANGE ME
String PASS = ""; // CHANGE MEString API = "YOUR_API_KEY";   // CHANGE ME
String HOST = "";
String PORT = "80";

int countTrueCommand;
int countTimeCommand;
boolean found = false;
int temp_result = ("TEST10");
String rfid_result = ("TEST1");
String ip_result = ("TEST2");
String mac_result = ("TEST3");
String timestamp_result = ("TEST4");

String url_prefix = ("/index.php?temp=");
String url_prefix_2 = ("&rfid=");
String url_prefix_3 = ("&ip=");
String url_prefix_4 = ("&maclist=");
String url_prefix_5 = ("&timestamp=");


int ardprintf(char *, ...);

Adafruit_MLX90614 mlx = Adafruit_MLX90614();

void setup() {
  Serial.begin(9600);
  ESP.begin(9600);

  ESP.println("AT+CWLAPOPT=1,8");

  delay(500);
  ESP.println("AT+CWMODE=1");
  delay(500);
  ESP.println("AT+CWJAP=\"" + AP + "\",\"" + PASS + "\"");
  delay(1000);
  Serial.println("Initialised...");
  //Serial.println(getLocalIP());
  delay(500);
  // Temp Sensor
  mlx.begin();
  SPI.begin();      // Initiate  SPI bus
  mfrc522.PCD_Init();   // Initiate MFRC522
  system("clear");
  delay(500);
  Serial.println("Approximate your card to the reader...");
  Serial.println();
  digitalWrite(onboard, HIGH);
  delay(500);
}

void loop() {
//  Serial.println(result10);
  delay(500);
  rfid_result = getRFIDSensorData();
  delay(500);
  if (rfid_result != "0") {
    Serial.println("SUCCESSFUL SCAN!");
    Serial.println("Result1");
    ip_result = getGatewayIP();
    mac_result = getNearbyMacs();
    Serial.println("Scanning temp..");
    //temp_result = getTempSensorData();'
    temp_result = 3;
    timestamp_result = "test4";
    sendDataPacket(timestamp_result, temp_result, rfid_result, ip_result, mac_result);
  }
  else {
    Serial.println("NO SCAN DETECTED");

  }

  while (ESP.available()) {
    Serial.write(ESP.read());
  }
  while (Serial.available()) {
    char serIn = Serial.read();
    switch (serIn) {
      case 'c': //Connect
        Serial.println(serIn);
        Serial.println("SEND DATA");
        //Serial.println(result1);
        //String result2 = getNearbyMacs();
        break;
      default:
        ESP.print(serIn);


    }
  }
}





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
  String tmp_result4 = content.substring(1);

  if (content.substring(1) == "EB 9B 5F 0A") //change here the UID of the card/cards that you want to give access
  {
    digitalWrite(onboard, LOW);
    Serial.println("Authorized access");
    return tmp_result4;
  }
  else   {
    Serial.println(" Access denied");
    delay(3000);
    return "0";
  }
  return "0";
}


int getTempSensorData() {
  // Temp Sensor
  //Serial.print("Ambient = ");
  //Serial.print(mlx.readAmbientTempC());
  Serial.print("*C\tObject = ");
  int tmp_result3 = mlx.readObjectTempC();
  Serial.print(tmp_result3);
  return tmp_result3;
}


void sendDataPacket(String result1, int result2, String result3, String result4, String result5) {

  String temp1 = result1;
  int temp2 = result2;
  String temp3 = result3;
  String temp4 = result4;
  String temp5 = result5;
  //String temp4 = "34";
  //String temp5 = "35";
  //String url = ();
  //String url2 = ("&rfid=");
  String getData2 = (String("PUT ") + "/insert_db.php?timestamp=" + temp1 + "&temp=" + temp2 + "&rfid=" + temp3 + "&gateway_ip=" + temp4 + "&maclist=" + temp5 + " HTTP/1.1\r\n"
                     + "Host: " + HOST + "\r\n"
                     + "User-Agent: BuildFailureDetectorESP8266\r\n"
                     + "Connection: close\r\n\r\n");

  sendCommand("AT+CIPMUX=1", 5, "OK");
  sendCommand("AT+CIPSTART=0,\"TCP\",\"139.180.186.167\",80", 5, "OK");
  sendCommand("AT+CIPSEND=0," + String(getData2.length() + 4), 10, ">");
  delay(500);
  ESP.print(getData2);
  Serial.println(getData2);
  delay(2000);
  sendCommand("AT+CIPCLOSE=0", 5, "OK");
  //sendCommand("AT+CIPSHUT=0",5,"OK");
  Serial.println(" Sent");
  return;
}


String getGatewayIP()
{
  //runCommand("AT+CIFSR\r\n");
  delay(250);
  ESP.println("AT+CIPSTA_CUR?");
  String result9 = waitForResponse("OK\r\n", 1000, 0);
  
  return result9;
}

String getNearbyMacs() {
  delay(500);
  ESP.println("AT+CWLAP");
  String result3 = waitForResponse("OK\r\n", 1000, 0);
  return result3;
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

  switch (flag) {
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
    case 1: //Local IP
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
        if (espIn[charIndex - 1] == '\r' && espIn[charIndex] == '\n') //look for carriage return/new line
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

String runCommand(String command)
{
  ESP.print(command);
  delay(100);
  return command;
}


void sendCommand(String command, int maxTime, char readReplay[]) {
  String data;
  char a;
  char response;
  Serial.print(countTrueCommand);
  Serial.print(". at command => ");
  Serial.print(command);
  Serial.print(" ");
  while (countTimeCommand < (maxTime * 1))
  {
    ESP.println(command);//at+cipsend

    if (ESP.find(readReplay)) //ok
    {
      found = true;
      break;
    }
    countTimeCommand++;
  }

  if (found == true)
  {
    Serial.println("OK");
    countTrueCommand++;
    countTimeCommand = 0;
  }

  if (found == false)
  {
    Serial.println("Fail");
    countTrueCommand = 0;
    countTimeCommand = 0;
  }

  found = false;
  Serial.println(response);
}
