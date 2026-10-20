#include <WiFi.h>
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>

// Credenciales de tu red Wi-Fi
#define WLAN_SSID       "TU_RED_WIFI"
#define WLAN_PASS       "TU_CONTRASENA_WIFI"

// Credenciales de Adafruit IO
#define AIO_SERVER      "://adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "TU_USUARIO_ADAFRUIT"
#define AIO_KEY         "TU_KEY_ADAFRUIT"

// Variables globales para Wi-Fi y MQTT
WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

// --- CONFIGURACIÓN DE FEEDS ---
Adafruit_MQTT_Publish pub_feed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/temperatura");
Adafruit_MQTT_Subscribe sub_feed = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/interruptor");

// Función callback para la suscripción
void interruptorCallback(char *data, uint16_t len) {
  Serial.print(F("--> Mensaje recibido desde Adafruit: "));
  Serial.println(data);
  
  if (strcmp(data, "ON") == 0) {
    digitalWrite(LED_BUILTIN, HIGH);
  } else if (strcmp(data, "OFF") == 0) {
    digitalWrite(LED_BUILTIN, LOW);
  }
}

void setup() {
  Serial.begin(115200);
  delay(10);
  
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.println(F("Conectando a Wi-Fi..."));
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi conectado.");

  sub_feed.setCallback(interruptorCallback);
  mqtt.subscribe(&sub_feed);
}

void loop() {
  // Asegurar la conexión MQTT
  MQTT_connect();

  // --- BLOQUE DE PUBLICACIÓN ---
  float valorSensor = random(20, 35); // Simulación de tu sensor

  Serial.print(F("<-- Publicando dato en Adafruit... "));
  Serial.print(valorSensor);
  
  if (!pub_feed.publish(valorSensor)) {
    Serial.println(F(" ¡Falló la publicación!"));
  } else {
    Serial.println(F(" ¡Publicado con éxito!"));
  }

  // --- RETRASO Y ESCUCHA SIMULTÁNEA ---
  // En lugar de delay(5000), usamos processPackets(5000).
  // Esto hace una pausa de 5 segundos, pero procesa CUALQUIER mensaje que llegue en ese tiempo.
  mqtt.processPackets(5000);

  // Mantener vivo el servidor (Ping)
  if (!mqtt.ping()) {
    mqtt.disconnect();
  }
}

void MQTT_connect() {
  int8_t ret;
  if (mqtt.connected()) {
    return;
  }

  Serial.print(F("Conectando a MQTT... "));
  while ((ret = mqtt.connect()) != 0) { 
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println(F("Reintentando conexión en 5 segundos..."));
    mqtt.disconnect();
    delay(5000); // Este delay solo ocurre si se cae el servidor
  }
  Serial.println(F("¡MQTT Conectado!"));
}
