"""
Script de configuración inicial del proyecto ESP32 MPU6050
Ejecuta este script después de clonar el repositorio
"""

import subprocess
import sys
import os

def print_header(text):
    print("\n" + "="*60)
    print(f"  {text}")
    print("="*60)

def run_command(cmd, description):
    print(f"\n→ {description}...")
    try:
        result = subprocess.run(cmd, shell=True, check=True, capture_output=True, text=True)
        print(f"✓ {description} completado")
        return True
    except subprocess.CalledProcessError as e:
        print(f"✗ Error: {e}")
        return False

def check_platformio():
    """Verificar si PlatformIO está instalado"""
    try:
        subprocess.run(["pio", "--version"], capture_output=True, check=True)
        return True
    except:
        return False

def main():
    print_header("ESP32 MPU6050 - Configuración Inicial")
    
    # Verificar Python
    print(f"\nPython detectado: {sys.version}")
    
    # 1. Verificar/Instalar librerías Python
    print_header("1. Instalación de dependencias Python")
    
    packages = ["requests", "pandas", "numpy", "matplotlib"]
    
    for package in packages:
        run_command(
            f"pip install {package}",
            f"Instalando {package}"
        )
    
    print("\n✓ Todas las dependencias Python instaladas")
    
    # 2. Verificar PlatformIO
    print_header("2. Verificación de PlatformIO")
    
    if check_platformio():
        print("✓ PlatformIO CLI detectado")
        
        # Compilar proyecto
        if os.path.exists("platformio.ini"):
            print("\n¿Deseas compilar el proyecto ahora? (s/n): ", end="")
            if input().lower() == 's':
                run_command(
                    "pio run",
                    "Compilando proyecto para ESP32"
                )
    else:
        print("⚠ PlatformIO CLI no detectado")
        print("Por favor instala PlatformIO IDE en VS Code:")
        print("  1. Abre VS Code")
        print("  2. Extensions (Ctrl+Shift+X)")
        print("  3. Busca 'PlatformIO IDE'")
        print("  4. Instala la extensión")
    
    # 3. Crear estructura de directorios
    print_header("3. Verificación de estructura de proyecto")
    
    dirs = ["python_client", "docs", "data"]
    for dir_name in dirs:
        if not os.path.exists(dir_name):
            os.makedirs(dir_name)
            print(f"✓ Creado directorio: {dir_name}")
        else:
            print(f"✓ Directorio existe: {dir_name}")
    
    # 4. Resumen
    print_header("Configuración Completada")
    
    print("\n📋 Próximos pasos:")
    print("\n1. Hardware:")
    print("   - Conecta el MPU6050 al ESP32 (ver INICIO_RAPIDO.md)")
    print("   - VCC→3.3V, GND→GND, SCL→22, SDA→21")
    
    print("\n2. Programar ESP32:")
    print("   - Conecta ESP32 por USB")
    print("   - En VS Code: Click en Upload (→)")
    print("   - O ejecuta: pio run --target upload")
    
    print("\n3. Conectar:")
    print("   - WiFi SSID: ESP32-MPU6050")
    print("   - Password: 12345678")
    print("   - URL: http://192.168.4.1")
    
    print("\n4. Probar Python:")
    print("   - cd python_client")
    print("   - python ejemplo_rapido.py")
    
    print("\n📖 Documentación:")
    print("   - INICIO_RAPIDO.md - Guía paso a paso")
    print("   - README.md - Documentación completa")
    print("   - docs/CONEXIONES.md - Diagramas de conexión")
    
    print("\n" + "="*60)
    print("  ¡Configuración completada! 🎉")
    print("="*60 + "\n")

if __name__ == "__main__":
    main()
