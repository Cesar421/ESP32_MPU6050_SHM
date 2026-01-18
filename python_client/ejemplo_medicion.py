"""
Ejemplo de medición automatizada
Inicia medición, espera, descarga y grafica
"""

from esp32_control import ESP32MPU6050, wait_for_measurement
import time

print("="*60)
print("ESP32 MPU6050 - Medición Automatizada")
print("="*60)

# Conectar
esp32 = ESP32MPU6050(ip="192.168.4.1")

if not esp32.network_info():
    print("ERROR: No se pudo conectar al ESP32")
    exit(1)

# Configuración de la medición
DURACION = 10  # segundos
FRECUENCIA = 100  # Hz

print(f"\nIniciando medición:")
print(f"  Duración: {DURACION} segundos")
print(f"  Frecuencia: {FRECUENCIA} Hz")
print(f"  Muestras esperadas: ~{DURACION * FRECUENCIA}")
print()

# Iniciar medición
esp32.start_measurement(duration=DURACION, sample_rate=FRECUENCIA)

# Esperar a que termine
wait_for_measurement(esp32, check_interval=0.5)

# Esperar un momento para que se guarde el archivo
time.sleep(2)

# Descargar y graficar
print("\nDescargando datos...")
csv_file = esp32.download_latest_file("medicion_automatica.csv")

if csv_file:
    print("Generando gráficos...")
    esp32.plot_data(csv_file)
    
    print("\nAnalizando datos...")
    stats = esp32.analyze_data(csv_file)
    
    print("\n" + "="*60)
    print("RESUMEN DE MEDICIÓN")
    print("="*60)
    print(f"Muestras recolectadas: {stats['samples']}")
    print(f"Temperatura promedio: {stats['temp']['mean']:.2f} °C")
    print(f"Aceleración Z (gravedad): {stats['accel']['z']['mean']:.2f} ± {stats['accel']['z']['std']:.2f} m/s²")
    print("="*60)
    
    print("\n✓ Proceso completado exitosamente!")
    print(f"  - Datos guardados en: {csv_file}")
    print(f"  - Gráfico guardado en: mpu6050_data.png")
else:
    print("✗ Error descargando archivo")
