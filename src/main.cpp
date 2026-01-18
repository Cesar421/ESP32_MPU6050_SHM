/*
 * ESP32 MPU6050 Data Logger con WiFi Access Point
 * Sistema de medición con sensor MPU6050 y servidor web
 * Inspirado en el proyecto MATLAB_RaspberryPi
 */

#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <LittleFS.h>

// ==================== CONFIGURACIÓN ====================
// WiFi Access Point
const char* ap_ssid = "ESP32-MPU6050";
const char* ap_password = "12345678";  // Mínimo 8 caracteres

// Pines I2C para MPU6050
#define SDA_PIN 21
#define SCL_PIN 22

// ==================== OBJETOS GLOBALES ====================
Adafruit_MPU6050 mpu;
AsyncWebServer server(80);

// ==================== VARIABLES DE MEDICIÓN ====================
bool isMeasuring = false;
unsigned long measurementStartTime = 0;
unsigned long measurementDuration = 0;  // en segundos
unsigned long lastSampleTime = 0;
int sampleRate = 100;  // Hz
int sampleInterval = 1000 / sampleRate;  // ms

// Estructuras de datos
struct MPUData {
    unsigned long timestamp;
    float accel_x, accel_y, accel_z;
    float gyro_x, gyro_y, gyro_z;
    float temp;
};

// Estructura binaria para almacenamiento eficiente (28 bytes por muestra)
struct __attribute__((packed)) BinaryData {
    uint32_t timestamp;     // 4 bytes
    float accel_x;          // 4 bytes
    float accel_y;          // 4 bytes
    float accel_z;          // 4 bytes
    float gyro_x;           // 4 bytes
    float gyro_y;           // 4 bytes
    float gyro_z;           // 4 bytes
};  // Total: 28 bytes (vs ~100 bytes en CSV)

const int MAX_SAMPLES = 10000;
MPUData* dataBuffer;
int dataCount = 0;
String currentFileName = "";

// Declaración adelantada (forward declaration)
String getIndexHTML();

// ==================== FUNCIONES DE SISTEMA DE ARCHIVOS ====================
void initFileSystem() {
    if (!LittleFS.begin(true)) {
        Serial.println("Error montando LittleFS");
        return;
    }
    Serial.println("LittleFS montado correctamente");
}

void saveDataToFile() {
    if (dataCount == 0) return;
    
    // Guardar en formato BINARIO para eficiencia (28 bytes vs ~100 bytes CSV)
    String filename = "/data_" + String(millis()) + ".bin";
    currentFileName = filename;
    
    File file = LittleFS.open(filename, FILE_WRITE);
    if (!file) {
        Serial.println("Error creando archivo");
        return;
    }
    
    // Guardar datos en formato binario
    for (int i = 0; i < dataCount; i++) {
        BinaryData binData;
        binData.timestamp = dataBuffer[i].timestamp;
        binData.accel_x = dataBuffer[i].accel_x;
        binData.accel_y = dataBuffer[i].accel_y;
        binData.accel_z = dataBuffer[i].accel_z;
        binData.gyro_x = dataBuffer[i].gyro_x;
        binData.gyro_y = dataBuffer[i].gyro_y;
        binData.gyro_z = dataBuffer[i].gyro_z;
        
        file.write((uint8_t*)&binData, sizeof(BinaryData));
    }
    
    file.close();
    
    // Calcular espacio ahorrado
    int binarySize = dataCount * sizeof(BinaryData);
    int csvSize = dataCount * 100; // Aproximadamente
    Serial.println("Datos guardados en: " + filename);
    Serial.printf("Formato binario: %d bytes (vs CSV: ~%d bytes) - Ahorro: %.1f%%\n", 
                  binarySize, csvSize, (1.0 - (float)binarySize/csvSize) * 100);
}

String listFiles() {
    String fileList = "";
    File root = LittleFS.open("/");
    File file = root.openNextFile();
    
    while (file) {
        if (!file.isDirectory()) {
            fileList += String(file.name()) + "," + String(file.size()) + ";";
        }
        file = root.openNextFile();
    }
    
    return fileList;
}

// ==================== FUNCIONES MPU6050 ====================
booConvertir archivo binario a CSV (streaming para ahorrar memoria)
String convertBinaryToCSV(String binaryFilename, AsyncWebServerRequest *request) {
    if (!LittleFS.exists(binaryFilename)) {
        return "";
    }
    
    File binFile = LittleFS.open(binaryFilename, FILE_READ);
    if (!binFile) {
        return "";
    }
    
    // Crear respuesta CSV en streaming
    AsyncWebServerResponse *response = request->beginChunkedResponse(
        "text/csv", 
        [binFile](uint8_t *buffer, size_t maxLen, size_t index) mutable -> size_t {
            
            // Primera llamada: enviar encabezado CSV
            if (index == 0) {
                String header = "Timestamp,Accel_X,Accel_Y,Accel_Z,Gyro_X,Gyro_Y,Gyro_Z\n";
                size_t len = header.length();
                memcpy(buffer, header.c_str(), len);
                return len;
            }
            
            // Leer y convertir datos binarios a CSV
            BinaryData binData;
            size_t bytesWritten = 0;
            
            while (binFile.available() && bytesWritten < maxLen - 200) {
                size_t bytesRead = binFile.read((uint8_t*)&binData, sizeof(BinaryData));
                
                if (bytesRead != sizeof(BinaryData)) {
                    break;
                }
                
                char line[200];
                snprintf(line, sizeof(line), "%lu,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f\n",
                        (unsigned long)binData.timestamp,
                        binData.accel_x, binData.accel_y, binData.accel_z,
                        binData.gyro_x, binData.gyro_y, binData.gyro_z);
                
                size_t lineLen = strlen(line);
                memcpy(buffer + bytesWritten, line, lineLen);
                bytesWritten += lineLen;
            }
            
            if (!binFile.available()) {
                binFile.close();
            }
            
            return bytesWritten;
        }
    );
    
    // Generar nombre de archivo CSV para descarga
    String csvFilename = binaryFilename;
    csvFilename.replace(".bin", ".csv");
    csvFilename = csvFilename.substring(1); // Quitar el '/'
    
    response->addHeader("Content-Disposition", "attachment; filename=" + csvFilename);
    request->send(response);
    
    return "streaming";
}

// l initMPU6050() {
    Wire.begin(SDA_PIN, SCL_PIN);
    
    if (!mpu.begin()) {
        Serial.println("Error: MPU6050 no encontrado");
        return false;
    }
    
    Serial.println("MPU6050 inicializado correctamente");
    
    // Configuración del sensor
    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
    
    return true;
}

MPUData readMPU6050() {
    MPUData data;
    sensors_event_t accel, gyro, temp;
    
    mpu.getEvent(&accel, &gyro, &temp);
    
    data.timestamp = millis();
    data.accel_x = accel.acceleration.x;
    data.accel_y = accel.acceleration.y;
    data.accel_z = accel.acceleration.z;
    data.gyro_x = gyro.gyro.x;
    data.gyro_y = gyro.gyro.y;
    data.gyro_z = gyro.gyro.z;
    data.temp = temp.temperature;
    
    return data;
}

// ==================== FUNCIONES DE MEDICIÓN ====================
void startMeasurement(unsigned long durationSeconds) {
    isMeasuring = true;
    measurementStartTime = millis();
    measurementDuration = durationSeconds * 1000;
    dataCount = 0;
    lastSampleTime = 0;
    
    Serial.println("Medición iniciada - Duración: " + String(durationSeconds) + " segundos");
}

void stopMeasurement() {
    if (isMeasuring) {
        isMeasuring = false;
        saveDataToFile();
        Serial.println("Medición detenida - Muestras: " + String(dataCount));
    }
}

void processMeasurement() {
    if (!isMeasuring) return;
    
    unsigned long currentTime = millis();
    
    // Verificar duración
    if (measurementDuration > 0 && (currentTime - measurementStartTime >= measurementDuration)) {
        stopMeasurement();
        return;
    }
    
    // Tomar muestra según el intervalo
    if (currentTime - lastSampleTime >= sampleInterval) {
        if (dataCount < MAX_SAMPLES) {
            dataBuffer[dataCount] = readMPU6050();
            dataCount++;
            lastSampleTime = currentTime;
        } else {
            Serial.println("Buffer lleno - deteniendo medición");
            stopMeasurement();
        }
    }
}

// ==================== CONFIGURACIÓN SERVIDOR WEB ====================
void setupWebServer() {
    // Página principal
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/html", getIndexHTML());
    });
    
    // API: Estado del sistema
    server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request){
        StaticJsonDocument<512> doc;
        doc["measuring"] = is binario
    server.on("/api/download", HTTP_GET, [](AsyncWebServerRequest *request){
        if (request->hasParam("file")) {
            String filename = request->getParam("file")->value();
            
            if (LittleFS.exists(filename)) {
                request->send(LittleFS, filename, "application/octet-stream", true);
            } else {
                request->send(404, "text/plain", "Archivo no encontrado");
            }
        } else {
            request->send(400, "text/plain", "Parámetro 'file' requerido");
        }
    });
    
    // API: Convertir y descargar como CSV
    server.on("/api/convert-to-csv", HTTP_GET, [](AsyncWebServerRequest *request){
        if (request->hasParam("file")) {
            String filename = request->getParam("file")->value();
            
            if (filename.endsWith(".bin")) {
                convertBinaryToCSV(filename, request);
            } else {
                request->send(400, "text/plain", "Solo archivos .bin pueden ser convertidos
            doc["remaining"] = (measurementDuration - (millis() - measurementStartTime)) / 1000;
        }
        
        String json;
        serializeJson(doc, json);
        request->send(200, "application/json", json);
    });
    
    // API: Datos actuales en tiempo real
    server.on("/api/current", HTTP_GET, [](AsyncWebServerRequest *request){
        MPUData data = readMPU6050();
        
        StaticJsonDocument<512> doc;
        doc["timestamp"] = data.timestamp;
        doc["accel"]["x"] = data.accel_x;
        doc["accel"]["y"] = data.accel_y;
        doc["accel"]["z"] = data.accel_z;
        doc["gyro"]["x"] = data.gyro_x;
        doc["gyro"]["y"] = data.gyro_y;
        doc["gyro"]["z"] = data.gyro_z;
        doc["temp"] = data.temp;
        
        String json;
        serializeJson(doc, json);
        request->send(200, "application/json", json);
    });
    
    // API: Iniciar medición
    server.on("/api/start", HTTP_GET, [](AsyncWebServerRequest *request){
        int duration = 60;  // Por defecto 60 segundos
        
        if (request->hasParam("duration")) {
            duration = request->getParam("duration")->value().toInt();
        }
        
        if (request->hasParam("rate")) {
            sampleRate = request->getParam("rate")->value().toInt();
            sampleInterval = 1000 / sampleRate;
        }
        
        startMeasurement(duration);
        request->send(200, "text/plain", "Medición iniciada");
    });
    
    // API: Detener medición
    server.on("/api/stop", HTTP_GET, [](AsyncWebServerRequest *request){
        stopMeasurement();
        request->send(200, "text/plain", "Medición detenida");
    });
    
    // API: Listar archivos
    server.on("/api/files", HTTP_GET, [](AsyncWebServerRequest *request){
        String files = listFiles();
        request->send(200, "text/plain", files);
    });
    
    // API: Descargar archivo
    server.on("/api/download", HTTP_GET, [](AsyncWebServerRequest *request){
        if (request->hasParam("file")) {
            String filename = request->getParam("file")->value();
            
            if (LittleFS.exists(filename)) {
                request->send(LittleFS, filename, "text/csv", true);
            } else {
                request->send(404, "text/plain", "Archivo no encontrado");
            }
        } else {
            request->send(400, "text/plain", "Parámetro 'file' requerido");
        }
    });
    
    // API: Eliminar archivo
    server.on("/api/delete", HTTP_GET, [](AsyncWebServerRequest *request){
        if (request->hasParam("file")) {
            String filename = request->getParam("file")->value();
            
            if (LittleFS.remove(filename)) {
                request->send(200, "text/plain", "Archivo eliminado");
            } else {
                request->send(404, "text/plain", "Error eliminando archivo");
            }
        } else {
            request->send(400, "text/plain", "Parámetro 'file' requerido");
        }
    });
    
    server.begin();
    Serial.println("Servidor web iniciado");
}

// ==================== PÁGINA HTML ====================
String getIndexHTML() {
    return R"rawliteral(
<!DOCTYPE html>
<html lang="es">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 MPU6050 Logger</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            padding: 20px;
        }
        .container {
            max-width: 1200px;
            margin: 0 auto;
        }
        .header {
            background: white;
            padding: 20px;
            border-radius: 10px;
            box-shadow: 0 4px 6px rgba(0,0,0,0.1);
            margin-bottom: 20px;
            text-align: center;
        }
        h1 { color: #667eea; margin-bottom: 10px; }
        .status-badge {
            display: inline-block;
            padding: 8px 16px;
            border-radius: 20px;
            font-weight: bold;
            margin: 10px 5px;
        }
        .badge-measuring { background: #4ade80; color: white; }
        .badge-idle { background: #94a3b8; color: white; }
        .grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
            gap: 20px;
            margin-bottom: 20px;
        }
        .card {
            background: white;
            padding: 20px;
            border-radius: 10px;
            box-shadow: 0 4px 6px rgba(0,0,0,0.1);
        }
        .card h2 {
            color: #667eea;
            margin-bottom: 15px;
            font-size: 1.3em;
            border-bottom: 2px solid #667eea;
            padding-bottom: 10px;
        }
        table {
            width: 100%;
            border-collapse: collapse;
        }
        th, td {
            padding: 12px;
            text-align: left;
            border-bottom: 1px solid #e2e8f0;
        }
        th {
            background: #f8fafc;
            font-weight: 600;
            color: #475569;
        }
        tr:hover { background: #f8fafc; }
        .value {
            font-size: 1.5em;
            font-weight: bold;
            color: #667eea;
        }
        .controls {
            display: flex;
            gap: 10px;
            flex-wrap: wrap;
        }
        button {
            padding: 12px 24px;
            border: none;
            border-radius: 6px;
            font-size: 1em;
            cursor: pointer;
            transition: all 0.3s;
            font-weight: 600;
        }
        .btn-primary {
            background: #667eea;
            color: white;
        }
        .btn-primary:hover { background: #5568d3; }
        .btn-danger {
            background: #ef4444;
            color: white;
        }
        .btn-danger:hover { background: #dc2626; }
        .btn-success {
            background: #10b981;
            color: white;
        }
        .btn-success:hover { background: #059669; }
        input, select {
            padding: 10px;
            border: 2px solid #e2e8f0;
            border-radius: 6px;
            font-size: 1em;
        }
        .file-item {
            display: flex;
            justify-content: space-between;
            align-items: center;
            padding: 10px;
            border-bottom: 1px solid #e2e8f0;
        }
        .file-actions button {
            padding: 6px 12px;
            margin-left: 5px;
            font-size: 0.9em;
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>🚀 ESP32 MPU6050 Data Logger</h1>
            <div id="status-container">
                <span class="status-badge badge-idle" id="status-badge">Inactivo</span>
                <span class="status-badge badge-idle">Muestras: <span id="sample-count">0</span></span>
            </div>
        </div>

        <div class="grid">
            <div class="card">
                <h2>⚙️ Control de Medición</h2>
                <div class="controls">
                    <label>Duración (s):</label>
                    <input type="number" id="duration" value="60" min="1" max="3600">
                    <label>Frecuencia (Hz):</label>
                    <select id="sampleRate">
                        <option value="10">10 Hz</option>
                        <option value="50">50 Hz</option>
                        <option value="100" selected>100 Hz</option>
                        <option value="200">200 Hz</option>
                    </select>
                </div>
                <div class="controls" style="margin-top: 15px;">
                    <button class="btn-primary" onclick="startMeasurement()">▶️ Iniciar</button>
                    <button class="btn-danger" onclick="stopMeasurement()">⏹️ Detener</button>
                </div>
            </div>

            <div class="card">
                <h2>📊 Datos en Tiempo Real</h2>
                <table>
                    <tr><th>Parámetro</th><th>Valor</th></tr>
                    <tr><td>Temperatura</td><td><span id="temp" class="value">--</span> °C</td></tr>
                    <tr><td>Estado</td><td><span id="measuring-status">Inactivo</span></td></tr>
                    <tr><td>Tiempo transcurrido</td><td><span id="elapsed">0</span> s</td></tr>
                </table>
            </div>
        </div>

        <div class="grid">
            <div class="card">
                <h2>📈 Acelerómetro (m/s²)</h2>
                <table>
                    <tr><th>Eje</th><th>Valor</th></tr>
                    <tr><td>X</td><td><span id="accel-x" class="value">0.00</span></td></tr>
                    <tr><td>Y</td><td><span id="accel-y" class="value">0.00</span></td></tr>
                    <tr><td>Z</td><td><span id="accel-z" class="value">0.00</span></td></tr>
                </table>
            </div>

            <div class="card">
                <h2>🔄 Giroscopio (rad/s)</h2>
                <table>
                    <tr><th>Eje</th><th>Valor</th></tr>
                    <tr><td>X</td><td><span id="gyro-x" class="value">0.00</span></td></tr>
                    <tr><td>Y</td><td><span id="gyro-y" class="value">0.00</span></td></tr>
                    <tr><td>Z</td><td><span id="gyro-z" class="value">0.00</span></td></tr>
                </table>
            </div>
        </div>

        <div class="card">
            <h2>Archivos de Datos</h2>
            <button class="btn-success" onclick="refreshFiles()" style="margin-bottom: 15px;">Actualizar Lista</button>
            <div id="file-list"></div>
        </div>
    </div>

    <script>
        // Actualizar datos cada 200ms
        setInterval(updateData, 200);
        setInterval(updateStatus, 1000);
        
        // Cargar archivos al inicio
        refreshFiles();

        async function updateData() {
            try {
                const response = await fetch('/api/current');
                const data = await response.json();
                
                document.getElementById('accel-x').textContent = data.accel.x.toFixed(3);
                document.getElementById('accel-y').textContent = data.accel.y.toFixed(3);
                document.getElementById('accel-z').textContent = data.accel.z.toFixed(3);
                document.getElementById('gyro-x').textContent = data.gyro.x.toFixed(3);
                document.getElementById('gyro-y').textContent = data.gyro.y.toFixed(3);
                document.getElementById('gyro-z').textContent = data.gyro.z.toFixed(3);
                document.getElementById('temp').textContent = data.temp.toFixed(2);
            } catch (error) {
                console.error('Error actualizando datos:', error);
            }
        }

        async function updateStatus() {
            try {
                const response = await fetch('/api/status');
                const status = await response.json();
                
                document.getElementById('sample-count').textContent = status.samples;
                
                const badge = document.getElementById('status-badge');
                const measuringStatus = document.getElementById('measuring-status');
                
                if (status.measuring) {
                    badge.textContent = 'Midiendo';
                    badge.className = 'status-badge badge-measuring';
                    measuringStatus.textContent = 'Activo';
                    document.getElementById('elapsed').textContent = status.elapsed || 0;
                } else {
                    badge.textContent = 'Inactivo';
                    badge.className = 'status-badge badge-idle';
                    measuringStatus.textContent = 'Inactivo';
                    document.getElementById('elapsed').textContent = 0;
                }
            } catch (error) {
                console.error('Error actualizando estado:', error);
            }
        }

        async function startMeasurement() {
            const duration = document.getElementById('duration').value;
            const rate = document.getElementById('sampleRate').value;
            
            try {
                await fetch(`/api/start?duration=${duration}&rate=${rate}`);
                alert('Medición iniciada');
            } catch (error) {
                alert('Error iniciando medición');
            }
        }

        async function stopMeasurement() {
            try {
                await fetch('/api/stop');
                alert('Medición detenida');
                setTimeout(refreshFiles, 1000);
            } catch (error) {
                alert('Error deteniendo medición');
            }
        }

        async function refreshFiles() {
            try {
                const response = await fetch('/api/files');
                const filesText = await response.text();
                
                const fileList = document.getElementById('file-list');
                fileList.innerHTML = '';
                
                if (!filesText) {
                    fileList.innerHTML = '<p>No hay archivos disponibles</p>';
                    return;
                }
                
                const files = filesText.split(';').filter(f => f);
                
                files.forEach(file => {
                    const [name, size] = file.split(',');
                    const isBinary = name.endsWith('.bin');
                    const sizeKB = (size/1024).toFixed(2);
                    const samples = isBinary ? Math.floor(size / 28) : 'N/A';
                    
                    const fileDiv = document.createElement('div');
                    fileDiv.className = 'file-item';
                    fileDiv.innerHTML = `
                        <div>
                            <strong>${name}</strong><br>
                            <small>${sizeKB} KB - ${isBinary ? 'Binario (' + samples + ' muestras)' : 'Otro formato'}</small>
                        </div>
                        <div class="file-actions">
                            ${isBinary ? '<button class="btn-action" onclick="convertToCSV(\'' + name + '\')">CSV</button>' : ''}
                            <button class="btn-success" onclick="downloadFile('${name}')">BIN</button>
                            <button class="btn-danger" onclick="deleteFile('${name}')">DEL</button>
                        </div>
                    `;
                    fileList.appendChild(fileDiv);
                });
            } catch (error) {
                console.error('Error cargando archivos:', error);
            }
        }

        function downloadFile(filename) {
            window.location.href = `/api/download?file=${filename}`;
        }
        
        function convertToCSV(filename) {
            window.location.href = `/api/convert-to-csv?file=${filename}`;
        }

        async function deleteFile(filename) {
            if (confirm(`¿Eliminar ${filename}?`)) {
                try {
                    await fetch(`/api/delete?file=${filename}`);
                    refreshFiles();
                } catch (error) {
                    alert('Error eliminando archivo');
                }
            }
        }
    </script>
</body>
</html>
)rawliteral";
}

// ==================== SETUP ====================
void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n\n=== ESP32 MPU6050 Data Logger ===");
    
    // Inicializar buffer de datos
    dataBuffer = new MPUData[MAX_SAMPLES];
    
    // Inicializar sistema de archivos
    initFileSystem();
    
    // Inicializar MPU6050
    if (!initMPU6050()) {
        Serial.println("ERROR: No se pudo inicializar el MPU6050");
        while (1) delay(1000);
    }
    
    // Configurar Access Point WiFi
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ap_ssid, ap_password);
    
    IPAddress IP = WiFi.softAPIP();
    Serial.println("\n=== WiFi Access Point ===");
    Serial.println("SSID: " + String(ap_ssid));
    Serial.println("Password: " + String(ap_password));
    Serial.println("IP: " + IP.toString());
    Serial.println("URL: http://" + IP.toString());
    Serial.println("========================\n");
    
    // Configurar servidor web
    setupWebServer();
    
    Serial.println("Sistema listo!");
}

// ==================== LOOP ====================
void loop() {
    processMeasurement();
    delay(1);  // Small delay para evitar watchdog
}
