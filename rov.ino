#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "DEIVY";            
const char* password = "19082024Dav#Day";  
const char* mqtt_server = "192.168.1.49"; 

const int PIN_LAT_1 = 12; 
const int PIN_LAT_2 = 13;

const int PIN_VERT_1 = 14; 
const int PIN_VERT_2 = 15;

const int FRECUENCIA = 1000;
const int RESOLUCION = 8; 

int VELOCIDAD_MANTENIMIENTO = 100; 

WiFiClient espClient;
PubSubClient client(espClient);

void avanzar() {
  Serial.println("ACCION: Motor Avanzando"); // Debug
  digitalWrite(PIN_LAT_1, HIGH);
  digitalWrite(PIN_LAT_2, LOW);
}
void retroceder() {
  Serial.println("ACCION: Motor Retrocediendo");
  digitalWrite(PIN_LAT_1, LOW);
  digitalWrite(PIN_LAT_2, HIGH);
}
void pararLaterales() {
  Serial.println("ACCION: Stop Laterales");
  digitalWrite(PIN_LAT_1, LOW);
  digitalWrite(PIN_LAT_2, LOW);
}

void sumergirFull() {
  Serial.println("ACCION: Sumergiendo FULL");
  ledcWrite(PIN_VERT_1, 255); 
  ledcWrite(PIN_VERT_2, 0);
}

void mantenerProfundidad() {
  Serial.println("ACCION: Manteniendo Profundidad");
  ledcWrite(PIN_VERT_1, VELOCIDAD_MANTENIMIENTO);
  ledcWrite(PIN_VERT_2, 0);
}

void emerger() {
  Serial.println("ACCION: Emergiendo");
  ledcWrite(PIN_VERT_1, 0);
  ledcWrite(PIN_VERT_2, 255); 
}

void pararVertical() {
  Serial.println("ACCION: Stop Vertical");
  ledcWrite(PIN_VERT_1, 0);
  ledcWrite(PIN_VERT_2, 0);
}

void callback(char* topic, byte* payload, unsigned int length) {
  String mensaje = "";
  for (int i = 0; i < length; i++) {
    mensaje += (char)payload[i];
  }
  
  Serial.print("Mensaje recibido [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(mensaje);

  if (String(topic) == "submarino/movimiento") {
    if (mensaje == "1") avanzar();
    else if (mensaje == "-1") retroceder();
    else pararLaterales();
  }
  
  if (String(topic) == "submarino/profundidad") {
    if (mensaje == "1") sumergirFull();       
    else if (mensaje == "2") mantenerProfundidad(); 
    else if (mensaje == "-1") emerger();            
    else pararVertical();      
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Intentando conexion MQTT a ");
    Serial.print(mqtt_server);
    Serial.print("...");
    
    if (client.connect("SubmarinoCam")) { 
      Serial.println("¡CONECTADO!");
      client.subscribe("submarino/movimiento");
      client.subscribe("submarino/profundidad");
    } else {
      Serial.print("falló, rc=");
      Serial.print(client.state());
      Serial.println(" intentando en 5 segundos");
      delay(5000);
    }
  }
}

void setup() {
  // 1. INICIAR SERIAL (ESTO FALTABA)
  Serial.begin(115200);
  Serial.println("\n--- INICIANDO SUBMARINO ---");

  pinMode(PIN_LAT_1, OUTPUT);
  pinMode(PIN_LAT_2, OUTPUT);
  
  ledcAttach(PIN_VERT_1, FRECUENCIA, RESOLUCION);
  ledcAttach(PIN_VERT_2, FRECUENCIA, RESOLUCION);
  
  pararLaterales();
  pararVertical();

  // 2. CONECTAR WIFI
  Serial.print("Conectando a WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi Conectado!");
  Serial.print("IP asignada: ");
  Serial.println(WiFi.localIP());

  client.setServer(mqtt_server, 1883); 
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop(); 
}