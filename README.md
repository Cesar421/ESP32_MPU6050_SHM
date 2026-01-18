# ESP32 MPU6050 Data Logger

Sistema de medición con sensor MPU6050 y servidor web integrado para ESP32.

## Características

- Lectura de acelerómetro y giroscopio MPU6050
- Access Point WiFi propio
- Servidor web con visualización en tiempo real
- Almacenamiento de datos en formato CSV
- Descarga de archivos vía web
- Interfaz responsive con tablas de datos
- Control de mediciones (inicio/stop/duración)

## Hardware Necesario

- ESP32 (cualquier modelo)
- Sensor MPU6050
- Cables de conexión

## Conexiones

```
MPU6050  →  ESP32
VCC      →  3.3V
GND      →  GND
SCL      →  GPIO 22
SDA      →  GPIO 21
```

## Arquitectura del Sistema

```
┌─────────────────────────────────────────────────────────────────────────┐
│                        ARQUITECTURA COMPLETA                            │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                         │
│   ┌──────────────┐         I2C          ┌──────────────┐               │
│   │   MPU6050    │◄────────────────────►│    ESP32     │               │
│   │   (Sensor)   │      SCL/SDA         │  (main.cpp)  │               │
│   │ NO SE FLASHEA│                      │  SE FLASHEA  │               │
│   └──────────────┘                      └──────┬───────┘               │
│                                                │                        │
│                                           WiFi │ Access Point           │
│                                    "ESP32-MPU6050" / "12345678"        │
│                                                │                        │
│                                         ┌──────▼───────┐               │
│                                         │   Dispositivo│               │
│                                         │ (PC/Celular) │               │
│                                         └──────┬───────┘               │
│                                                │                        │
│                          ┌─────────────────────┴─────────────┐         │
│                          │                                   │         │
│                    ┌─────▼─────┐                    ┌────────▼──────┐  │
│                    │ Navegador │                    │    Python     │  │
│                    │  (HTTP)   │                    │  (Opcional)   │  │
│                    └───────────┘                    └───────────────┘  │
│                          │                                   │         │
│                          └──────────┬────────────────────────┘         │
│                                     │                                  │
│                            http://192.168.4.1                          │
│                          (Servidor Web ESP32)                          │
│                                                                         │
└─────────────────────────────────────────────────────────────────────────┘
```

### Flujo de Datos

```
1. MPU6050 mide movimiento → 2. ESP32 lee datos por I2C → 
3. Datos en RAM del ESP32 → 4. Navegador solicita datos → 
5. ESP32 envía JSON por WiFi → 6. Navegador muestra en tablas
```

### Almacenamiento de Datos

```
┌─────────────────────────────────────────────────────────────┐
│                ALMACENAMIENTO EN ESP32                      │
│                                                             │
│  Formato: CSV (texto plano)                                │
│  Ubicación: Memoria Flash (LittleFS)                       │
│  Tamaño: ~100 bytes por muestra                            │
│                                                             │
│  Estructura del archivo CSV:                               │
│  ┌───────────────────────────────────────────────────┐    │
│  │ Timestamp,Accel_X,Accel_Y,Accel_Z,Gyro_X,...      │    │
│  │ 1234,0.123,-0.456,9.812,0.001,-0.002,0.000,25.34 │    │
│  │ 1245,0.125,-0.453,9.815,0.002,-0.001,0.001,25.35 │    │
│  └───────────────────────────────────────────────────┘    │
│                                                             │
│  Descarga: http://192.168.4.1/api/download?file=...       │
└─────────────────────────────────────────────────────────────┘
```

## Instalación

### 1. Instalar PlatformIO en VS Code

Si no tienes PlatformIO instalado:
1. Abre VS Code
2. Ve a Extensions (Ctrl+Shift+X)
3. Busca "PlatformIO IDE"
4. Instala la extensión

### 2. Clonar Proyecto desde GitHub

```powershell
# Navegar a tu carpeta de proyectos
cd "C:\Users\TU_USUARIO\Proyectos"

# Clonar el repositorio
git clone https://github.com/Cesar421/ESP32_MPU6050_SHM.git

# Entrar a la carpeta del proyecto
cd ESP32_MPU6050_SHM

# Instalar dependencias Python (opcional, solo si usarás scripts Python)
pip install requests pandas numpy matplotlib
```

**O descarga el ZIP:**
1. Ve a https://github.com/Cesar421/ESP32_MPU6050_SHM
2. Click en "Code" → "Download ZIP"
3. Extrae y abre la carpeta en VS Code

## Comandos de Compilación y Flasheo

### Comandos Básicos

#### 1. **Compilar (Build)**
```powershell
pio run
```
**¿Qué hace?**
- Compila el código C++ de `src/main.cpp`
- Descarga automáticamente todas las librerías necesarias
- Verifica errores de sintaxis
- Genera el archivo binario `.elf` para el ESP32
- **NO** sube el código al ESP32 (solo compila)

**Salida esperada:**
```
RAM:   [=         ]  13.6% (used 44536 bytes from 327680 bytes)
Flash: [======    ]  67.9% (used 890965 bytes from 1310720 bytes)
```

---

#### 2. **Flashear/Subir al ESP32 (Upload)**
```powershell
pio run --target upload
```
**¿Qué hace?**
- Compila el código (si hay cambios)
- Detecta el puerto COM del ESP32
- Borra la flash del ESP32
- Sube el firmware compilado
- Reinicia el ESP32 automáticamente

**Requisitos:**
- ESP32 conectado por cable USB
- Drivers CH340/CP2102 instalados (instalación automática en Windows 10/11)

**Nota:** Si falla la detección del puerto, mantén presionado el botón **BOOT** del ESP32 durante el flasheo.

---

#### 3. **Compilar + Subir + Monitor (Todo en uno)**
```powershell
pio run --target upload --target monitor
```
**¿Qué hace?**
- Compila el código
- Sube al ESP32
- Abre el monitor serial automáticamente
- **Recomendado para desarrollo**

**Para salir del monitor:** `Ctrl + C`

---

#### 4. **Solo Monitor Serial**
```powershell
pio device monitor
```
**¿Qué hace?**
- Muestra la salida del `Serial.println()` del ESP32
- Útil para ver:
  - Mensajes de inicialización
  - Estado de conexión WiFi
  - Errores del sensor MPU6050
  - Información de mediciones

**Velocidad configurada:** 115200 baud (ver `platformio.ini`)

---

#### 5. **Limpiar Proyecto (Clean)**
```powershell
pio run --target clean
```
**¿Qué hace?**
- Elimina archivos compilados (`.pio/build/`)
- Útil cuando:
  - Cambias configuración en `platformio.ini`
  - Tienes errores raros de compilación
  - Quieres recompilar todo desde cero

---

#### 6. **Borrar Completamente la Flash del ESP32**
```powershell
pio run --target erase
```
**¿Qué hace?**
- **BORRA TODO** el contenido de la memoria flash
- Elimina:
  - Firmware actual
  - Archivos guardados en LittleFS
  - Configuraciones
- Deja el ESP32 vacío (necesitarás volver a flashear)

**⚠️ ADVERTENCIA:** Perderás todos los archivos `.bin` guardados.

---

#### 7. **Solo Compilar (Sin Subir)**
```powershell
pio run --environment esp32dev
```
**¿Qué hace?**
- Compila únicamente para ESP32
- Verifica que no haya errores
- No sube al dispositivo

---

#### 8. **Actualizar Librerías**
```powershell
pio pkg update
```
**¿Qué hace?**
- Actualiza todas las librerías a sus últimas versiones:
  - Adafruit MPU6050
  - ESPAsyncWebServer
  - ArduinoJson
  - etc.

---

### Interfaz de PlatformIO en VS Code

#### Barra de Estado Inferior (Project Tasks)

Cuando abres un proyecto PlatformIO en VS Code, verás una barra azul en la parte inferior con estos botones:

```
┌─────────────────────────────────────────────────────────────────────────────┐
│ BARRA INFERIOR DE VS CODE (cuando archivo del proyecto está abierto)       │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  [HOME] [BUILD] [UPLOAD] [UPLOAD+MON] [CLEAN] [MONITOR] [TASKS] [TERM] [DEBUG]│
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

---

#### Descripción de Cada Botón

| Botón | Nombre | Comando Equivalente | Descripción |
|-------|--------|---------------------|-------------|
| **HOME** | PlatformIO Home | - | Abre la página principal de PlatformIO |
| **BUILD** | Build Project | `pio run` | Compila el proyecto sin subirlo al ESP32 |
| **UPLOAD** | Upload | `pio run --target upload` | Sube el firmware al ESP32 conectado |
| **UPLOAD+MON** | Upload and Monitor | `pio run --target upload --target monitor` | Sube el código y abre el monitor serial |
| **CLEAN** | Clean | `pio run --target clean` | Elimina archivos compilados |
| **MONITOR** | Serial Monitor | `pio device monitor` | Abre el monitor serial (ver salida del ESP32) |
| **TASKS** | PlatformIO Tasks | - | Muestra todas las tareas disponibles |
| **TERM** | New Terminal | - | Abre terminal de PlatformIO |
| **DEBUG** | Start Debugging | - | Inicia depuración (requiere debugger hardware) |

---

#### Uso Recomendado de los Botones

##### **1. Primera Compilación**
```
Click: ✓ Build
└─► Compila y verifica errores
    Si sale ✅ SUCCESS → Todo bien
    Si sale ❌ FAILED → Revisa errores en terminal
```

##### **2. Subir al ESP32 (Primera vez)**
```
Click: → Upload
└─► Compila (si hay cambios) + Sube al ESP32
    Requisito: ESP32 conectado por USB
    Resultado: Firmware instalado, ESP32 reinicia
```

##### **3. Desarrollo Normal (Recomendado)**
```
Click: ↻ Upload and Monitor
└─► Compila + Sube + Abre Monitor Serial
    ┌────────────────────────────────────┐
    │ Lo más usado durante desarrollo    │
    │ Ver logs inmediatamente después    │
    │ de subir el código                 │
    └────────────────────────────────────┘
```

##### **4. Ver Salida del ESP32**
```
Click: 🔌 Serial Monitor
└─► Abre monitor para ver Serial.println()
    ┌─────────────────────────────────────────┐
    │ Muestra:                                │
    │ - "WiFi AP iniciado"                    │
    │ - "MPU6050 inicializado"                │
    │ - Errores y warnings                    │
    │ - IP del servidor (192.168.4.1)        │
    └─────────────────────────────────────────┘
    
    Para cerrar: Ctrl+C o click en 🗑️ (cerrar terminal)
```

##### **5. Limpiar Compilación**
```
Click: 🗑️ Clean
└─► Elimina archivos .o y binarios compilados
    Usar cuando:
    - Cambias platformio.ini
    - Errores extraños de compilación
    - Quieres recompilar desde cero
```

---

#### Panel Lateral de PlatformIO

Haz click en el ícono de **PlatformIO** en la barra lateral izquierda de VS Code:

```
┌─────────────────────────────────────────┐
│  PLATFORMIO SIDEBAR                     │
├─────────────────────────────────────────┤
│                                         │
│  PROJECT TASKS                          │
│    └─ esp32dev                          │
│       ├─ General                        │
│       │  ├─ Build                       │
│       │  ├─ Clean                       │
│       │  ├─ Upload                      │
│       │  └─ Monitor                     │
│       │                                 │
│       ├─ Advanced                       │
│       │  ├─ Upload and Monitor          │
│       │  ├─ Erase Flash                 │
│       │  ├─ Upload File System Image    │
│       │  └─ Verbose Build               │
│       │                                 │
│       └─ Platform                       │
│          └─ Update                      │
│                                         │
│  LIBRARIES                              │
│    └─ Search Libraries                  │
│    └─ Update All                        │
│                                         │
│  QUICK ACCESS                           │
│    └─ PIO Home                          │
│    └─ Open Project                      │
│    └─ New Project                       │
│                                         │
└─────────────────────────────────────────┘
```

---

#### Atajos de Teclado (Opcional)

Configura atajos personalizados en VS Code:

| Acción | Atajo Sugerido | Comando |
|--------|----------------|---------|
| Build | `Ctrl+Alt+B` | `platformio-ide.build` |
| Upload | `Ctrl+Alt+U` | `platformio-ide.upload` |
| Upload & Monitor | `Ctrl+Alt+M` | `platformio-ide.uploadAndMonitor` |
| Serial Monitor | `Ctrl+Alt+S` | `platformio-ide.serialMonitor` |

Para configurar:
1. `Ctrl+K Ctrl+S` → Abrir atajos de teclado
2. Buscar "PlatformIO"
3. Asignar atajos personalizados

---

#### Monitor Serial - Comandos Útiles

Cuando el monitor serial está activo:

```powershell
# En la terminal del monitor serial:

# Cerrar monitor
Ctrl + C

# Filtrar salida (solo líneas con "ERROR")
Ctrl+F → Buscar "ERROR"

# Limpiar pantalla
Ctrl + L

# Desplazarse
↑ ↓ (flechas) o Page Up/Page Down
```

---

### Comandos de Terminal - Referencia Completa

#### Comandos Básicos

```powershell
# 1. Compilar proyecto
pio run

# 2. Subir al ESP32
pio run --target upload

# 3. Subir y abrir monitor
pio run --target upload --target monitor

# 4. Solo monitor serial
pio device monitor

# 5. Limpiar proyecto
pio run --target clean

# 6. Borrar flash del ESP32
pio run --target erase
```

---

#### Comandos con Opciones Avanzadas

```powershell
# Subir especificando puerto COM
pio run --target upload --upload-port COM3

# Subir con velocidad reducida (si falla)
pio run --target upload --upload-speed 115200

# Compilar con salida detallada
pio run --verbose

# Monitor con filtros
pio device monitor --filter esp32_exception_decoder

# Monitor con tasa de baudios personalizada
pio device monitor --baud 115200

# Ver tamaño del firmware
pio run --target size
```

---

#### Comandos de Gestión de Librerías

```powershell
# Buscar librería
pio pkg search MPU6050

# Instalar librería específica
pio pkg install "adafruit/Adafruit MPU6050@^2.0.0"

# Actualizar todas las librerías
pio pkg update

# Ver librerías instaladas
pio pkg list

# Desinstalar librería
pio pkg uninstall "adafruit/Adafruit MPU6050"
```

---

#### Comandos de Información

```powershell
# Ver información del sistema
pio system info

# Ver versión de PlatformIO
pio --version

# Listar dispositivos conectados
pio device list

# Ver detalles del proyecto
pio project config
```

---

#### Comandos de Limpieza Total

```powershell
# Limpiar caché completo de PlatformIO (Windows PowerShell)
Remove-Item -Recurse -Force .pio
pio run

# Eliminar librerías descargadas
Remove-Item -Recurse -Force .pio\libdeps

# Limpiar todo y reinstalar
pio run --target clean
pio pkg install
pio run
```

---

### Flujo de Trabajo Típico

#### Primera vez (Setup inicial):

**Opción A - Usando Botones:**
```
1. Click: BUILD → Verifica que compila sin errores
2. Conecta ESP32 por USB
3. Click: UPLOAD+MON → Sube y muestra logs
4. Verifica en monitor:
   - "WiFi AP iniciado"
   - "Servidor web iniciado"
   - "MPU6050 inicializado"
```

**Opción B - Usando Terminal:**
```powershell
# 1. Compilar para verificar que todo está bien
pio run

# 2. Subir al ESP32 con monitor
pio run --target upload --target monitor

# 3. Verificar en el monitor que dice:
#    - "WiFi AP iniciado"
#    - "Servidor web iniciado"
#    - "MPU6050 inicializado"
```

---

#### Durante desarrollo (cada cambio):

**Opción A - Usando Botones (Más Rápido):**
```
1. Modifica código en src/main.cpp
2. Guarda archivo (Ctrl+S)
3. Click: UPLOAD+MON
4. Espera a ver logs en monitor serial
```

**Opción B - Usando Terminal:**
```powershell
# Opción rápida: Subir y ver monitor
pio run --target upload --target monitor
```

---

#### Si hay problemas:

**Opción A - Usando Botones:**
```
1. Click: CLEAN
2. Click: BUILD → Recompila todo
3. Si persiste el error:
   - Panel lateral PlatformIO → esp32dev → Advanced → Erase Flash
4. Click: UPLOAD
```

**Opción B - Usando Terminal:**
```powershell
# 1. Limpiar proyecto
pio run --target clean

# 2. Recompilar todo
pio run

# 3. Borrar flash del ESP32 (último recurso)
pio run --target erase

# 4. Volver a subir
pio run --target upload
```

---

### Comparación: Botones vs Terminal

| Tarea | Botones VS Code | Terminal | Recomendación |
|-------|-----------------|----------|---------------|
| **Compilar** | Click: BUILD | `pio run` | Botón (más visual) |
| **Subir código** | Click: UPLOAD | `pio run --target upload` | Botón (un click) |
| **Subir + Monitor** | Click: UPLOAD+MON | `pio run --target upload --target monitor` | Botón (más rápido) |
| **Monitor serial** | Click: MONITOR | `pio device monitor` | Botón (más fácil) |
| **Limpiar** | Click: CLEAN | `pio run --target clean` | Botón (simple) |
| **Borrar flash** | Panel → Erase Flash | `pio run --target erase` | Terminal (más control) |
| **Especificar puerto** | No disponible | `pio run --upload-port COM3` | **Terminal** |
| **Velocidad upload** | No disponible | `pio run --upload-speed 115200` | **Terminal** |
| **Verbose build** | Panel → Verbose Build | `pio run --verbose` | Cualquiera |
| **Automatización** | No | Scripts .bat/.ps1 | **Terminal** |

**Conclusión:**
- **Botones:** Mejor para desarrollo diario (más rápido, visual)
- **Terminal:** Mejor para troubleshooting y automatización

---

### Tips y Trucos

#### 1. Abrir PlatformIO rápidamente
```
Método 1: Click en ícono PlatformIO en barra lateral
Método 2: Ctrl+Shift+P → "PlatformIO: Home"
Método 3: Click en HOME en barra inferior
```

#### 2. Ver salida de compilación completa
```
Método 1: Terminal → Problems (Ctrl+Shift+M)
Método 2: Terminal → Output → Seleccionar "PlatformIO"
```

#### 3. Cambiar entre proyectos
```
Click: HOME PlatformIO Home → Open Project
O bien: File → Open Folder → Seleccionar proyecto
```

#### 4. Autocompletar código
```
PlatformIO instala automáticamente IntelliSense para ESP32
Ctrl+Space → Muestra funciones disponibles
Ctrl+Click en función → Va a definición
```

#### 5. Ver uso de memoria
```
Después de compilar, busca en terminal:
RAM:   [====      ]  43.2% (usado XXXXX bytes)
Flash: [========  ]  82.1% (usado XXXXX bytes)
```

---

### Atajos en VS Code (PlatformIO)

Si prefieres usar la interfaz gráfica:

| Botón | Nombre | Equivalente |
|-------|--------|-------------|
| BUILD | Build | `pio run` |
| UPLOAD | Upload | `pio run --target upload` |
| CLEAN | Clean | `pio run --target clean` |
| MONITOR | Serial Monitor | `pio device monitor` |
| UPLOAD+MON | Upload and Monitor | `pio run --target upload --target monitor` |

**Ubicación:** Barra inferior de VS Code (cuando tienes un archivo del proyecto abierto)

---

### Verificar Compilación Exitosa

Después de `pio run`, verás:

```
Compilación exitosa:
Advanced Memory Usage is available via "PlatformIO Home > Project Inspect"
RAM:   [=         ]  13.6% (used 44536 bytes from 327680 bytes)
Flash: [======    ]  67.9% (used 890965 bytes from 1310720 bytes)
========================= [SUCCESS] Took X.XX seconds =========================
```

```
❌ Compilación fallida:
src/main.cpp:123:5: error: 'Serial' was not declared in this scope
*** [.pio\build\esp32dev\src\main.cpp.o] Error 1
========================= [FAILED] Took X.XX seconds =========================
```

## 📡 Uso

### Conexión WiFi

```
┌─────────────────────────────────────────────────────────────┐
│               PROCESO DE CONEXIÓN PASO A PASO               │
└─────────────────────────────────────────────────────────────┘

1️⃣ FLASHEAR ESP32
   ┌────────────────────────────────────────────┐
   │ PC con VS Code + PlatformIO                │
   │   └─► Click "Upload and Monitor"          │
   │        └─► Código se graba en ESP32       │
   └────────────────────────────────────────────┘
           │
           │ (Desconectar USB - ESP32 con batería/fuente)
           ▼
2️⃣ ESP32 CREA RED WiFi
   ┌────────────────────────────────────────────┐
   │ Red: ESP32-MPU6050                         │
   │ Password: 12345678                         │
   │ IP: 192.168.4.1                            │
   └────────────────────────────────────────────┘
           │
           ▼
3️⃣ CONECTAR DESDE TU DISPOSITIVO
   ┌────────────────────────────────────────────┐
   │ Celular/Laptop:                            │
   │  • Abre configuración WiFi                 │
   │  • Busca "ESP32-MPU6050"                   │
   │  • Ingresa password: "12345678"            │
   │  • Conecta                                 │
   └────────────────────────────────────────────┘
           │
           ▼
4️⃣ ABRIR NAVEGADOR
   ┌────────────────────────────────────────────┐
   │ Chrome / Safari / Firefox                  │
   │  • Dirección: http://192.168.4.1          │
   │  • ¡Página web con datos en tiempo real!  │
   └────────────────────────────────────────────┘
```

**Nota**: Puedes conectarte desde múltiples dispositivos simultáneamente (celular + laptop, etc.)

1. Enciende el ESP32
2. Busca la red WiFi: `ESP32-MPU6050`
3. Contraseña: `12345678`
4. Abre navegador y ve a: `http://192.168.4.1`

### Interfaz Web

La página web incluye:

#### 📊 Datos en Tiempo Real (Tablas)
- Acelerómetro (X, Y, Z) en m/s²
- Giroscopio (X, Y, Z) en rad/s
- Temperatura en °C

#### ⚙️ Control de Medición
- **Duración**: 1-3600 segundos
- **Frecuencia**: 10, 50, 100, 200 Hz
- **Botones**: Iniciar/Detener

#### 📁 Gestión de Archivos
- Lista de archivos CSV
- Descargar archivos
- Eliminar archivos

## 🔧 Configuración Avanzada

### Cambiar credenciales WiFi

Edita en `src/main.cpp`:

```cpp
const char* ap_ssid = "TU_NOMBRE";
const char* ap_password = "TU_PASSWORD";
```

### Ajustar buffer de datos

```cpp
const int MAX_SAMPLES = 10000;  // Ajusta según RAM disponible
```

### Configurar pines I2C

```cpp
#define SDA_PIN 21
#define SCL_PIN 22
```

## 📊 Formato de Datos

### Almacenamiento Binario (Eficiente)

Los archivos se guardan en formato **binario** para máxima eficiencia:

- **Formato:** `.bin` (28 bytes por muestra)
- **Capacidad:** ~71,000 muestras en 2 MB de flash
- **Ahorro:** 72% menos espacio vs CSV

**Estructura binaria:**
```cpp
struct BinaryData {
    uint32_t timestamp;  // 4 bytes - milisegundos
    float accel_x;       // 4 bytes - m/s²
    float accel_y;       // 4 bytes - m/s²
    float accel_z;       // 4 bytes - m/s²
    float gyro_x;        // 4 bytes - rad/s
    float gyro_y;        // 4 bytes - rad/s
    float gyro_z;        // 4 bytes - rad/s
};  // Total: 28 bytes
```

### Conversión a CSV

La interfaz web convierte automáticamente binario → CSV al descargar:

1. **Navegador:** Click "📄 Descargar CSV"
2. **ESP32:** Lee `.bin`, convierte línea por línea, envía CSV
3. **Resultado:** Archivo CSV listo para Excel/MATLAB

**Formato CSV resultante:**
```csv
Timestamp,Accel_X,Accel_Y,Accel_Z,Gyro_X,Gyro_Y,Gyro_Z
1234,0.123456,-0.456789,9.812345,0.001234,-0.002345,0.000123
1244,0.125678,-0.453456,9.815678,0.002456,-0.001234,0.001345
```

### Leer Binario en Python

```python
import struct
import pandas as pd

def read_binary_data(filename):
    data = []
    with open(filename, 'rb') as f:
        while True:
            chunk = f.read(28)  # 28 bytes por muestra
            if len(chunk) < 28:
                break
            
            # Desempaquetar: unsigned int + 6 floats
            values = struct.unpack('I6f', chunk)
            data.append({
                'timestamp': values[0],
                'accel_x': values[1],
                'accel_y': values[2],
                'accel_z': values[3],
                'gyro_x': values[4],
                'gyro_y': values[5],
                'gyro_z': values[6]
            })
    
    return pd.DataFrame(data)

# Usar
df = read_binary_data('data_12345.bin')
print(df.head())
```

## 🌐 API REST

### Endpoints disponibles:

- `GET /api/status` - Estado del sistema
- `GET /api/current` - Datos actuales
- `GET /api/start?duration=60&rate=100` - Iniciar medición
- `GET /api/stop` - Detener medición
- `GET /api/files` - Listar archivos
- `GET /api/download?file=/data_xxx.csv` - Descargar archivo
- `GET /api/delete?file=/data_xxx.csv` - Eliminar archivo

### Ejemplo de respuesta `/api/status`:

```json
{
  "measuring": true,
  "samples": 1234,
  "maxSamples": 10000,
  "sampleRate": 100,
  "uptime": 123456,
  "freeHeap": 234567,
  "elapsed": 12,
  "remaining": 48
}
```

### Ejemplo de respuesta `/api/current`:

```json
{
  "timestamp": 12345,
  "accel": {
    "x": 0.123,
    "y": -0.456,
    "z": 9.812
  },
  "gyro": {
    "x": 0.001,
    "y": -0.002,
    "z": 0.000
  },
  "temp": 25.34
}
```

## 🐍 Integración con Python

### ¿Cómo funciona Python con el ESP32?

```
┌─────────────────────────────────────────────────────────────┐
│         COMUNICACIÓN PYTHON ←→ ESP32 (VÍA WiFi HTTP)        │
└─────────────────────────────────────────────────────────────┘

    TU PC/LAPTOP                      ESP32
    ┌─────────────┐                ┌──────────┐
    │   Python    │                │ Servidor │
    │   Script    │                │   Web    │
    └──────┬──────┘                └────┬─────┘
           │                            │
           │   WiFi (HTTP Request)      │
           │  GET /api/current          │
           ├───────────────────────────►│
           │                            │
           │   JSON Response            │
           │  {"accel":{"x":0.12...}}   │
           │◄───────────────────────────┤
           │                            │
           
Ejemplo práctico:

    import requests
    
    # Conectado al WiFi "ESP32-MPU6050"
    response = requests.get('http://192.168.4.1/api/current')
    data = response.json()
    print(f"Aceleración: {data['accel']['x']}")
```

### Python Scripts Incluidos (OPCIONALES)

**Los scripts Python NO son necesarios para usar el ESP32. Son solo para automatización.**

| Archivo | Función |
|---------|---------|
| `esp32_control.py` | Librería para comunicarse con ESP32 |
| `ejemplo_rapido.py` | Demo: muestra datos 5 segundos |
| `ejemplo_medicion.py` | Demo: medición automática + gráficos |

### Dos formas de usar el sistema:

```
┌──────────────────────────────────────────────────────────┐
│  OPCIÓN 1: NAVEGADOR (Recomendado para empezar)         │
│  ═══════════════════════════════════════════════         │
│                                                          │
│  ✅ No necesitas Python                                 │
│  ✅ Funciona desde celular o PC                         │
│  ✅ Interfaz visual con botones                         │
│  ✅ Ver datos en tiempo real                            │
│  ✅ Descargar archivos manualmente                      │
│                                                          │
│  http://192.168.4.1                                     │
└──────────────────────────────────────────────────────────┘

┌──────────────────────────────────────────────────────────┐
│  OPCIÓN 2: PYTHON (Para automatización avanzada)        │
│  ═══════════════════════════════════════════            │
│                                                          │
│  ⚙️ Automatizar mediciones                              │
│  ⚙️ Procesar datos con pandas/numpy                     │
│  ⚙️ Crear gráficos automáticos                          │
│  ⚙️ Integrar con otros sistemas                         │
│                                                          │
│  python ejemplo_medicion.py                             │
└──────────────────────────────────────────────────────────┘
```

## 🐍 Integración con Python

### Ejemplo de lectura de datos:

```python
import requests
import pandas as pd

# Conectarse al ESP32
ESP32_IP = "192.168.4.1"

# Leer datos actuales
response = requests.get(f"http://{ESP32_IP}/api/current")
data = response.json()
print(f"Aceleración X: {data['accel']['x']} m/s²")

# Descargar archivo CSV
files_response = requests.get(f"http://{ESP32_IP}/api/files")
files = files_response.text.split(';')[0].split(',')[0]

csv_response = requests.get(f"http://{ESP32_IP}/api/download?file={files}")
with open('datos_esp32.csv', 'wb') as f:
    f.write(csv_response.content)

# Analizar con pandas
df = pd.read_csv('datos_esp32.csv')
print(df.describe())
```

## 📱 Acceso desde Python (similar al proyecto RaspberryPi)

```python
class ESP32MPU6050:
    def __init__(self, ip="192.168.4.1"):
        self.ip = ip
        self.base_url = f"http://{ip}"
    
    def get_status(self):
        return requests.get(f"{self.base_url}/api/status").json()
    
    def start_measurement(self, duration=60, rate=100):
        return requests.get(
            f"{self.base_url}/api/start",
            params={"duration": duration, "rate": rate}
        )
    
    def stop_measurement(self):
        return requests.get(f"{self.base_url}/api/stop")
    
    def download_latest_file(self, output_path):
        files = requests.get(f"{self.base_url}/api/files").text
        latest = files.split(';')[0].split(',')[0]
        
        data = requests.get(
            f"{self.base_url}/api/download",
            params={"file": latest}
        )
        
        with open(output_path, 'wb') as f:
            f.write(data.content)

# Uso
esp32 = ESP32MPU6050()
esp32.start_measurement(duration=30, rate=100)
time.sleep(35)
esp32.download_latest_file("medicion.csv")
```

## 🛠️ Troubleshooting

### ❌ Error: "Device not found"
**Problema:** PlatformIO no encuentra el puerto COM del ESP32

**Soluciones:**
```powershell
# 1. Listar puertos disponibles
pio device list

# 2. Especificar puerto manualmente
pio run --target upload --upload-port COM3

# 3. Instalar drivers (si es necesario)
# Descargar CH340 driver o CP2102 driver según tu ESP32
```

**Windows:** Verifica en "Administrador de dispositivos" → "Puertos (COM y LPT)"

---

### ❌ Error: "Timed out waiting for packet header"
**Problema:** No se puede flashear el ESP32

**Soluciones:**
1. Mantén presionado el botón **BOOT** del ESP32
2. Mientras está presionado, ejecuta:
   ```powershell
   pio run --target upload
   ```
3. Suelta **BOOT** cuando veas "Connecting..."

**Alternativa:**
```powershell
# Reducir velocidad de flasheo
pio run --target upload --upload-speed 115200
```

---

### ❌ MPU6050 no detectado
**Síntoma:** Monitor serial muestra "Error: MPU6050 no encontrado"

**Soluciones:**
1. **Verificar conexiones:**
   ```
   MPU6050 VCC → ESP32 3.3V (NO 5V!)
   MPU6050 GND → ESP32 GND
   MPU6050 SCL → ESP32 GPIO 22
   MPU6050 SDA → ESP32 GPIO 21
   ```

2. **Probar dirección I2C:**
   - Dirección por defecto: `0x68`
   - Si AD0 está conectado a VCC: `0x69`

3. **Verificar voltaje:**
   - Usa multímetro: debe ser 3.3V en VCC del MPU6050

---

### ❌ No se conecta al WiFi "ESP32-MPU6050"
**Problema:** La red WiFi no aparece

**Soluciones:**
```powershell
# 1. Verificar en monitor serial
pio device monitor

# Debes ver:
# "Configurando Access Point..."
# "WiFi AP iniciado"
# "IP: 192.168.4.1"

# 2. Si no aparece, borrar y reflashear
pio run --target erase
pio run --target upload --target monitor

# 3. Aumentar tiempo de espera
# Espera 15-20 segundos después de ver "WiFi AP iniciado"
```

---

### ❌ Error de compilación: "LittleFS not declared"
**Problema:** Falta librería del sistema de archivos

**Solución:**
```powershell
# Limpiar y recompilar
pio run --target clean
pio pkg update
pio run
```

---

### ❌ RAM insuficiente / Reinicios inesperados
**Síntoma:** ESP32 se reinicia solo, muestra "Brownout detector"

**Solución:** Reducir `MAX_SAMPLES` en `src/main.cpp`
```cpp
// Antes
const int MAX_SAMPLES = 10000;  // ~280 KB RAM

// Después (reducir a la mitad)
const int MAX_SAMPLES = 5000;   // ~140 KB RAM
```

---

### ❌ Buffer lleno muy rápido
**Problema:** Medición se detiene antes de tiempo

**Solución:** Ajustar parámetros
```cpp
// Opción 1: Reducir frecuencia en la web
// 100 Hz → 50 Hz

// Opción 2: Guardar más frecuentemente
// Modificar en main.cpp:
const int MAX_SAMPLES = 5000;  // Guardar cada 5000 muestras
```

---

### ❌ No puedo abrir http://192.168.4.1
**Problema:** Navegador no carga la página

**Checklist:**
1. ✅ ¿Estás conectado a "ESP32-MPU6050"?
   - Ve a configuración WiFi del dispositivo
   - Verifica conexión activa

2. ✅ ¿El ESP32 muestra "Servidor web iniciado"?
   ```powershell
   pio device monitor
   # Debe mostrar: "Servidor web iniciado"
   ```

3. ✅ ¿Tienes datos móviles activos?
   - **Desactiva datos móviles** en el celular
   - Solo usa WiFi

4. ✅ Prueba URLs alternativas:
   ```
   http://192.168.4.1
   http://192.168.4.1/
   ```

---

### 🔧 Comandos de Diagnóstico

#### Ver información del ESP32:
```powershell
pio device monitor

# Presiona el botón RESET del ESP32
# Verás:
# - Versión del firmware
# - Estado del MPU6050
# - IP del Access Point
# - Estado del servidor web
```

#### Verificar compilación sin subir:
```powershell
pio run --verbose
```

#### Ver tamaño exacto del firmware:
```powershell
pio run --target size
```

#### Limpiar completamente y recompilar:
```powershell
pio run --target clean
Remove-Item -Recurse -Force .pio
pio run
```

---

### 📞 Obtener Más Ayuda

Si los problemas persisten:

1. **Captura salida completa:**
   ```powershell
   pio run --target upload --target monitor > log.txt 2>&1
   ```

2. **Información del sistema:**
   ```powershell
   pio system info
   ```

3. **Versión de PlatformIO:**
   ```powershell
   pio --version
   ```

## 📄 Licencia

MIT License - Úsalo libremente para tus proyectos

## 🤝 Contribuciones

¡Mejoras y sugerencias son bienvenidas!

---

**Proyecto inspirado en MATLAB_RaspberryPi**
