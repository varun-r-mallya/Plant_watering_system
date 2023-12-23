#include <DebugMacros.h>
#include <HTTPSRedirect.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "HTTPSRedirect.h"
// Enter network credentials:
const char* ssid     = "<add code here>"; //wifi userid
const char* password = "<add code here>"; //wifi password

// Enter Google Script Deployment ID:
const char *GScriptId = "<add code here>";
const char *cellAddress = "A1";
// Enter command (insert_row or append_row) and your Google Sheets sheet name (default is Sheet1):
String payload_base =  "{\"command\": \"append_row\", \"sheet_name\": \"sheet1\", \"values\": ";
String payload = "";

// Google Sheets setup (do not edit)
const char* host = "script.google.com";
const int httpsPort = 443;
const char* fingerprint = "";
String url = String("/macros/s/") + GScriptId + "/exec";
HTTPSRedirect* client = nullptr;

// Declare variables that will be published to Google Sheets
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
  if (client->GET(url, host)){
    //get the value of the cell
    payload = client->getResponseBody();
    payload.trim();
    Serial.println(payload);
    if (payload == "on" || payload == "On" || payload == "ON" || payload == "1" || payload == "true" || payload == "TRUE") {
      digitalWrite(16, LOW);
    } else {
      digitalWrite(16, HIGH);
    }
  }
  else{
    Serial.println("Error getting response");
  }
  delay(3000);
}
