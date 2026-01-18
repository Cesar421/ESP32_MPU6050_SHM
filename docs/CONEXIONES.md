# Diagrama de conexión ESP32 + MPU6050

```
┌─────────────────────┐
│      MPU6050        │
│  ┌──────────────┐   │
│  │ VCC  →  3.3V │   │
│  │ GND  →  GND  │   │
│  │ SCL  →  22   │   │
│  │ SDA  →  21   │   │
│  └──────────────┘   │
└─────────────────────┘
           │
           │ I2C
           ↓
┌─────────────────────┐
│       ESP32         │
│                     │
│  GPIO 21 (SDA)      │
│  GPIO 22 (SCL)      │
│                     │
│  WiFi AP:           │
│  SSID: ESP32-MPU    │
│  IP: 192.168.4.1    │
└─────────────────────┘
           │
           │ WiFi
           ↓
┌─────────────────────┐
│   Tu Computadora    │
│                     │
│  Navegador Web:     │
│  192.168.4.1        │
│                     │
│  Python Script:     │
│  esp32_control.py   │
└─────────────────────┘
```

## Flujo de trabajo

1. **Conexión Física**
   - Conecta MPU6050 al ESP32 según el diagrama
   - Alimenta el ESP32 (USB o 5V)

2. **Configuración WiFi**
   - ESP32 crea red WiFi "ESP32-MPU6050"
   - Conéctate desde tu PC/móvil
   - Password: "12345678"

3. **Acceso Web**
   - Abre navegador → 192.168.4.1
   - Interfaz con tablas en tiempo real
   - Control de mediciones
   - Descarga de archivos CSV

4. **Control desde Python**
   - Usa script esp32_control.py
   - API REST completa
   - Análisis y gráficos automáticos
