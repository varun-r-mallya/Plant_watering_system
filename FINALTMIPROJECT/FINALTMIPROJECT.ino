//google sheets doc: https://docs.google.com/spreadsheets/d/1zIdt5UI3NgEUyF7FMaQC0UFEQJ902NfOVyP7DmJWO28/edit#gid=0
// sheet1 is where values are stored
// sheet2 is where ON/OFF is done cell A2 is used
// https://www.youtube.com/watch?v=gVcITjeVCCk refer for wiring analog sensor multiplexer
// pin D0 is for relay output
// pin D1 is for VCC of water level sensor
// pin D3 is for VCC of moisture sensor
// pin A0 is connection of outputs of analog sensors after passing through diodes
// pin 3.3v and GND is for 5v relay input and for sensor voltage inputs
// adjust AirValue and WaterValue variables as and when the sensors are added
// calibrate the temperature sensor, the moisture sensor and the water level sensors using potentiometers as an output bridge.
#include <DebugMacros.h>
#include <HTTPSRedirect.h>
#include <DHT.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "HTTPSRedirect.h"
#include "DHT.h"
#define DHTPIN 4  //D2 on board
#define DHTTYPE DHT11   // check version of humidity sensor and put accordingly
DHT dht(DHTPIN, DHTTYPE); //initialise DHT
// Enter network credentials:
const char* ssid     = "vrm";
const char* password = "passcode";

// Enter Google Script Deployment ID:
const char *GScriptId = "AKfycbyYVzEtag3hRUMOFaFhfZf_sAQ6RrhTjMQz6gtVodofOxobqwf6yu7m9HFyS-EDYAq9TA";
const char *cellAddress = "A2";
const char *cellAddress1 = "B2";
// Enter command (insert_row or append_row) and your Google Sheets sheet name (default is Sheet1):
String payload_base =  "{\"command\": \"append_row\", \"sheet_name\": \"sheet1\", \"values\": ";
String payload = "";

// Google Sheets setup (do not edit)
const char* host = "script.google.com";
const int httpsPort = 443;
const char* fingerprint = "";
String url = String("/macros/s/") + GScriptId + "/exec";
String url3 = String("/macros/s/") + GScriptId + "/exec?read=" + cellAddress;
String url4 = String("/macros/s/") + GScriptId + "/exec?read=" + cellAddress1;
HTTPSRedirect* client = nullptr;

// Declare variables that will be published to Google Sheets
float value0 = 0;
int value1 = 0;
int value2 = 0;
int readD1 = 0;
int readD2 = 0;
const int AirValue = 790;   //you need to replace this value with Value_1
const int WaterValue = 390;  //you need to replace this value with Value_2
int soilmoisturepercent=0;
int Pin_D1 = 5;
int Pin_D3 = 0;


void setup() {

  pinMode(Pin_D1,OUTPUT);
  pinMode(Pin_D3,OUTPUT);
  pinMode(A0,INPUT);
  pinMode(16, OUTPUT);
  digitalWrite(16, HIGH);
  Serial.begin(9600);        
  delay(10);
  Serial.println('\n');
  dht.begin();
  // Connect to WiFi
  WiFi.begin(ssid, password);             
  Serial.print("Connecting to ");
  Serial.print(ssid); Serial.println(" ...");

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println('\n');
  Serial.println("Connection established!");  
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());

  // Use HTTPSRedirect class to create a new TLS connection
  client = new HTTPSRedirect(httpsPort);
  client->setInsecure();
  client->setPrintResponseBody(true);
  client->setContentTypeHeader("application/json");
  
  Serial.print("Connecting to ");
  Serial.println(host);

  // Try to connect for a maximum of 5 times
  bool flag = false;
  for (int i=0; i<5; i++){ 
    int retval = client->connect(host, httpsPort);
    if (retval == 1){
       flag = true;
       Serial.println("Connected");
       break;
    }
    else
      Serial.println("Connection failed. Retrying...");
  }
  if (!flag){
    Serial.print("Could not connect to server: ");
    Serial.println(host);
    return;
  }
  delete client;    // delete HTTPSRedirect object
  client = nullptr; // delete HTTPSRedirect object
  
}


void loop() {

  //D1 is connected to VCC of water level sensor and D3 is connected to VCC of moisture sensor
  //the outputs of these are connected to a diode each and the negative end of diode faces arduino
  //the diodes are connected to A0 of esp8266
  float t = dht.readTemperature();
    digitalWrite(Pin_D1, HIGH);     //Turn D1 On
  digitalWrite(Pin_D3, LOW);
  delay(100);                     
  readD1 = analogRead(0);       //Read Analog value of first sensor
  delay(100);   
 
  digitalWrite(Pin_D1, LOW); 
  digitalWrite(Pin_D3, HIGH);     //Turn D2 On
  delay(100);                     
  readD2 = analogRead(0); //Read Analog value of second sensor
  delay(100);
  soilmoisturepercent = map(readD2, AirValue, WaterValue, 0, 100);
  value0 = readD1 ; //water level
  value1 = t ; //temperature
  value2 = soilmoisturepercent; //moisture level


  static bool flag = false;
  if (!flag){
    client = new HTTPSRedirect(httpsPort);
    client->setInsecure();
    flag = true;
    client->setPrintResponseBody(true);
    client->setContentTypeHeader("application/json");
  }
  if (client != nullptr){
    if (!client->connected()){
      client->connect(host, httpsPort);
    }
  }
  else{
    Serial.println("Error creating client object!");
  }
  
  Serial.print("GET Data from cell: ");
  Serial.println(cellAddress);
  if (client->GET(url3, host)){
    //get the value of the cell
    payload = client->getResponseBody();
    payload.trim();
    Serial.println(payload);
    if (payload == "on" || payload == "On" || payload == "ON" || payload == "1" || payload == "true" || payload == "TRUE") {
      digitalWrite(16, LOW);
    } else {
      digitalWrite(16, HIGH);
    }
    payload = "";
  }
  Serial.print("GET Moisture from cell: ");
  Serial.println(cellAddress1);
  if (client->GET(url4, host)){
    //get the value of the cell
    payload = client->getResponseBody();
    payload.trim();
    Serial.println(payload);
    int Temporal =  (payload.toInt());
    if(Temporal != 0)
    {
    if (soilmoisturepercent<=Temporal) {
      Serial.print("Moisture payload and actual soil moisture: ");
      Serial.println(Temporal);
      Serial.println(soilmoisturepercent);
      digitalWrite(16, LOW);
    } else {
      digitalWrite(16, HIGH);
    }
    }
    else
    Serial.print("Moisture returned 0");
    payload = "";
  }

  
  // Create json object string to send to Google Sheets
  payload = payload_base + "\"" + value0 + "," + value1 + "," + value2 + "\"}";
  
  // Publish data to Google Sheets
  Serial.println("Publishing data...");
  Serial.println(payload);
  if(client->POST(url, host, payload)){ 
    // do stuff here if publish was successful
  }
  else{
    // do stuff here if publish was not successful
    Serial.println("Error while connecting");
  }

  // a delay of several seconds is required before publishing again    
  delay(3000);
}
