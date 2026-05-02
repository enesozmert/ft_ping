# COMMAND — Genel Komut Referansı

> ft_ping projesini geliştirmek, derlemek, çalıştırmak ve denetlemek
> için kullanılan **tüm komutların genel listesi**. OS / build-system
> bazlı **detaylar** ayrı dosyalarda yer alır.

---

## OS / Build System Detay Dosyaları

| Dosya                      | Kapsam                                                |
|----------------------------|-------------------------------------------------------|
| [linux.md](linux.md)       | Native Linux (Debian/Ubuntu/Fedora/Arch)              |
| [windows.md](windows.md)   | Windows (WSL2 / Docker Desktop / MSYS2)               |
| [macos.md](macos.md)       | macOS (Apple clang / Homebrew GCC)                    |
| [docker.md](docker.md)     | Docker / docker-compose (önerilen geliştirme ortamı)  |
| [cmake.md](cmake.md)       | CMake (opsiyonel, IDE entegrasyonu için)              |

---

## 1. Build Komutları (Make — subject zorunlu)

```bash
make                # default: all → output/ft_ping
make all            # tam build
make clean          # *.o + *.d sil
make fclean         # *.o + executable + output/ sil
make re             # fclean + all
make help           # tüm target'ların listesi
```

| Target          | Subject   | Açıklama                                    |
|-----------------|-----------|---------------------------------------------|
| `all`           | ✅ zorunlu | Default target, executable üretir           |
| `clean`         | ✅ zorunlu | Object & dependency file’ları temizler      |
| `fclean`        | ✅ zorunlu | `clean` + executable + `output/`            |
| `re`            | ✅ zorunlu | `fclean` + `all`                            |
| `run`           | opsiyonel | Build edip çalıştırır (`ARGS=` `SUDO=1`)    |
| `setcap`        | opsiyonel | `CAP_NET_RAW` capability uygular            |
| `norm`          | quality   | Ecole 42 norminette                         |
| `cppcheck`      | quality   | Cppcheck statik analiz                      |
| `misra`         | quality   | MISRA C:2012 (cppcheck addon)               |
| `strict`        | quality   | Ekstra strict warning’ler ile rebuild       |
| `test`          | quality   | Smoke test (CLI/exit-code)                  |
| `valgrind`      | quality   | Valgrind altında çalıştır                   |
| `asan`          | quality   | AddressSanitizer build                      |
| `diff-ref`      | quality   | `inetutils-2.0` ile diff                    |
| `cmake-build`   | opsiyonel | CMake ile out-of-source build               |
| `cmake-clean`   | opsiyonel | `build/` dizinini sil                       |

---

## 2. Çalıştırma Komutları

### 2.1 Linux / WSL2 (capability ile — önerilen)

```bash
sudo setcap cap_net_raw+ep output/ft_ping
./output/ft_ping <hostname-or-ip>
```

### 2.2 Linux / WSL2 (sudo ile)

```bash
sudo ./output/ft_ping <hostname-or-ip>
make run SUDO=1 ARGS="google.com"
```

### 2.3 macOS (sudo zorunlu — `setcap` yok)

```bash
sudo ./output/ft_ping <hostname-or-ip>
```

### 2.4 Docker (container içinde)

```bash
docker exec -it ft_ping_container bash -lc "./output/ft_ping <host>"
```

---

## 3. ft_ping CLI Bayrakları

| Bayrak     | Tür         | Açıklama                                        |
|------------|-------------|-------------------------------------------------|
| `-v`       | mandatory   | Verbose mode (paket hata/anomalileri görünür)   |
| `-?`       | mandatory   | Usage / help mesajı                             |
| `-c <n>`   | bonus       | n paket gönder ve dur                           |
| `-i <s>`   | bonus       | n saniyede bir paket                            |
| `-s <n>`   | bonus       | Payload boyutu                                  |
| `-t/--ttl` | bonus       | TTL değeri                                      |
| `-w <n>`   | bonus       | Toplam süre limiti                              |
| `-W <n>`   | bonus       | Reply timeout                                   |
| `-n`       | bonus       | Reverse DNS resolution kapat                    |

---

## 4. Test Senaryoları (Smoke Test)

```bash
# Mandatory
./output/ft_ping 8.8.8.8                  # IPv4
./output/ft_ping google.com               # hostname
./output/ft_ping www.example.com          # FQDN
./output/ft_ping -v 1.1.1.1               # verbose
./output/ft_ping -?                       # usage

# Hata yolu
./output/ft_ping nonexistent.invalid.tld  # unknown host
./output/ft_ping -X 8.8.8.8               # invalid flag
./output/ft_ping                          # argümansız

# Bonus / verbose hata
./output/ft_ping -v --ttl=1 8.8.8.8       # TTL exceeded ICMP type 11
```

---

## 5. Statik Analiz

### 5.1 Make ile (önerilen)

```bash
make norm          # Ecole 42 norminette
make cppcheck      # Cppcheck genel analiz → cppcheck.txt
make misra         # MISRA C:2012 (cppcheck addon) → misra.txt
make strict        # Ekstra strict warning'ler ile rebuild
```

### 5.2 CMake ile

```bash
cmake --build build --target norm
cmake --build build --target cppcheck
cmake --build build --target misra
cmake --build build --target quality   # norm + cppcheck + check
```

### 5.3 Manuel

```bash
norminette src/ hdr/
cppcheck --enable=all --inconclusive --std=c11 \
         --force --quiet src/ 2> cppcheck.txt
cppcheck --dump --std=c11 src/*.c
python3 /usr/share/cppcheck/addons/misra.py src/*.dump > misra.txt
```

---

## 6. Dinamik Analiz

### 6.1 Make ile

```bash
make valgrind VALGRIND_HOST=127.0.0.1   # default host: 127.0.0.1
make asan                               # AddressSanitizer rebuild
make test                               # smoke test (CLI/exit-code)
```

### 6.2 CMake ile

```bash
cmake --build build --target valgrind
cmake --build build --target check      # ctest --output-on-failure
```

### 6.3 Manuel

```bash
# Valgrind — memory leak / invalid access
sudo valgrind --leak-check=full \
              --show-leak-kinds=all \
              --track-origins=yes \
              ./output/ft_ping -c 3 8.8.8.8

# Strace — syscall takibi
sudo strace -e trace=network ./output/ft_ping 8.8.8.8

# tcpdump — wire-level paket
sudo tcpdump -n -i any icmp
```

---

## 7. Reference (`inetutils-2.0`) ile Karşılaştırma

```bash
# Sistem ping çıktısı
ping -V                                       # versiyon kontrolü
ping -c 4 google.com > /tmp/ref.txt
sudo ./output/ft_ping google.com > /tmp/our.txt   # birkaç paket sonra Ctrl+C

# Diff (RTT ve reverse DNS satırları hariç)
diff -u /tmp/ref.txt /tmp/our.txt
```

---

## 8. Docker Yaşam Döngüsü

```bash
# Build & start
docker compose -f docker/docker-compose.yml up -d --build

# Bağlan
docker exec -it ft_ping_container bash

# Stop / start / restart
docker compose -f docker/docker-compose.yml stop
docker compose -f docker/docker-compose.yml start
docker compose -f docker/docker-compose.yml restart

# Tamamen kaldır
docker compose -f docker/docker-compose.yml down -v
docker rmi ft_ping:latest
```

---

## 9. CMake

```bash
# Configure + build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j

# Ninja generator (hızlı)
cmake -S . -B build -G Ninja
ninja -C build

# Quality targets
cmake --build build --target norm        # norminette
cmake --build build --target cppcheck    # cppcheck
cmake --build build --target misra       # MISRA C:2012
cmake --build build --target check       # CTest suite
cmake --build build --target valgrind    # valgrind
cmake --build build --target quality     # norm + cppcheck + check (umbrella)

# Strict / sanitizer
cmake -S . -B build -DFT_PING_STRICT=ON
cmake -S . -B build -DFT_PING_ASAN=ON
```

---

## 10. Git Komutları

```bash
git status
git add <files>
git commit -m "<message>"
git push
git log --oneline --graph --decorate -20
git diff --stat
```

> ⚠️ `make`, `valgrind`, `cppcheck` çıktıları `git status`’ta görünmemeli.
> `.gitignore` içinde `obj/`, `output/`, `*.dump`, `cppcheck.txt`,
> `misra.txt` bulunmalıdır.

---

## 11. Komut Hızlı Erişim Tablosu

| Amaç                            | Komut                                                     |
|---------------------------------|-----------------------------------------------------------|
| Build                           | `make`                                                    |
| Tam yeniden build               | `make re`                                                 |
| Temizle                         | `make fclean`                                             |
| Çalıştır (capability)           | `sudo setcap cap_net_raw+ep output/ft_ping && ./output/ft_ping host` |
| Çalıştır (sudo)                 | `sudo ./output/ft_ping host`                              |
| Norm denetimi                   | `norminette src/ hdr/`                                    |
| Statik analiz                   | `cppcheck --enable=all --std=c11 src/`                    |
| Memory leak                     | `sudo valgrind --leak-check=full ./output/ft_ping host`   |
| Docker build & up               | `docker compose -f docker/docker-compose.yml up -d --build` |
| Docker bash                     | `docker exec -it ft_ping_container bash`                  |
| Sistem ping ile diff            | `diff <(ping -c 3 host) <(./output/ft_ping host)`         |

---

## Detaylar İçin

OS / build-system bazlı **adım adım** komut akışı için ilgili detay
dosyasına bakın:

- 🐧 [Linux](linux.md)
- 🪟 [Windows](windows.md)
- 🍎 [macOS](macos.md)
- 🐳 [Docker](docker.md)
- 🔧 [CMake](cmake.md)
