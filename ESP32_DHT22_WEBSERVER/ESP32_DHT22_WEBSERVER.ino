#include "WiFi.h"
#include <HTTPClient.h>
#include "DHTesp.h"
#include <HTTPClient.h>
#include "ESPAsyncWebServer.h"
#include <Adafruit_Sensor.h>
String url_base = "https://api.thingspeak.com/update?api_key=W8F4EN91XVCDJNJY";
AsyncWebServer server(80);
DHTesp dht;
int dhtPin = 4;
String readDHTTemperature() {
  float t = dht.getTemperature();

  if (isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return "--";
  }
  else {
    return String(t);
  }
}
String readDHTHumidity() {
  float h = dht.getHumidity();

  if (isnan(h)) {
    Serial.println("Failed to read from DHT sensor!");
    return "--";
  }
  else {
    Serial.println(h);
    return String(h);
  }
}
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <style>
    html {
     font-family: Arial;
     display: inline-block;
     margin: 0px auto;
     text-align: center;
    }
    h2 { font-size: 3.0rem; }
    p { font-size: 3.0rem; }
    .units { font-size: 1.2rem; }
    .dht-labels{
      font-size: 1.5rem;
      vertical-align:middle;
      padding-bottom: 15px;
    }
  </style>
</head>
<body>
  <h2>ESP32 DHT Server</h2>
  <p>
    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i> 
    <span class="dht-labels">Temperature</span> 
    <span id="temperature">%TEMPERATURE%</span>
    <sup class="units">&deg;C</sup>
  </p>
  <p>
    <i class="fas fa-tint" style="color:#00add6;"></i> 
    <span class="dht-labels">Humidity</span>
    <span id="humidity">%HUMIDITY%</span>
    <sup class="units">&percnt;</sup>
  </p>
  <h2>Temperature</h2>
    <iframe
        width="450"
        height="260"
        style="border:1px solid #cccccc;"
        src="https://thingspeak.mathworks.com/channels/3400208/charts/1?bgcolor=%23ffffff&color=%23d62020&dynamic=true&results=60&type=spline">
    </iframe>

    <h2>Humidity</h2>
    <iframe
        width="450"
        height="260"
        style="border:1px solid #cccccc;"
        src="https://thingspeak.mathworks.com/channels/3400208/charts/2?bgcolor=%23ffffff&color=%23d62020&dynamic=true&results=60&title=Humidity&type=spline&xaxis=Time&yaxis=H">
    </iframe>
</body>
<script>
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("temperature").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/temperature", true);
  xhttp.send();
}, 1000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("humidity").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/humidity", true);
  xhttp.send();
}, 1000 ) ;
</script>
</html>)rawliteral";

// Replaces placeholder with DHT values
String processor(const String& var){
  //Serial.println(var);
  if(var == "TEMPERATURE"){
    return readDHTTemperature();
  }
  else if(var == "HUMIDITY"){
    return readDHTHumidity();
  }
  return String();
}
void setup() {
  Serial.begin(115200);
  Serial.println("Setup done");
  dht.setup(dhtPin, DHTesp::DHT22);

  WiFi.begin("BKSTAR_T.302", "stemstar");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("delay 1000ms");
    delay(1000);
  }
  Serial.println("Wifi Connected!");
  Serial.println(WiFi.localIP());
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readDHTTemperature().c_str());
  });
  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readDHTHumidity().c_str());
  });
  server.begin();
}


void loop() {
TempAndHumidity newValues = dht.getTempAndHumidity();
  if (dht.getStatus() != 0) {
    Serial.println("DHT22 error status: " + String(dht.getStatusString()));
  }
  float t = newValues.temperature;
  float h = newValues.humidity;
  HTTPClient http;
  String url_thingspeak = url_base + "&field1=" + String(t) + "&field2=" + String(h);
  http.begin(url_thingspeak);  
  int httpCode = http.GET();
  if (httpCode > 0) {
    Serial.printf("[HTTP] GET... code: %d\n", httpCode);
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      Serial.println(payload);
    }
  } else {
    Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }
  http.end();
  delay(1000);

}
