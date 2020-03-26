#include <ESP8266WiFi.h>

#ifndef STASSID
#define STASSID "RED-VALAREZO1"
#define STAPSK  "clavered1"
#endif

void setup_wifi(){
  delay(10);
  Serial.println();
  Serial.print("Conectando a ");
  Serial.println(STASSID);
  WiFi.begin(STASSID, STAPSK);
  
  while (WiFi.waitForConnectResult()  != WL_CONNECTED) {    // se espera indefinidamente hasta que exista coneccion
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("Conectado a red WiFi!");
  Serial.println("Direccion IP: ");
  Serial.println(WiFi.localIP());
}
