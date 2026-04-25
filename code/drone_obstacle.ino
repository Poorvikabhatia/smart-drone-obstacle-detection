#include <WiFi.h>

const char* ssid = "GT";
const char* password = "12345678";

WiFiServer server(80);

#define IR_PIN 4
#define LED_PIN 2
#define BUZZER 13
int sensorValue = 0;
void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(IR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);

  ledcAttach(BUZZER, 2000, 8);

  WiFi.begin(ssid, password);

  Serial.print("Connecting");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected");
  Serial.println(WiFi.localIP());

  server.begin();
}

void loop() {

  // ---------------- SENSOR LOGIC ----------------
  sensorValue = digitalRead(IR_PIN);

  if (sensorValue == LOW) {
    digitalWrite(LED_PIN, HIGH);
    ledcWriteTone(BUZZER, 2000);
  } else {
    digitalWrite(LED_PIN, LOW);
    ledcWriteTone(BUZZER, 0);
  }

  // ---------------- SERVER ----------------
  WiFiClient client = server.available();
  if (!client) return;

  String request = client.readStringUntil('\r');
  client.flush();

  // ---------- API ENDPOINT ----------
  if (request.indexOf("GET /status") >= 0) {
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/plain");
    client.println("Connection: close");
    client.println();

    client.print(sensorValue);
    client.stop();
    return;
  }

  // ---------- MAIN WEB PAGE ----------
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/html");
  client.println("Connection: close");
  client.println();

  client.println(R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Drone System</title>
</head>

<body style="font-family:Arial;text-align:center;background:#f2f2f2;">

<h1>Smart Drone Obstacle Detection System</h1>

<div id="statusBox"
     style="padding:20px;margin:20px;border-radius:10px;font-size:22px;">
</div>

<script>
async function updateStatus() {
  const res = await fetch('/status');
  const value = await res.text();

  const box = document.getElementById('statusBox');

  if (value == "0") {
    box.style.background = "#e74c3c";
    box.style.color = "white";
    box.innerHTML = "OBSTACLE DETECTED";
  } else {
    box.style.background = "#2ecc71";
    box.style.color = "white";
    box.innerHTML = "CLEAR PATH";
  }
}

// update every 500ms
setInterval(updateStatus, 500);
updateStatus();
</script>

</body>
</html>
)rawliteral");

  client.stop();
}
