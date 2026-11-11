# CRIS AUDIO SUITE

Suite de desarrollo para Windows / FL Studio con dos plugins independientes:

- **CRIS VOX TUNER** — corrección monofónica de afinación en tiempo real (prototipo DSP V1).
- **CRIS VOX ENHANCER** — realce tonal vocal con filtros, saturación armónica y mezcla paralela.

> Estado: base de desarrollo compilable y ampliable. No es todavía un sustituto de Auto-Tune o Melodyne ni una suite comercial validada. El Tuner usa detección de tono por autocorrelación y un pitch shifter de doble cabeza de retardo; puede producir artefactos en voces polifónicas, ruido, consonantes o notas muy graves/agudas. La edición gráfica de notas, la preservación avanzada de formantes y un de-esser dinámico quedan para versiones posteriores.

## Requisitos

- Windows 10/11 de 64 bits
- Visual Studio 2022 con Desktop development with C++
- CMake 3.22+
- Conexión a Internet durante la primera configuración (CMake descarga JUCE 8.0.6)
- FL Studio de 64 bits

## Compilar

Abre **Developer PowerShell for VS 2022** en esta carpeta:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

Los VST3 se generan normalmente en:

```text
build/CRIS_VOX_TUNER_artefacts/Release/VST3/
build/CRIS_VOX_ENHANCER_artefacts/Release/VST3/
```

Si la ruta exacta cambia según la versión de JUCE/CMake, busca las carpetas `*.vst3` dentro de `build`.

## Instalar en FL Studio

Copia cada carpeta `.vst3` a:

```text
C:\Program Files\Common Files\VST3\
```

En FL Studio: **Options > Manage plugins > Find installed plugins**.

## Estructura

```text
CRIS_AUDIO_SUITE/
├── CMakeLists.txt
├── README.md
├── LICENSE
├── .gitignore
├── docs/
│   └── ROADMAP.md
├── CRIS_VOX_TUNER/
│   ├── CMakeLists.txt
│   └── Source/
└── CRIS_VOX_ENHANCER/
    ├── CMakeLists.txt
    └── Source/
```

## GitHub

Crea un repositorio vacío llamado `CRIS-AUDIO-SUITE`, descomprime esta carpeta y sube su contenido. No subas la carpeta `build/`.

## Uso responsable

Los nombres de presets son referencias creativas generales, no emulaciones exactas de artistas. Ajusta tonalidad y escala según la canción. Una buena afinación no reemplaza una buena interpretación ni una edición auditiva.
