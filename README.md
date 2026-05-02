# ft_ping

> Ecole 42 — Network Programming Project (Subject v5.1)
> ICMP `ping` komutunun **C** dilinde sıfırdan yeniden yazılması.

[![Norminette](https://img.shields.io/badge/norminette-OK%20(0%20errors)-brightgreen)](docs/tasks/TASKS.md)
[![Cppcheck](https://img.shields.io/badge/cppcheck-clean-brightgreen)](docs/cppcheck/MISRA-REPORT.md)
[![Valgrind](https://img.shields.io/badge/valgrind-0%20leaks-brightgreen)](docs/tasks/TASKS.md)
[![Build](https://img.shields.io/badge/build-make%20%2B%20cmake-blue)](Makefile)

---

## İçindekiler

- [Genel Bakış](#genel-bakış)
- [Hızlı Başlangıç](#hızlı-başlangıç)
- [Build Sistemleri](#build-sistemleri)
- [Quality Gates](#quality-gates)
- [Proje Yapısı](#proje-yapısı)
- [Dokümantasyon](#dokümantasyon)
- [Sınırlar & Bilinen Sorunlar](#sınırlar--bilinen-sorunlar)
- [Lisans & Yazar](#lisans--yazar)

---

## Genel Bakış

ft_ping, GNU `inetutils-2.0`'ın `ping` komutunun **C** dilinde sıfırdan
yeniden yazılmasıdır. Subject (Ecole 42 v5.1) tarafından zorunlu kılınan:

- IPv4 (address veya hostname) parametresi
- FQDN desteği (paket dönüşünde DNS resolution **yok**)
- `-v` (verbose) ve `-?` (help) opsiyonları
- ICMP raw socket üzerinden Echo Request/Reply
- `inetutils-2.0` ile **birebir indentation** uyumlu çıktı (RTT + reverse DNS hariç)
- Segfault / bus error / double free **yasak**

Subject dosyası: [`docs/main/en.subject.pdf`](docs/main/en.subject.pdf)

---

## Hızlı Başlangıç

### Docker (önerilen — host bağımsız)

```bash
docker compose -f docker/docker-compose.yml up -d --build
docker exec -it ft_ping_container bash
# container içinde:
make
sudo ./output/ft_ping google.com
```

### Native Linux

```bash
make
sudo setcap cap_net_raw+ep output/ft_ping   # bir kez
./output/ft_ping google.com
./output/ft_ping -v 8.8.8.8
./output/ft_ping -?
```

Detaylı OS-bazlı komutlar için: [`docs/command/COMMAND.md`](docs/command/COMMAND.md)

---

## Build Sistemleri

ft_ping iki build sistemi destekler — ikisi de **aynı kalite hedeflerini**
karşılar.

### Make (subject zorunlu)

```bash
make            # build → output/ft_ping
make re         # fclean + all
make clean      # *.o + *.d sil
make fclean     # clean + executable sil

make norm       # Ecole 42 norminette
make cppcheck   # cppcheck statik analiz
make misra      # MISRA C:2012
make test       # smoke test (CLI/exit-code)
make valgrind   # memory leak / invalid access
make asan       # AddressSanitizer build
make help       # tüm target'lar
```

### CMake (paralel, IDE entegrasyonu için)

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j
cmake --build build --target check       # CTest 4 case
cmake --build build --target quality      # norm + cppcheck + check
```

Detay: [`docs/command/cmake.md`](docs/command/cmake.md)

---

## Quality Gates

### Norminette (Ecole 42 v2.0.2)

```
30/30 dosya OK!
1 Notice (GLOBAL_VAR_DETECTED — g_ping subject zorunlu)
0 Error
```

### Cppcheck

```
1 finding (style: unusedFunction calculate_rtt — kabul edilebilir)
0 BUG / 0 portability / 0 nullPointer / 0 uninitvar / 0 memleak
```

### MISRA C:2012

```
200 finding (advisory + required)
- 84 belgelenmiş deviation (network/IO için tipik)
- 0 mandatory ihlali
```

Detay rapor: [`docs/cppcheck/MISRA-REPORT.md`](docs/cppcheck/MISRA-REPORT.md)

### Valgrind

```
4/4 path: 0 errors, 0 leaks (no-args, -?, -X invalid, unknown host)
```

### Smoke Tests

```
T-505: -? usage          OK
T-510: unknown host      OK
T-511: no-args usage     OK
T-512: invalid flag      OK
```

---

## Proje Yapısı

```
ft_ping/
├── Makefile                       # Subject zorunlu build sistemi
├── CMakeLists.txt                 # Alternatif build (IDE)
├── README.md                      # Bu dosya
├── .gitignore / .dockerignore
│
├── src/                           # 26 .c dosyası
│   ├── main.c
│   ├── parse_args.c
│   ├── resolve_hostname.c
│   ├── create_*.c                 # packet/socket/header oluşturucular
│   ├── get_network_*.c            # interface/MAC/gateway sorguları
│   ├── checksum.c                 # RFC 1071 1's complement sum
│   ├── calculate_rtt.c
│   ├── interrupt_handler.c        # SIGINT statistics flush
│   └── ping_create_*.c            # alloc + function pointer table
│
├── hdr/                           # 4 .h dosyası
│   ├── header.h                   # merkezi header
│   ├── ft_ping.h                  # public umbrella
│   ├── packet.h                   # packet katmanı (geleceğe ayrılmış)
│   └── parse_args.h               # CLI parsing
│
├── docker/
│   ├── Dockerfile                 # Ubuntu 22.04 + tooling
│   └── docker-compose.yml         # cap_add: NET_RAW + sysctls
│
├── opt/
│   └── norm_autofix.py            # Norminette mekanik düzeltici
│
└── docs/
    ├── README.md                  # docs index
    ├── main/en.subject.pdf        # subject (resmi)
    ├── rules/                     # 8 kural seti md (subject, norm, MISRA, docker, vb.)
    ├── command/                   # 5 OS/build md (linux, windows, macos, docker, cmake)
    ├── tasks/TASKS.md             # proje görev listesi (faz bazlı)
    ├── presentation/              # defense slaytları (21 slayt)
    ├── report/report.md           # proje durum raporu
    ├── cppcheck/MISRA-REPORT.md   # MISRA analizi + deviation
    └── normcheck/NORMCHECK.md     # Norminette referansı
```

---

## Dokümantasyon

| Konu | Dosya |
|------|-------|
| Subject (resmi PDF) | [`docs/main/en.subject.pdf`](docs/main/en.subject.pdf) |
| Görev listesi | [`docs/tasks/TASKS.md`](docs/tasks/TASKS.md) |
| Komut referansı | [`docs/command/COMMAND.md`](docs/command/COMMAND.md) |
| Defense sunumu | [`docs/presentation/presentation.md`](docs/presentation/presentation.md) |
| Proje raporu | [`docs/report/report.md`](docs/report/report.md) |
| Subject kuralları | [`docs/rules/01-subject-rules.md`](docs/rules/01-subject-rules.md) |
| Norminette | [`docs/rules/02-norminette.md`](docs/rules/02-norminette.md) |
| MISRA C:2012 | [`docs/rules/03-misra-c2012.md`](docs/rules/03-misra-c2012.md) |
| Docker | [`docs/rules/04-docker-standards.md`](docs/rules/04-docker-standards.md) |
| Network protocol | [`docs/rules/08-network-protocol.md`](docs/rules/08-network-protocol.md) |

---

## Sınırlar & Bilinen Sorunlar

- **IPv6 desteği yok** — subject yalnızca IPv4 zorunlu kılar.
- **Reverse DNS** yalnızca başlangıçta yapılır (subject R-02 zorunluluğu).
- **macOS native build**: POSIX-portable struct ile uyumlu;
  detay [`docs/command/macos.md`](docs/command/macos.md).
- **Native Windows build**: Winsock raw socket uyumsuz;
  WSL2 veya Docker önerilir ([`docs/command/windows.md`](docs/command/windows.md)).
- **Ping loop / RTT statistics output**: subject `inetutils-2.0` çıktı
  uyumu için printf temizliği halen devam ediyor (görev T-520).

---

## Lisans & Yazar

**Yazar:** ozmerte (Ecole 42 — student)
**Subject:** Ecole 42 — `ft_ping` v5.1
**Repository:** ft_ping/
**Bağımlılıklar:** Yalnızca `libc` (subject zorunluluğu)
**Yasaklar:** Sistem `ping` komutu çağırma veya standart bir ping
implementasyonunun kaynak kodunu kullanma — **yasak**.

> "Ping is the name of a command that allows you to test the
> accessibility of another machine through the IP network."
> *— en.subject.pdf, Chapter II*
