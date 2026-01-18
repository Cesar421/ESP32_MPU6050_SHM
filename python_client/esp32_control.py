"""
Script de ejemplo para conectarse al ESP32 MPU6050 Data Logger
Compatible con el entorno CesPy13MLDS

Este script replica la funcionalidad del proyecto MATLAB_RaspberryPi
pero para ESP32 con MPU6050
"""

import requests
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import time
from datetime import datetime
import os


class ESP32MPU6050:
    """
    Clase para interactuar con el ESP32 MPU6050 Data Logger
    Similar a las funciones del proyecto MATLAB_RaspberryPi
    """
    
    def __init__(self, ip="192.168.4.1"):
        """
        Inicializar conexión con ESP32
        
        Args:
            ip: Dirección IP del ESP32 (default: 192.168.4.1)
        """
        self.ip = ip
        self.base_url = f"http://{ip}"
        self.measuring = False
        
    def network_info(self):
        """
        Obtener información de red del ESP32
        Similar a Func_NetworkInfo.m
        """
        try:
            status = self.get_status()
            print("=" * 50)
            print("ESP32 MPU6050 - Información de Red")
            print("=" * 50)
            print(f"IP Address: {self.ip}")
            print(f"Status: Connected")
            print(f"Uptime: {status['uptime']/1000:.2f} seconds")
            print(f"Free Heap: {status['freeHeap']/1024:.2f} KB")
            print("=" * 50)
            return True
        except Exception as e:
            print(f"Error conectando al ESP32: {e}")
            return False
    
    def get_status(self):
        """
        Obtener estado del sistema
        Similar a Func_GetStatus.m
        
        Returns:
            dict: Estado del sistema
        """
        response = requests.get(f"{self.base_url}/api/status", timeout=5)
        return response.json()
    
    def is_measuring(self):
        """
        Verificar si está midiendo
        Similar a Func_IsMeasuring.m
        
        Returns:
            bool: True si está midiendo
        """
        status = self.get_status()
        return status['measuring']
    
    def get_current_data(self):
        """
        Obtener datos actuales del sensor
        
        Returns:
            dict: Datos actuales
        """
        response = requests.get(f"{self.base_url}/api/current", timeout=5)
        return response.json()
    
    def start_measurement(self, duration=60, sample_rate=100):
        """
        Iniciar medición con duración específica
        Similar a Func_StartMeasurementDuration.m
        
        Args:
            duration: Duración en segundos
            sample_rate: Frecuencia de muestreo en Hz
        """
        params = {
            "duration": duration,
            "rate": sample_rate
        }
        response = requests.get(f"{self.base_url}/api/start", params=params)
        
        if response.status_code == 200:
            self.measuring = True
            print(f"✓ Medición iniciada: {duration}s @ {sample_rate}Hz")
            return True
        else:
            print("✗ Error iniciando medición")
            return False
    
    def stop_measurement(self):
        """
        Detener medición
        Similar a Func_StopMeasurementStartStop.m
        """
        response = requests.get(f"{self.base_url}/api/stop")
        
        if response.status_code == 200:
            self.measuring = False
            print("✓ Medición detenida")
            return True
        else:
            print("✗ Error deteniendo medición")
            return False
    
    def list_files(self):
        """
        Listar archivos disponibles
        
        Returns:
            list: Lista de tuplas (nombre, tamaño)
        """
        response = requests.get(f"{self.base_url}/api/files")
        files_text = response.text
        
        if not files_text:
            return []
        
        files = []
        for file_info in files_text.split(';'):
            if file_info:
                name, size = file_info.split(',')
                files.append((name, int(size)))
        
        return files
    
    def download_file(self, filename, output_path=None):
        """
        Descargar archivo del ESP32
        Similar a Func_DownloadFile.m
        
        Args:
            filename: Nombre del archivo en el ESP32
            output_path: Ruta de salida (opcional)
        
        Returns:
            str: Ruta del archivo descargado
        """
        if output_path is None:
            output_path = filename.replace('/', '')
        
        params = {"file": filename}
        response = requests.get(f"{self.base_url}/api/download", params=params)
        
        if response.status_code == 200:
            with open(output_path, 'wb') as f:
                f.write(response.content)
            print(f"✓ Archivo descargado: {output_path}")
            return output_path
        else:
            print(f"✗ Error descargando archivo: {filename}")
            return None
    
    def download_latest_file(self, output_path="latest_data.csv"):
        """
        Descargar el archivo más reciente
        
        Returns:
            str: Ruta del archivo descargado
        """
        files = self.list_files()
        
        if not files:
            print("No hay archivos disponibles")
            return None
        
        latest_file = files[-1][0]  # Último archivo
        return self.download_file(latest_file, output_path)
    
    def delete_file(self, filename):
        """
        Eliminar archivo del ESP32
        
        Args:
            filename: Nombre del archivo a eliminar
        """
        params = {"file": filename}
        response = requests.get(f"{self.base_url}/api/delete", params=params)
        
        if response.status_code == 200:
            print(f"✓ Archivo eliminado: {filename}")
            return True
        else:
            print(f"✗ Error eliminando archivo: {filename}")
            return False
    
    def plot_data(self, csv_file):
        """
        Graficar datos del archivo CSV
        Similar a Func_PlotData.m
        
        Args:
            csv_file: Ruta del archivo CSV
        """
        # Leer datos
        df = pd.read_csv(csv_file)
        
        # Convertir timestamp a tiempo relativo en segundos
        time_s = (df['Timestamp'] - df['Timestamp'].iloc[0]) / 1000.0
        
        # Crear figura con subplots
        fig, axes = plt.subplots(3, 1, figsize=(12, 10))
        fig.suptitle('ESP32 MPU6050 - Datos de Medición', fontsize=16)
        
        # Acelerómetro
        axes[0].plot(time_s, df['Accel_X'], label='X', linewidth=1)
        axes[0].plot(time_s, df['Accel_Y'], label='Y', linewidth=1)
        axes[0].plot(time_s, df['Accel_Z'], label='Z', linewidth=1)
        axes[0].set_ylabel('Aceleración (m/s²)')
        axes[0].set_title('Acelerómetro')
        axes[0].legend()
        axes[0].grid(True, alpha=0.3)
        
        # Giroscopio
        axes[1].plot(time_s, df['Gyro_X'], label='X', linewidth=1)
        axes[1].plot(time_s, df['Gyro_Y'], label='Y', linewidth=1)
        axes[1].plot(time_s, df['Gyro_Z'], label='Z', linewidth=1)
        axes[1].set_ylabel('Velocidad Angular (rad/s)')
        axes[1].set_title('Giroscopio')
        axes[1].legend()
        axes[1].grid(True, alpha=0.3)
        
        # Temperatura
        axes[2].plot(time_s, df['Temperature'], color='red', linewidth=1)
        axes[2].set_xlabel('Tiempo (s)')
        axes[2].set_ylabel('Temperatura (°C)')
        axes[2].set_title('Temperatura')
        axes[2].grid(True, alpha=0.3)
        
        plt.tight_layout()
        plt.savefig('mpu6050_data.png', dpi=300)
        plt.show()
        
        print("✓ Gráfico guardado como: mpu6050_data.png")
    
    def quick_plot(self):
        """
        Descarga y grafica el último archivo
        Similar a Func_QuickPlot.m
        """
        print("Descargando último archivo...")
        csv_file = self.download_latest_file()
        
        if csv_file:
            print("Generando gráficos...")
            self.plot_data(csv_file)
    
    def analyze_data(self, csv_file):
        """
        Análisis estadístico de los datos
        Similar a Func_ConvertData.m
        
        Args:
            csv_file: Ruta del archivo CSV
        
        Returns:
            dict: Estadísticas de los datos
        """
        df = pd.read_csv(csv_file)
        
        stats = {
            'accel': {
                'x': {'mean': df['Accel_X'].mean(), 'std': df['Accel_X'].std()},
                'y': {'mean': df['Accel_Y'].mean(), 'std': df['Accel_Y'].std()},
                'z': {'mean': df['Accel_Z'].mean(), 'std': df['Accel_Z'].std()},
            },
            'gyro': {
                'x': {'mean': df['Gyro_X'].mean(), 'std': df['Gyro_X'].std()},
                'y': {'mean': df['Gyro_Y'].mean(), 'std': df['Gyro_Y'].std()},
                'z': {'mean': df['Gyro_Z'].mean(), 'std': df['Gyro_Z'].std()},
            },
            'temp': {
                'mean': df['Temperature'].mean(),
                'std': df['Temperature'].std(),
                'min': df['Temperature'].min(),
                'max': df['Temperature'].max()
            },
            'samples': len(df)
        }
        
        return stats


# ==================== FUNCIONES DE UTILIDAD ====================

def wait_for_measurement(esp32, check_interval=1):
    """
    Esperar hasta que termine la medición
    
    Args:
        esp32: Instancia de ESP32MPU6050
        check_interval: Intervalo de verificación en segundos
    """
    print("Esperando finalización de medición...")
    
    while esp32.is_measuring():
        status = esp32.get_status()
        remaining = status.get('remaining', 0)
        samples = status['samples']
        print(f"  Muestras: {samples} | Tiempo restante: {remaining}s", end='\r')
        time.sleep(check_interval)
    
    print("\n✓ Medición completada")


def create_sensor_network(ip_addresses):
    """
    Crear red de múltiples sensores ESP32
    Similar a Func_CreateSensor.m para múltiples dispositivos
    
    Args:
        ip_addresses: Lista de direcciones IP
    
    Returns:
        list: Lista de instancias ESP32MPU6050
    """
    sensors = []
    
    for ip in ip_addresses:
        sensor = ESP32MPU6050(ip)
        if sensor.network_info():
            sensors.append(sensor)
        else:
            print(f"✗ No se pudo conectar a {ip}")
    
    return sensors


# ==================== EJEMPLO DE USO ====================

def main():
    """
    Ejemplo completo de uso del sistema
    """
    print("=" * 60)
    print("ESP32 MPU6050 Data Logger - Script de Control")
    print("=" * 60)
    print()
    
    # Conectar al ESP32
    esp32 = ESP32MPU6050(ip="192.168.4.1")
    
    # Verificar conexión
    if not esp32.network_info():
        print("Error: No se pudo conectar al ESP32")
        return
    
    print()
    
    # Mostrar estado
    status = esp32.get_status()
    print(f"Estado actual:")
    print(f"  - Midiendo: {'Sí' if status['measuring'] else 'No'}")
    print(f"  - Muestras: {status['samples']}/{status['maxSamples']}")
    print(f"  - Frecuencia: {status['sampleRate']} Hz")
    print()
    
    # Mostrar datos actuales
    print("Datos en tiempo real:")
    data = esp32.get_current_data()
    print(f"  Acelerómetro: X={data['accel']['x']:.3f}, "
          f"Y={data['accel']['y']:.3f}, Z={data['accel']['z']:.3f} m/s²")
    print(f"  Giroscopio: X={data['gyro']['x']:.3f}, "
          f"Y={data['gyro']['y']:.3f}, Z={data['gyro']['z']:.3f} rad/s")
    print(f"  Temperatura: {data['temp']:.2f} °C")
    print()
    
    # Menú interactivo
    while True:
        print("\n" + "=" * 60)
        print("Opciones:")
        print("  1. Iniciar medición")
        print("  2. Detener medición")
        print("  3. Listar archivos")
        print("  4. Descargar último archivo")
        print("  5. Graficar último archivo")
        print("  6. Analizar datos")
        print("  7. Eliminar todos los archivos")
        print("  0. Salir")
        print("=" * 60)
        
        opcion = input("Selecciona una opción: ")
        
        if opcion == "1":
            duration = int(input("Duración (segundos): "))
            rate = int(input("Frecuencia (Hz) [10/50/100/200]: "))
            esp32.start_measurement(duration, rate)
            
            # Esperar a que termine
            wait_for_measurement(esp32)
            
        elif opcion == "2":
            esp32.stop_measurement()
            
        elif opcion == "3":
            files = esp32.list_files()
            print(f"\nArchivos disponibles ({len(files)}):")
            for name, size in files:
                print(f"  - {name} ({size/1024:.2f} KB)")
                
        elif opcion == "4":
            csv_file = esp32.download_latest_file()
            if csv_file:
                print(f"Archivo guardado: {csv_file}")
                
        elif opcion == "5":
            esp32.quick_plot()
            
        elif opcion == "6":
            files = esp32.list_files()
            if files:
                csv_file = esp32.download_latest_file("temp_analysis.csv")
                stats = esp32.analyze_data(csv_file)
                
                print("\n" + "=" * 60)
                print("ESTADÍSTICAS")
                print("=" * 60)
                print(f"Muestras totales: {stats['samples']}")
                print(f"\nAcelerómetro (m/s²):")
                print(f"  X: {stats['accel']['x']['mean']:.3f} ± {stats['accel']['x']['std']:.3f}")
                print(f"  Y: {stats['accel']['y']['mean']:.3f} ± {stats['accel']['y']['std']:.3f}")
                print(f"  Z: {stats['accel']['z']['mean']:.3f} ± {stats['accel']['z']['std']:.3f}")
                print(f"\nGiroscopio (rad/s):")
                print(f"  X: {stats['gyro']['x']['mean']:.4f} ± {stats['gyro']['x']['std']:.4f}")
                print(f"  Y: {stats['gyro']['y']['mean']:.4f} ± {stats['gyro']['y']['std']:.4f}")
                print(f"  Z: {stats['gyro']['z']['mean']:.4f} ± {stats['gyro']['z']['std']:.4f}")
                print(f"\nTemperatura (°C):")
                print(f"  Media: {stats['temp']['mean']:.2f} ± {stats['temp']['std']:.2f}")
                print(f"  Rango: {stats['temp']['min']:.2f} - {stats['temp']['max']:.2f}")
                print("=" * 60)
            else:
                print("No hay archivos para analizar")
                
        elif opcion == "7":
            confirm = input("¿Eliminar TODOS los archivos? (s/n): ")
            if confirm.lower() == 's':
                files = esp32.list_files()
                for name, _ in files:
                    esp32.delete_file(name)
                    
        elif opcion == "0":
            print("¡Hasta luego!")
            break


if __name__ == "__main__":
    main()
