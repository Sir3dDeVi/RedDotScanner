#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#include <Adafruit_NeoPixel.h>


#define LED_PIN D8
#define LEDS_NUM 3
#define WIFI_SSID "*****"
#define WIFI_PASS "*****"

// https://medium.com/@hauyang/convert-int-into-string-with-c-macro-125eeaa71600
#define STRINGIFY_INDIR(x) #x
#define STRINGIFY(x) STRINGIFY_INDIR(x)


Adafruit_NeoPixel pixels(LEDS_NUM, LED_PIN, NEO_GRB + NEO_KHZ800);
ESP8266WebServer server(80);

void handleRoot() {
  server.send(200, "text/html",
    "<h1>Leveler - " STRINGIFY(LEDS_NUM) " LEDs</h1>\r\n"
    "<a href=\"/led?led=0&r=255&g=255&b=255\">/led?led=0&#x26;r=255&#x26;g=255&#x26;b=255</a>\r\n"
  );
}

void handleLed() {
  int led = server.arg("led").toInt();
  int r = server.arg("r").toInt();
  int g = server.arg("g").toInt();
  int b = server.arg("b").toInt();

  pixels.setPixelColor(led, r, g, b);
  pixels.show();

  server.send(200, "text/plain", "ok");
}

void handleNotFound() {
  server.send(404, "text/html", "<h1>404 Not Found</h1>");
}

void setup() {
  Serial.begin(115200);
  
  // Init NeoPixels
  pixels.begin();
  pixels.clear();
  pixels.show();

  // Start connecting to WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  // Wait for connection
  bool blinking = false;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);

    for (uint16_t i = 0; i < LEDS_NUM; ++i)
      pixels.setPixelColor(i, 0, 0, blinking ? 255 : 0);
    pixels.show();

    blinking = !blinking;
  }

  // Now we are connected
  pixels.clear();
  pixels.show();
  
  Serial.println();
  Serial.print("Connected to ");
  Serial.println(WIFI_SSID);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("leveler")) {
    Serial.println("MDNS responder started");
  }

  // Configure web server
  server.on("/", handleRoot);
  server.on("/led", handleLed);
  server.onNotFound(handleNotFound);

  server.begin();

  // Indicate that the leveler is initialized
  for (uint16_t i = 0; i < LEDS_NUM; ++i)
    pixels.setPixelColor(i, 0, 255, 0);
  pixels.show();
  delay(500);

  pixels.clear();
  pixels.show();
}

void loop() {
  server.handleClient();
  MDNS.update();
}
