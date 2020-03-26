 #include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <RCSwitch.h>
#include "config.h"


const int pin_led = LED_BUILTIN;              // pin al cual se conecta el LED
int estado_led = HIGH;
int t_muestreo = 5000;        

const char *mqtt_server = "pccontrol.ml";
const int mqtt_port = 1883;
const char *mqtt_user = "web_client";
const char *mqtt_pass = "121212";
String clientId = "esp32_C";
char * variable_id = "led1";

WiFiClient espClient;
PubSubClient client(espClient);


// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Auxiliar variables to store the current output state
String output5State = "off";
String output4State = "off";
String output3State = "off";
String output2State = "off";

// Assign output variables to GPIO pins
const int output5 = 5;
const int output4 = 4;
const int output3 = 3;
const int output2 = 2;
// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;


long last_msg = 0;
char msg[25];

// ------------------------- SE DECLARAN LAS FUNCIONES -------------------------
void setup_wifi();
void callback(char* topic, byte* payload, unsigned int length);
void reconnect();


// ----------------------------------- SETUP -----------------------------------

void setup() {
  Serial.begin(115200);
  pinMode(pin_led, OUTPUT);                   // se inicializa el led del esp 
  digitalWrite(pin_led, estado_led);
  randomSeed(micros());
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);   // se define el servidor mqtt
  client.setCallback(callback);               // se define el callback que se levanta al recibir informacion del servidor

  // Initialize the output variables as outputs
  pinMode(output5, OUTPUT);
  pinMode(output4, OUTPUT);

  // Set outputs to LOW
  digitalWrite(output5, LOW);
  digitalWrite(output4, LOW);


  ArduinoOTA.setHostname("PCCONTROL-IOT");
  ArduinoOTA.setPassword("CLAVEPARAOTA");
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { 
      type = "filesystem";
    }
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());


    Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
  
}


// ------------------------------ CICLO PRINCIPAL ------------------------------
void loop() {
  if (!client.connected()) {
    reconnect();
  }

  client.loop();

  long now = millis();
  if (now - last_msg > t_muestreo){
    last_msg = now;
    String to_send = String(estado_led);
    to_send.toCharArray(msg, 25);
    Serial.print("Publicamos mensaje: ");
    Serial.println(msg);
    client.publish("values", msg);
  }


    WiFiClient client1 = server.available();   // Listen for incoming clients

  if (client1) {                             // If a new client connects,
  //  Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    currentTime = millis();
    previousTime = currentTime;
    while (client1.connected() && currentTime - previousTime <= timeoutTime) { // loop while the client's connected
      currentTime = millis();         
      if (client1.available()) {             // if there's bytes to read from the client,
        char c = client1.read();             // read a byte, then
 //       Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client1.println("HTTP/1.1 200 OK");
            client1.println("Content-type:text/html");
            client1.println("Connection: close");
            client1.println();
            
            // turns the GPIOs on and off
            if (header.indexOf("GET /5/on") >= 0) {
              Serial.println("GPIO 5 on");
              Serial.println(1);
              output5State = "on";
              digitalWrite(output5, HIGH);
            } else if (header.indexOf("GET /5/off") >= 0) {
              Serial.println("GPIO 5 off");
               Serial.println(2);
              output5State = "off";
              digitalWrite(output5, LOW);
            } else if (header.indexOf("GET /4/on") >= 0) {
              Serial.println("GPIO 4 on");
              output4State = "on";
              digitalWrite(output4, HIGH);
            } else if (header.indexOf("GET /4/off") >= 0) {
         //     Serial.println("GPIO 4 off");
              output4State = "off";
              digitalWrite(output4, LOW);
            }
            
            // Display the HTML web page
            client1.println("<!DOCTYPE html><html>");
            client1.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client1.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client1.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client1.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;");
            client1.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client1.println(".button2 {background-color: #77878A;}</style></head>");
            
            // Web Page Heading
            client1.println("<body><h1>OpenVenti</h1>");
            
            // Display current state, and ON/OFF buttons for GPIO 5  
            client1.println("<p>Ventilador " + output5State + "</p>");
            // If the output5State is off, it displays the ON button       
            if (output5State=="off") {
              client1.println("<p><a href=\"/5/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client1.println("<p><a href=\"/5/off\"><button class=\"button button2\">OFF</button></a></p>");
            } 
               
            // Display current state, and ON/OFF buttons for GPIO 4  
            client1.println("<p>Volume - State " + output4State + "</p>");
            // If the output4State is off, it displays the ON button       
            if (output4State=="off") {
              client1.println("<p><a href=\"/4/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client1.println("<p><a href=\"/4/off\"><button class=\"button button2\">OFF</button></a></p>");
            }
            client1.println("</body></html>");
            
            // The HTTP response ends with another blank line
            client1.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client1.stop();
  //  Serial.println("Client disconnected.");
  //  Serial.println("");
  }

  
}


// --------------------------------- FUNCIONES ---------------------------------
void callback(char* topic, byte* payload, unsigned int length){
  String incoming = "";
  Serial.print("Mensaje recibido desde: ");
  Serial.println(topic);
  for (int i = 0; i < length; i++) {      // se itera cada byte y se lo convierte en char y posteriormente se concatena
    incoming += (char)payload[i];
  }
  incoming.trim();
  Serial.println("Mensaje: " + incoming);
  if ( incoming == "on") {
    estado_led = LOW;
    digitalWrite(pin_led, estado_led); 
  } else {
    estado_led = HIGH;
    digitalWrite(pin_led, estado_led); 
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Intentando reconectar con el servidor MQTT...");
    if (client.connect(clientId.c_str(),mqtt_user,mqtt_pass)) {
      Serial.println("Conectado!");
      client.subscribe(variable_id);
    } else {
      Serial.print("Algo ha pasado, error: ");
      Serial.print(client.state());
      Serial.println("Reintentamos en 5 segundos");
      delay(2000);
    }
  }
}
