# 🚀 Guía de Inicio Rápido

## Paso 1: Hardware

### Materiales necesarios:
- ✅ ESP32 (cualquier modelo)
- ✅ Sensor MPU6050
- ✅ 4 cables jumper
- ✅ Cable USB para programar

### Conexiones:
```
MPU6050          ESP32
───────────────────────
VCC      →       3.3V
GND      →       GND
SCL      →       GPIO 22
SDA      →       GPIO 21
```

## Paso 2: Software - PlatformIO

### A. Instalar PlatformIO (si no lo tienes)
1. Abre VS Code
2. Ve a Extensions (Ctrl+Shift+X)
3. Busca "PlatformIO IDE"
4. Haz clic en "Install"
5. Reinicia VS Code

### B. Compilar y subir el código
1. Abre este proyecto en VS Code
2. PlatformIO detectará automáticamente el proyecto
3. Conecta el ESP32 por USB
4. Haz clic en el botón "Upload" (→) en la barra inferior
   - O presiona: `Ctrl+Alt+U`
   - O desde terminal: `pio run --target upload`

### C. Verificar funcionamiento
1. Abre el Monitor Serial
   - Botón "Serial Monitor" (🔌) en la barra inferior
   - O presiona: `Ctrl+Alt+S`
   - O desde terminal: `pio device monitor`

2. Deberías ver:
```
=== ESP32 MPU6050 Data Logger ===
MPU6050 inicializado correctamente
LittleFS montado correctamente

=== WiFi Access Point ===
SSID: ESP32-MPU6050
Password: 12345678
IP: 192.168.4.1
URL: http://192.168.4.1
========================

Sistema listo!
```

## Paso 3: Conectarse al ESP32

### A. Conexión WiFi
1. En tu PC/móvil, busca redes WiFi
2. Conecta a: **ESP32-MPU6050**
3. Contraseña: **12345678**

### B. Acceder a la interfaz web
1. Abre navegador
2. Ve a: **http://192.168.4.1**
3. ¡Verás la interfaz con las tablas en tiempo real!

## Paso 4: Usar desde Python

### A. Activar entorno
```bash
conda activate CesPy13MLDS
```

### B. Ejecutar ejemplo rápido
```bash
cd python_client
python ejemplo_rapido.py
```

### C. Ejecutar medición automatizada
```bash
python ejemplo_medicion.py
```

### D. Interfaz interactiva completa
```bash
python esp32_control.py
```

## 🎯 Primeras Pruebas

### Prueba 1: Datos en tiempo real (Web)
1. Abre http://192.168.4.1
2. Observa las tablas actualizándose
3. Mueve el sensor y verás cambios en acelerómetro/giroscopio

### Prueba 2: Medición simple
1. En la web, configura:
   - Duración: 10 segundos
   - Frecuencia: 100 Hz
2. Haz clic en "▶️ Iniciar"
3. Espera 10 segundos
4. Verás el archivo en "Archivos de Datos"
5. Haz clic en "⬇️ Descargar"

### Prueba 3: Python
```python
from esp32_control import ESP32MPU6050

# Conectar
esp32 = ESP32MPU6050()
esp32.network_info()

# Leer datos actuales
data = esp32.get_current_data()
print(f"Temperatura: {data['temp']} °C")

# Hacer medición
esp32.start_measurement(duration=5, sample_rate=100)
# ... esperar 5 segundos ...
esp32.download_latest_file("mis_datos.csv")
```

## ⚙️ Configuración Personalizada

### Cambiar nombre WiFi
En [src/main.cpp](src/main.cpp) líneas 24-25:
```cpp
const char* ap_ssid = "MI_NOMBRE";
const char* ap_password = "MI_PASS_SEGURO";
```

### Cambiar pines I2C
En [src/main.cpp](src/main.cpp) líneas 28-29:
```cpp
#define SDA_PIN 21
#define SCL_PIN 22
```

### Ajustar buffer de memoria
En [src/main.cpp](src/main.cpp) línea 47:
```cpp
const int MAX_SAMPLES = 10000;  // Reduce si falta RAM
```

## 🐛 Solución de Problemas

### ❌ "MPU6050 no encontrado"
- ✅ Verifica conexiones (SDA, SCL, VCC, GND)
- ✅ Comprueba que VCC esté en 3.3V (NO 5V)
- ✅ Prueba intercambiar SDA/SCL

### ❌ No aparece red WiFi
- ✅ Espera 10-15 segundos después de encender
- ✅ Verifica en Monitor Serial que dice "Sistema listo!"
- ✅ Reinicia el ESP32

### ❌ "Error compilando"
- ✅ Cierra y abre VS Code
- ✅ Ejecuta: `pio lib install` en terminal
- ✅ Verifica conexión a Internet (para descargar librerías)

### ❌ Python no conecta
- ✅ Verifica que estés conectado al WiFi del ESP32
- ✅ Prueba ping: `ping 192.168.4.1`
- ✅ Verifica firewall de Windows

## 📊 Formato de Datos CSV

Los archivos descargados tienen este formato:
```csv
Timestamp,Accel_X,Accel_Y,Accel_Z,Gyro_X,Gyro_Y,Gyro_Z,Temperature
1234,0.123,-0.456,9.812,0.001,-0.002,0.000,25.34
1245,0.125,-0.453,9.815,0.002,-0.001,0.001,25.35
```

Puedes abrirlos en:
- Excel
- Python (pandas)
- MATLAB
- Cualquier programa de análisis

## 🎓 Recursos Adicionales

- [README completo](README.md) - Documentación detallada
- [Conexiones](docs/CONEXIONES.md) - Diagramas de conexión
- [API REST](README.md#-api-rest) - Endpoints disponibles

## ✅ Checklist de Verificación

- [ ] Hardware conectado correctamente
- [ ] PlatformIO instalado en VS Code
- [ ] Código subido al ESP32
- [ ] Monitor Serial muestra "Sistema listo!"
- [ ] Red WiFi "ESP32-MPU6050" visible
- [ ] Conectado a la red WiFi
- [ ] Interfaz web accesible en 192.168.4.1
- [ ] Datos en tiempo real funcionando
- [ ] Python instalado con librerías
- [ ] Script de prueba ejecutado exitosamente

¡Listo! Ahora tienes un sistema completo de adquisición de datos con ESP32 y MPU6050 🎉
