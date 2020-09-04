#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <PMserial.h>

#define MQTT_TOPIC_PM01 "home/sensors/livingroom/air/pm01"
#define MQTT_TOPIC_PM25 "home/sensors/livingroom/air/pm25"
#define MQTT_TOPIC_PM10 "home/sensors/livingroom/air/pm10"
#define MQTT_TOPIC_N0P3 "home/sensors/livingroom/air/n0p3"
#define MQTT_TOPIC_N0P5 "home/sensors/livingroom/air/n0p5"
#define MQTT_TOPIC_N1P0 "home/sensors/livingroom/air/n1p0"
#define MQTT_TOPIC_N2P5 "home/sensors/livingroom/air/n2p5"
#define MQTT_TOPIC_N5P0 "home/sensors/livingroom/air/n5p0"
#define MQTT_TOPIC_N10P0 "home/sensors/livingroom/air/n10p0"
#define MQTT_TOPIC_STATE "home/iot/status/esp8266pms-livingroom"
#define MQTT_PUBLISH_DELAY 10000

const char* MQTT_CLIENT_ID = "esp8266pms-livingroom";
const char* WIFI_SSID = "WIFI_SSID";
const char* WIFI_PASSWORD = "WIFI_PASSWORD";

const char *MQTT_SERVER = "mqtt";
const char *MQTT_USER = NULL;
const char *MQTT_PASSWORD = NULL;

long lastMsgTime = 0;

const uint8_t PMS_RX = 14, PMS_TX = 15;
SerialPM pms(PMS5003, PMS_RX, PMS_TX); // PMSx003, RX, TX
WiFiClient espClient;
PubSubClient mqttClient(espClient);

int pm01, pm25, pm10, n0p3, n0p5, n1p0, n2p5, n5p0, n10p0;

void setup() {
  Serial.begin(115200);
  while (! Serial);
  Serial.println(F("Booted"));

  Serial.println(F("PMS sensor on SWSerial"));
  Serial.print(F("  RX:"));
  Serial.println(PMS_RX);
  Serial.print(F("  TX:"));
  Serial.println(PMS_TX);

  setupWifi();
  mqttClient.setServer(MQTT_SERVER, 1883);
  pms.init();
}

void loop() {
  if (!mqttClient.connected()) {
    mqttReconnect();
  }
  mqttClient.loop();

  long now = millis();
  if (now - lastMsgTime > MQTT_PUBLISH_DELAY) {
    lastMsgTime = now;

    pms.read();
    if (pms)
    {
      pm01 = pms.pm01;
      pm25 = pms.pm25;
      pm10 = pms.pm10;
      
      Serial.printf("PM1.0 %2d, PM2.5 %2d, PM10 %2d [ug/m3]\n",
                    pm01, pm25, pm10);
      mqttPublish(MQTT_TOPIC_PM01, pm01);
      mqttPublish(MQTT_TOPIC_PM25, pm25);
      mqttPublish(MQTT_TOPIC_PM10, pm10);
      if (pms.has_number_concentration()) {
        n0p3 = pms.n0p3;
        n0p5 = pms.n0p5;
        n1p0 = pms.n1p0;
        n2p5 = pms.n2p5;
        n5p0 = pms.n5p0;
        n10p0 = pms.n10p0;
        Serial.printf("N0.3 %4d, N0.5 %3d, N1.0 %2d, N2.5 %2d, N5.0 %2d, N10 %2d [#/100cc]\n",
                      n0p3, n0p5, n1p0, n2p5, n5p0, n10p0);
        mqttPublish(MQTT_TOPIC_N0P3, n0p3);
        mqttPublish(MQTT_TOPIC_N0P5, n0p5);
        mqttPublish(MQTT_TOPIC_N1P0, n1p0);
        mqttPublish(MQTT_TOPIC_N2P5, n2p5);
        mqttPublish(MQTT_TOPIC_N5P0, n5p0);
        mqttPublish(MQTT_TOPIC_N10P0, n10p0);
      }
    }
    else
    { // something went wrong
      switch (pms.status)
      {
      case pms.OK: // should never come here
        break;     // included to compile without warnings
      case pms.ERROR_TIMEOUT:
        Serial.println(F(PMS_ERROR_TIMEOUT));
        break;
      case pms.ERROR_MSG_UNKNOWN:
        Serial.println(F(PMS_ERROR_MSG_UNKNOWN));
        break;
      case pms.ERROR_MSG_HEADER:
        Serial.println(F(PMS_ERROR_MSG_HEADER));
        break;
      case pms.ERROR_MSG_BODY:
        Serial.println(F(PMS_ERROR_MSG_BODY));
        break;
      case pms.ERROR_MSG_START:
        Serial.println(F(PMS_ERROR_MSG_START));
        break;
      case pms.ERROR_MSG_LENGTH:
        Serial.println(F(PMS_ERROR_MSG_LENGTH));
        break;
      case pms.ERROR_MSG_CKSUM:
        Serial.println(F(PMS_ERROR_MSG_CKSUM));
        break;
      case pms.ERROR_PMS_TYPE:
        Serial.println(F(PMS_ERROR_PMS_TYPE));
        break;
      }
    }

  }
}

void setupWifi() {
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void mqttReconnect() {
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");

    // Attempt to connect
    if (mqttClient.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASSWORD, MQTT_TOPIC_STATE, 1, true, "disconnected", false)) {
      Serial.println("connected");

      // Once connected, publish an announcement...
      mqttClient.publish(MQTT_TOPIC_STATE, "connected", true);
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void mqttPublish(char* topic, int payload) {
  Serial.print(topic);
  Serial.print(": ");
  Serial.println(payload);

  mqttClient.publish(topic, String(payload).c_str(), true);
}
