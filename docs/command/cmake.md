# CMake — Alternatif Build Sistemi

> ⚠️ **Subject Notu:** Subject **yalnızca Makefile** zorunlu kılar
> (R-01: "You must use C and submit a Makefile"). CMake, **defense
> sırasında resmi build sistemi olarak kabul edilmez**.
>
> Bu dosya, CMake’in **opsiyonel** kullanım amaçlarını anlatır:
>
> - IDE entegrasyonu (CLion, VSCode CMake Tools, Visual Studio).
> - Cross-platform syntax kontrolü.
> - Test target’ları (CTest entegrasyonu).
> - Modern dependency tracking.

---

## 1. CMake Kurulumu

```bash
# Linux (Debian/Ubuntu)
sudo apt-get install -y cmake

# macOS
brew install cmake

# Windows
winget install Kitware.CMake
# veya https://cmake.org/download/
```

```bash
cmake --version
# en az: cmake version 3.16
```

---

## 2. Önerilen `CMakeLists.txt`

Aşağıdaki içerik, projenin kökünde **Makefile’a paralel** çalışabilir:

```cmake
cmake_minimum_required(VERSION 3.16)
project(ft_ping
    VERSION 5.1
    DESCRIPTION "ft_ping — Ecole 42 ICMP ping recoder"
    LANGUAGES C
)

# Subject zorunlu flag'leri
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Subject mandatory warning flag'leri
add_compile_options(
    -Wall
    -Wextra
    -Werror
    -Wpedantic
    -Wshadow
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-g -O0)
endif()

# Header path
include_directories(${CMAKE_SOURCE_DIR}/hdr)

# Source listesi (glob — yeni dosya eklendiğinde re-configure gerek)
file(GLOB FT_PING_SRCS CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.c")

add_executable(ft_ping ${FT_PING_SRCS})

set_target_properties(ft_ping PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/output"
)

# POSIX gereksinimleri (Linux + macOS)
if(UNIX AND NOT APPLE)
    target_compile_definitions(ft_ping PRIVATE _GNU_SOURCE)
endif()

# Linker flags (network library) — Linux/macOS'ta default linkende mevcut
# Solaris vb. için: target_link_libraries(ft_ping PRIVATE socket nsl)
```

---

## 3. Build Komutları

### 3.1 Linux / macOS

```bash
# Out-of-source build (önerilen)
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j$(nproc)

# Çıktı: output/ft_ping
```

### 3.2 Windows (PowerShell)

```powershell
# Visual Studio generator
cmake -S . -B build -G "Visual Studio 17 2022" -A x64

# Ninja (önerilen — daha hızlı)
cmake -S . -B build -G "Ninja"
cmake --build build --config Debug
```

> Windows üzerinde build başarılı olsa bile **çalıştırmak** mümkün
> değildir (POSIX socket header’ları yok). Bkz.
> [windows.md](windows.md).

### 3.3 Ninja (Linux/macOS — Hızlı Build)

```bash
sudo apt-get install -y ninja-build  # veya brew install ninja
cmake -S . -B build -G Ninja
ninja -C build
```

---

## 4. Build Type’ları

```bash
# Debug (default)
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug

# Release (optimize)
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

# Address Sanitizer
cmake -S . -B build \
      -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_C_FLAGS="-fsanitize=address -fno-omit-frame-pointer"

# Coverage
cmake -S . -B build \
      -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_C_FLAGS="--coverage"
```

---

## 5. Custom Targets (Norminette / Cppcheck / Run)

```cmake
# norm hedefi
add_custom_target(norm
    COMMAND norminette ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/hdr
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Running Ecole 42 norminette..."
)

# cppcheck hedefi
add_custom_target(cppcheck
    COMMAND cppcheck --enable=all --inconclusive --std=c11
                     --force --quiet ${CMAKE_SOURCE_DIR}/src
    COMMENT "Running cppcheck..."
)

# run hedefi
add_custom_target(run
    COMMAND sudo $<TARGET_FILE:ft_ping> 8.8.8.8
    DEPENDS ft_ping
    USES_TERMINAL
)
```

Kullanım:

```bash
cmake --build build --target norm
cmake --build build --target cppcheck
cmake --build build --target run
```

---

## 6. CTest ile Test (Opsiyonel)

```cmake
enable_testing()

add_test(NAME help_flag
         COMMAND ft_ping -?)

add_test(NAME unknown_host
         COMMAND ft_ping nonexistent.invalid.tld)
set_tests_properties(unknown_host PROPERTIES WILL_FAIL TRUE)
```

```bash
cd build
ctest --output-on-failure
```

---

## 7. IDE Entegrasyonu

### CLion

`CMakeLists.txt`’ı **Open Project** ile aç. CLion CMake’i otomatik
algılar; build/debug/run profilleri hazır gelir.

### VSCode

```json
// .vscode/settings.json
{
  "cmake.sourceDirectory": "${workspaceFolder}",
  "cmake.buildDirectory": "${workspaceFolder}/build",
  "cmake.generator": "Ninja",
  "cmake.configureOnOpen": true
}
```

`CMake Tools` extension yüklü olmalı.

### Visual Studio (Windows)

`File → Open → CMake...` ile `CMakeLists.txt` seçilir. Visual Studio
otomatik configure & build yapar.

---

## 8. Makefile vs CMake — Karar Tablosu

| Kriter                            | Makefile      | CMake          |
|-----------------------------------|---------------|----------------|
| Subject zorunlu                   | ✅            | ❌             |
| Defense kabul                     | ✅            | ❌ (yardımcı)  |
| Cross-platform                    | ⚠ POSIX only  | ✅             |
| IDE entegrasyonu                  | sınırlı       | mükemmel       |
| Öğrenme eğrisi                    | düşük         | orta           |
| Dependency tracking               | manuel `-MMD` | otomatik       |
| Build artifact’ı izole            | manuel        | otomatik (build/) |

> **Sonuç:** Subject için **Makefile vazgeçilmezdir**. CMake **paralel
> olarak** geliştirme kolaylığı için bulundurulabilir; ancak teslim
> edilen artifact `make`’in çalıştığı projedir.
