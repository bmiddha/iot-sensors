# iot-sensors

Sensor data logging and graphing.

## Tech

ESP8266 w/ sensors report data via MQTT to *Eclipse Mosquitto*.
*Telegraf* is subscribed to the events and posts data to the *Influxdb* database.
*Grafana* is used to query data and visualize it with fancy dashboards.

## Hardware

- ESP8266 - Arduino compatible microcontroller with Wi-Fi.
- DHT22 - Temperature-Humidity Sensor
- PMS5003 - PM2.5 Air Quality Sensor
