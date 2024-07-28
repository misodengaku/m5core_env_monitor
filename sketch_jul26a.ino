/**
 * @file Unit_ENVIII_M5Core2.ino
 * @author SeanKwok (shaoxiang@m5stack.com)
 * @brief
 * @version 0.1
 * @date 2024-01-30
 *
 *
 * @Hardwares: M5Core2 + Unit ENVIII
 * @Platform Version: Arduino M5Stack Board Manager v2.1.0
 * @Dependent Library:
 * M5UnitENV: https://github.com/m5stack/M5Unit-ENV
 * M5Unified: https://github.com/m5stack/M5Unified
 */

#include <M5Unified.h>
#include "M5UnitENV.h"
#include <WiFi.h>
#include <WebServer.h>
#include "wifi_pass.h"

#define I2C_SDA 2
#define I2C_SCL 1

WebServer server(80);
IPAddress ip(192, 168, 88, 10);
IPAddress gateway(192, 168, 88, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns(1, 1, 1, 1);
String header;

void getMetric();

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0;
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

char metric[256];

SHT3X sht3x;
QMP6988 qmp;

void setup()
{
  Serial.begin(115200);
  Serial.println("sta");

  WiFi.disconnect();
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_STA);
  if (WiFi.config(ip, gateway, subnet, dns, dns) == false)
  {
    Serial.println("Configuration failed.");
  }
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  Serial.println("disc");
  delay(100);

  Serial.println("wait for SHT3x...");
  if (!sht3x.begin(&Wire, SHT3X_I2C_ADDR, I2C_SDA, I2C_SCL, 400000U))
  {
    Serial.println("Couldn't find SHT3X");
    while (1)
      delay(1);
  }
  Serial.println("wait for QMP6988...");
  if (!qmp.begin(&Wire, QMP6988_SLAVE_ADDRESS_L, I2C_SDA, I2C_SCL, 400000U))
  {
    Serial.println("Couldn't find QMP6988");
    while (1)
      delay(1);
  }

  Serial.print("wait for wifi...");
  // Try forever
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(100);
  }
  Serial.println("ok.");
  Serial.println("\nConnected to the WiFi network");
  Serial.print("Local ESP32 IP: ");
  Serial.println(WiFi.localIP());

  server.on("/metrics", HTTP_GET, getMetric);
  server.begin();
}

void loop()
{
  if (sht3x.update())
  {
    // Serial.println("-----SHT3X-----");
    // Serial.print("Temperature: ");
    // Serial.print(sht3x.cTemp);
    // Serial.println(" degrees C");
    // Serial.print("Humidity: ");
    // Serial.print(sht3x.humidity);
    // Serial.println("% rH");
    // Serial.println("-------------\r\n");
  }

  if (qmp.update())
  {
    // Serial.println("-----QMP6988-----");
    // Serial.print(F("Temperature: "));
    // Serial.print(qmp.cTemp);
    // Serial.println(" *C");
    // Serial.print(F("Pressure: "));
    // Serial.print(qmp.pressure);
    // Serial.println(" Pa");
    // Serial.print(F("Approx altitude: "));
    // Serial.print(qmp.altitude);
    // Serial.println(" m");
    // Serial.println("-------------\r\n");
  }
  server.handleClient();
  delay(200);
}

void getMetric()
{
  server.sendHeader("Cache-Control", "no-cache");
  sprintf(metric, "env_monitor_temperature %.02f\nenv_monitor_humidity %.02f\nenv_monitor_pressure %.02f\n", sht3x.cTemp, sht3x.humidity, qmp.pressure);
  server.send(200, "text/plain", metric);
}
