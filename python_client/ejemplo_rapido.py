"""
Ejemplo rápido de uso del ESP32 MPU6050
"""

from esp32_control import ESP32MPU6050
import time

# Conectar al ESP32
print("Conectando al ESP32...")
esp32 = ESP32MPU6050(ip="192.168.4.1")

# Verificar conexión
if not esp32.network_info():
    print("ERROR: No se pudo conectar")
    exit(1)

print("\n✓ Conectado exitosamente\n")

# Mostrar datos en tiempo real por 5 segundos
print("Mostrando datos en tiempo real (5 segundos)...")
for i in range(5):
    data = esp32.get_current_data()
    print(f"\nSegundo {i+1}:")
    print(f"  Aceleración: X={data['accel']['x']:7.3f}  Y={data['accel']['y']:7.3f}  Z={data['accel']['z']:7.3f} m/s²")
    print(f"  Giroscopio:  X={data['gyro']['x']:7.3f}  Y={data['gyro']['y']:7.3f}  Z={data['gyro']['z']:7.3f} rad/s")
    print(f"  Temperatura: {data['temp']:.2f} °C")
    time.sleep(1)

print("\n" + "="*60)
print("Ejemplo completado!")
print("="*60)
print("\nPara más opciones, ejecuta: python esp32_control.py")
