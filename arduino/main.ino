#include "DHT.h"
#include <Adafruit_Sensor.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

// defineerime andurite asukoha ja tyybi
#define DHTPIN 0
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE); // loome anduri klassi

const char* ssid = "wifi_name"; // siia tuleb lisada wifi nimi
const char* password = "wifi_password"; // siia tuleb lisada wifi parool
String serverName = "http://192:168:8:106:8080/"; // siia tuleb serveri asukoht, node server jookseb lokaalselt pordis 8080

const int ledPin = 13;        

WiFiClient client; // loove wifi client klassi
HTTPClient http; // loome http klassi

void setup() {
    // alustame seriali baud tasemel 9600
    Serial.begin(9600);
    Serial.setTimeout(2000);

    // ootame millal serial algab
    while (!Serial) { }

    WiFi.begin(ssid, password); // alusta wifi paketiga
    
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    
    Serial.print("Wifi connected");
    delay(1000);
    dht.begin(); // alusta temperatuuri andrui paketiga
    delay(1000);

    // set up on l2bi  
    Serial.println("Set up has been completed");
    Serial.println("-------------------------------------");
}

void loop() {
    String sensorStatusRequest = serverName + "/sensor/status"; //API url: localhost:8080/sensor/status tagastab sensori staatuse
    http.begin(client, sensorStatusRequest.c_str());
    int sensorStatusRequestCode = http.GET(); // teeme serverisse p2ringu

    // p2ring tehti edukalt
    if (sensorStatusRequestCode > 0) {
       String sensorStatusPayload = http.getString();
        int sensorStatus = sensorStatusPayload.toInt();
        // sensorStatus === 0 => siis ei hakka pealt kuulama temperatuuri
        // sensorStatus === 1 => hakkame temperatuuri pealt kuulama

        if (sensorStatus == 1) {
           digitalWrite(13, HIGH); // paneme led tule p6lema

            // saame temperatuuri ja niiskuse DHT sensorilt
            float h = dht.readHumidity();
            float t = dht.readTemperature();

            // lugemine eba6nnestus, testimise eesm2rgil saadame suvalised andmed serverisse
            if (isnan(h) || isnan(t)) {
                Serial.println("Failed to read from DHT sensor, sending out mock data!");
                String sensorDataRequest = serverName + "sensor/tempHum/20/40"; //API url: localhost:8080/sensor/hum/{temperature}/{humdity} saadab andmed serverisse
                http.begin(client, sensorDataRequest.c_str());

                int httpResponseCode = http.GET();
                if(httpResponseCode > 0){
                  Serial.println("Data was sent sucessfully!");
                } else {
                  Serial.println("Sending out data failed");
                }
                return;
            }

            // andmed saati korrektselt k2tte sensorilt, ning saadame need serverisse
            String sensorDataRequest = serverName + "sensor/tempHum" + String(t) + "/" + String(h); //API url: localhost:8080/sensor/hum/{temperature}/{humdity} saadab andmed serverisse
            http.begin(client, sensorDataRequest.c_str());

            int httpResponseCode = http.GET();
            if(httpResponseCode > 0){
              Serial.println("Data was sent sucessfully!");
            } else {
              Serial.println("Sending out data failed");
            }
        } else {
            // sensori lugemine ei ole lubatud
            Serial.println("Not permitted to start listening for temperature");
            return;
        }


    } else {
        // p2ring eba6nnestus
        Serial.print("Error code: ");
        Serial.println(sensorStatusRequestCode);
        return;
    }


    // end of loop
    http.end();
    delay(2000);
}