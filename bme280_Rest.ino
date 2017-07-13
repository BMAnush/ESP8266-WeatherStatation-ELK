#include <Wire.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <time.h>


Adafruit_BME280 bme; // I2C

// Replace with your network details
const char* ssid = "Airtel-WD670-0857";
const char* password = "D8F20857";
float h, t, p, pin, dp, dc;
float temperatureFString;
char dpString[6];
float dcString;
char humidityString[6];
char pressureString[7];
char pressureInchString[6];
char* currentTime;

// Web Server on port 80
WiFiServer server(80);

// only runs once on boot
void setup() {
  // Initializing serial port for debugging purposes
  Serial.begin(115200);
  delay(10);
  Wire.begin(D3, D4);
  Wire.setClock(100000);
  // Connecting to WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  
  // Starting the web server
  server.begin();
  Serial.println("Web server running. Waiting for the ESP IP...");
  delay(10);
  
  // Printing the ESP IP address
  Serial.println(WiFi.localIP());
  Serial.println(F("BME280 test"));

 // Get Current Time for web
  configTime(19800, 0, "pool.ntp.org", "time.nist.gov");
  Serial.println("\nWaiting for time");
  while (!time(nullptr)) {
    Serial.print(".");
    delay(1000);
  }
  if (!bme.begin()) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }
}

void getWeather() {
  
    h = bme.readHumidity();
    t = bme.readTemperature();
    t = t*1.8+32.0;
    dp = t-0.36*(100.0-h);
    dc = (t - 32) * 0.55;
    
    p = bme.readPressure()/100.0F;
    pin = 0.02953*p;
    //dtostrf(t, 5, 1, temperatureFString);
    dtostrf(h, 5, 1, humidityString);
    dtostrf(p, 6, 1, pressureString);
    dtostrf(pin, 5, 2, pressureInchString);
    dtostrf(dp, 5, 1, dpString);
    //dtostrf(dc, 5, 1, dcString);
    Serial.println("Temperature Data");
    temperatureFString = t;
    dcString = dc;
    Serial.println(t);
    Serial.println(temperatureFString);   
    delay(10); 
}

void sendDataPost(){
  
    if(WiFi.status()== WL_CONNECTED){   //Check WiFi connection status
        HTTPClient http;    //Declare object of class HTTPClient
        http.begin("http://54.218.99.244:9200/elkweatherstation1/mechaniq");      //Specify request destination
        http.addHeader("Content-Type", "application/json");    //Specify content-type header

        StaticJsonBuffer<200> jsonBuffer;
        JsonObject& root = jsonBuffer.createObject();
        root["sensor"] = "bme280";
        root["currentTime"] = currentTime;
        root["temperatureFahrenhiet"] = temperatureFString;
        root["temperatureCelcius"] = dcString;
        root["dewPoint"] = dpString;
        root["humidity"] = humidityString;
        root["pressure"] = pressureString;
        
        String output;
        root.printTo(output);
        
        int httpCode = http.POST(output);   //Send the request
        String payload = http.getString();  //Get the response payload
        Serial.println(httpCode);   //Print HTTP return code
        Serial.println(payload);    //Print request response payload
        http.end();  //Close connection
    }else{
        Serial.println("Error in WiFi connection");   
    }
}

// runs over and over again
void loop() {
    time_t now = time(nullptr) + 1800;
    currentTime = ctime(&now);
   
    Serial.println(currentTime);
    Serial.println("New Send Request...");
    // Get Weather Data
    getWeather();
    // Post Data
    sendDataPost();
    delay(600000);
    //client.stop();
    Serial.println("Client disconnected.");
}



