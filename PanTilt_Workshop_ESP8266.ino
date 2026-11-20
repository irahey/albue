// Pan-tilt workshop sketch: ESP8266 (NodeMCU / Wemos D1 mini) + 2 hobby servos, phone browser.
// Needs: ESP8266 Arduino core and the built-in Servo library.
// Board in the Arduino IDE: "NodeMCU 1.0 (ESP-12E Module)" or "LOLIN(WEMOS) D1 R2 & mini".
//
// Safety design (same as the ESP32 sketch):
//  - Angle limits are enforced here in firmware, not only in the web page.
//  - Movement is slew-limited, so the servos never jump to a new angle.
//  - The phone sends the target every 200 ms. If commands stop for FAILSAFE_MS,
//    the platform moves back to the centre by itself.
//  - Both servos go to the centre at boot.

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Servo.h>

// ---------- Wiring (keep in sync with the pin table) ----------
const int PAN_PIN  = 5;  // D1 = GPIO5
const int TILT_PIN = 4;  // D2 = GPIO4

// ---------- Settings ----------
const char*   AP_SSID    = "PT-01";           // unique per unit
const char*   AP_PASS    = "change-me-1234";  // change this (8+ characters)
const uint8_t AP_CHANNEL = 1;                 // use 1, 6 or 11

// Angle limits in degrees. Calibrate on your bracket, then stay inside the hard stops.
const float PAN_MIN  = 30, PAN_MAX  = 150, PAN_CENTER  = 90;
const float TILT_MIN = 50, TILT_MAX = 130, TILT_CENTER = 90;

// If a servo moves the wrong way, flip it here instead of rewiring.
const bool PAN_INVERT  = false;
const bool TILT_INVERT = false;

// Pulse width (microseconds) that maps to 0 and 180 degrees. Servos differ: calibrate.
const int PULSE_MIN_US = 500;
const int PULSE_MAX_US = 2400;

const float         SLEW_DEG_PER_S = 90.0;  // maximum speed
const unsigned long FAILSAFE_MS    = 1200;  // no command for this long: go to centre
const unsigned long STEP_MS        = 20;    // control step

ESP8266WebServer server(80);
Servo panServo, tiltServo;
float curPan = PAN_CENTER, curTilt = TILT_CENTER;
float tgtPan = PAN_CENTER, tgtTilt = TILT_CENTER;
unsigned long lastCmd = 0;
unsigned long lastStep = 0;

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1, user-scalable=no">
<title>Pan-tilt</title>
<style>
  body { font-family: sans-serif; text-align: center; margin: 0; padding: 16px;
         background: #101820; color: #fff; }
  .row { max-width: 360px; margin: 28px auto; text-align: left; }
  input[type=range] { width: 100%; height: 40px; accent-color: #2b7a57; }
  button { font-size: 1.2rem; padding: 14px 28px; border: 0; border-radius: 12px;
           background: #2b7a57; color: #fff; }
  #status { color: #9ab; min-height: 1.5em; }
</style>
</head>
<body>
<h1>Pan-tilt</h1>
<div id="status">Connecting</div>
<div class="row"><div>Pan: <span id="pv">90</span>&deg;</div>
  <input type="range" id="pan" min="0" max="180" value="90"></div>
<div class="row"><div>Tilt: <span id="tv">90</span>&deg;</div>
  <input type="range" id="tilt" min="0" max="180" value="90"></div>
<button id="centre">Centre</button>
<script>
  var pan = 90, tilt = 90, pc = 90, tc = 90;
  var panEl = document.getElementById("pan"), tiltEl = document.getElementById("tilt");
  var pv = document.getElementById("pv"), tv = document.getElementById("tv");
  var status = document.getElementById("status");
  function show() { pv.textContent = pan; tv.textContent = tilt; }
  function send() {
    fetch("/aim?p=" + pan + "&t=" + tilt)
      .then(function () { status.textContent = "Connected"; })
      .catch(function () { status.textContent = "No connection"; });
  }
  fetch("/info").then(function (r) { return r.json(); }).then(function (i) {
    panEl.min = i.pmin; panEl.max = i.pmax; tiltEl.min = i.tmin; tiltEl.max = i.tmax;
    pc = i.pc; tc = i.tc; pan = pc; tilt = tc;
    panEl.value = pan; tiltEl.value = tilt; show();
  });
  panEl.addEventListener("input", function () { pan = Number(panEl.value); show(); });
  tiltEl.addEventListener("input", function () { tilt = Number(tiltEl.value); show(); });
  document.getElementById("centre").addEventListener("click", function () {
    pan = pc; tilt = tc; panEl.value = pan; tiltEl.value = tilt; show();
  });
  setInterval(send, 200);
</script>
</body>
</html>
)rawliteral";

// Write one servo. deg is 0..180 before inversion.
void writeServo(Servo &servo, float deg, bool invert) {
  if (invert) deg = 180.0 - deg;
  deg = constrain(deg, 0.0, 180.0);
  int us = PULSE_MIN_US + (int)((PULSE_MAX_US - PULSE_MIN_US) * deg / 180.0);
  servo.writeMicroseconds(us);
}

float stepToward(float cur, float tgt, float maxStep) {
  if (tgt > cur) return min(cur + maxStep, tgt);
  return max(cur - maxStep, tgt);
}

void handleRoot() {
  server.send_P(200, "text/html", INDEX_HTML);  // page lives in flash, so use send_P
}

void handleInfo() {
  char buf[112];
  snprintf(buf, sizeof(buf),
           "{\"pmin\":%d,\"pmax\":%d,\"tmin\":%d,\"tmax\":%d,\"pc\":%d,\"tc\":%d}",
           (int)PAN_MIN, (int)PAN_MAX, (int)TILT_MIN, (int)TILT_MAX,
           (int)PAN_CENTER, (int)TILT_CENTER);
  server.send(200, "application/json", buf);
}

void handleAim() {
  if (!server.hasArg("p") || !server.hasArg("t")) {
    server.send(400, "text/plain", "need p and t");
    return;
  }
  tgtPan  = constrain((float)server.arg("p").toInt(), PAN_MIN, PAN_MAX);
  tgtTilt = constrain((float)server.arg("t").toInt(), TILT_MIN, TILT_MAX);
  lastCmd = millis();
  server.send(200, "text/plain", "OK");
}

void setup() {
  Serial.begin(115200);

  panServo.attach(PAN_PIN, PULSE_MIN_US, PULSE_MAX_US);
  tiltServo.attach(TILT_PIN, PULSE_MIN_US, PULSE_MAX_US);
  writeServo(panServo, curPan, PAN_INVERT);     // centre at boot
  writeServo(tiltServo, curTilt, TILT_INVERT);

  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASS, AP_CHANNEL);
  Serial.println();
  Serial.print("Wi-Fi: ");
  Serial.print(AP_SSID);
  Serial.print("  open http://");
  Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.on("/info", handleInfo);
  server.on("/aim", handleAim);
  server.begin();
}

void loop() {
  server.handleClient();

  unsigned long now = millis();
  if (now - lastStep >= STEP_MS) {
    float dt = (now - lastStep) / 1000.0;
    lastStep = now;

    if (now - lastCmd > FAILSAFE_MS) {  // commands stopped: return to centre
      tgtPan = PAN_CENTER;
      tgtTilt = TILT_CENTER;
    }

    float maxStep = SLEW_DEG_PER_S * dt;
    curPan  = stepToward(curPan, tgtPan, maxStep);
    curTilt = stepToward(curTilt, tgtTilt, maxStep);
    writeServo(panServo, curPan, PAN_INVERT);
    writeServo(tiltServo, curTilt, TILT_INVERT);
  }
}
