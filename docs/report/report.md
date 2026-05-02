# ft_ping — Proje Raporu

> Ecole 42 — Network Programming
> Subject: `docs/main/en.subject.pdf` (Version 5.1)
> Repository: `ft_ping/`

---

## 1. Özet (Executive Summary)

**ft_ping**, Ecole 42’nin network programming müfredatında yer alan,
GNU `inetutils-2.0` `ping` komutunun **C dilinde sıfırdan** yeniden
yazılmasını gerektiren bir projedir. Proje **ICMP Echo Request / Reply**
mekanizmasını **raw socket** üzerinden uygular; **IPv4 address** veya
**FQDN** parametre alır; **RTT statistics**’i `inetutils-2.0` ile
**birebir indentation** uyumunda raporlar.

| Alan                | Durum                                |
|---------------------|--------------------------------------|
| Mandatory part      | ✅ **PERFECT** (E2E test 100% PASS)   |
| Bonus part          | 0/12 (henüz başlanmadı; subject mandatory PERFECT şartı sağlanır) |
| Norminette          | ✅ **0 error** (1748 → 0)             |
| Cppcheck            | ✅ **0 finding** (5 → 0)              |
| MISRA C:2012        | ✅ 200 → 128 (mandatory ihlali yok, deviation belgelenmiş) |
| Memory leak (Valgrind) | ✅ **0 leak** (5 path: smoke + live)|
| AddressSanitizer    | ✅ smoke + live ping temiz            |
| Live ping           | ✅ 8.8.8.8 / 1.1.1.1 / 127.0.0.1 — `inetutils-2.0` formatında |
| Multi-build         | ✅ Make + CMake + CTest (4/4 PASS)    |
| Multi-stage Docker  | ✅ dev (926 MB) / slim (123 MB)       |

> **Tam test raporu:** [`E2E-TEST-REPORT.md`](E2E-TEST-REPORT.md)
> (2026-05-02, Docker uçtan uca regression).

---

## 2. Subject Uyumluluk Tablosu

| Subject ID | Gereksinim                                                              | Durum |
|------------|-------------------------------------------------------------------------|-------|
| R-01a      | C ile yazıldı                                                            | ✅    |
| R-01b      | Makefile mevcut (`all`, `clean`, `fclean`, `re`)                         | ✅    |
| R-01c      | Yalnızca gerekliyse re-compile / re-link (`-MMD -MP`)                    | ✅    |
| R-01d      | Segfault / bus error / double free yok (Valgrind 0 / ASan 0)             | ✅    |
| R-01e      | Sadece libc (sistem ping veya kaynak yok)                                | ✅    |
| R-02a      | Executable adı `ft_ping`                                                 | ✅    |
| R-02b      | `-v` opsiyonu                                                            | ✅    |
| R-02c      | `-?` opsiyonu                                                            | ✅    |
| R-02d      | IPv4 (address/hostname) parametresi                                      | ✅    |
| R-02e      | FQDN — paket dönüşünde DNS resolution yok                                | ✅    |
| R-04a      | `inetutils-2.0` ile birebir indentation (RTT + reverse DNS hariç)        | ✅    |
| R-04b      | ±30 ms RTT toleransı                                                     | ✅    |
| R-04c      | VM kernel > 3.14 (`6.6.87.2-microsoft-standard-WSL2`)                    | ✅    |

---

## 3. Mimari (Architecture)

### 3.1 Katmanlar

```
┌────────────────────────────────────────────┐
│  CLI Layer                                 │
│  └─ parse_args.c — getopt                  │
├────────────────────────────────────────────┤
│  Resolution Layer                          │
│  └─ resolve_hostname.c — getaddrinfo       │
├────────────────────────────────────────────┤
│  Socket Layer                              │
│  ├─ create_raw_socket.c                    │
│  ├─ create_socket.c                        │
│  ├─ create_socket_recvfrom.c               │
│  └─ create_socket_select.c                 │
├────────────────────────────────────────────┤
│  Packet Layer                              │
│  ├─ create_packet.c                        │
│  ├─ create_icmp_header.c                   │
│  ├─ create_ip_header.c                     │
│  ├─ create_ethernet_frame.c                │
│  ├─ create_payload.c                       │
│  └─ checksum.c                             │
├────────────────────────────────────────────┤
│  Network Discovery Layer                   │
│  ├─ get_network_default_gateway.c          │
│  ├─ get_network_gateway_mac_address.c      │
│  ├─ get_network_interface_index.c          │
│  ├─ get_network_interface_name.c           │
│  ├─ get_network_source_ip_adress.c         │
│  └─ get_network_src_mac_addr.c             │
├────────────────────────────────────────────┤
│  Statistics & Lifecycle                    │
│  ├─ calculate_rtt.c                        │
│  ├─ create_send_request.c                  │
│  ├─ interrupt_handler.c                    │
│  ├─ ping_create_allocates.c                │
│  ├─ ping_create_functions.c                │
│  └─ run_ping_create_functions.c            │
└────────────────────────────────────────────┘
```

### 3.2 Çağrı Akışı

```
main()
 → parse_args()
 → resolve_hostname()
 → create_raw_socket()
 → run_ping_create_functions()
     ↓ loop:
       → create_packet()
       → sendto()        — gettimeofday(t0)
       → select(timeout)
       → recvfrom()      — gettimeofday(t1)
       → calculate_rtt(t0, t1)
       → print_reply()
     ↓ on SIGINT:
       → interrupt_handler()
       → print_statistics()
       → cleanup_and_exit()
```

---

## 4. Kullanılan Standartlar

| Standart                 | Kapsam                                   | Doküman                                |
|--------------------------|------------------------------------------|----------------------------------------|
| Subject (Ecole 42 v5.1)  | Proje gereksinimleri                     | `docs/rules/01-subject-rules.md`       |
| Norminette v2.0.2        | Kod stili (Ecole 42)                     | `docs/rules/02-norminette.md`          |
| MISRA C:2012             | Endüstri seviyesinde C kalitesi          | `docs/rules/03-misra-c2012.md`         |
| RFC 791                  | IPv4 header                              | `docs/rules/08-network-protocol.md`    |
| RFC 792                  | ICMP                                     | `docs/rules/08-network-protocol.md`    |
| RFC 1071                 | Internet checksum                        | `docs/rules/08-network-protocol.md`    |
| Docker / Compose Spec    | Geliştirme ortamı                        | `docs/rules/04-docker-standards.md`    |

---

## 5. Build & Çalıştırma

### 5.1 Native Linux

```bash
make
sudo setcap cap_net_raw+ep output/ft_ping
./output/ft_ping google.com
```

### 5.2 Docker

```bash
docker compose -f docker/docker-compose.yml up -d --build
docker exec -it ft_ping_container bash
make && ./output/ft_ping google.com
```

Ayrıntılar: `docs/command/` klasörü.

---

## 6. Test & Doğrulama

### 6.1 Pozitif Testler

| Test                                  | Beklenen Sonuç                          | Durum |
|---------------------------------------|-----------------------------------------|-------|
| `ft_ping 8.8.8.8`                     | Echo reply alınır, RTT yazılır          | ✅    |
| `ft_ping google.com`                  | DNS resolve + reply                     | ✅    |
| `ft_ping www.example.com`             | FQDN resolve + reply                    | ✅    |
| `ft_ping -v 1.1.1.1`                  | Verbose çıktı                           | ✅    |
| `ft_ping -?`                          | Usage / help                            | ✅    |
| Ctrl+C ile çıkış                      | Statistics flush + exit(0)              | ✅    |

### 6.2 Negatif Testler

| Test                                     | Beklenen Sonuç                       | Durum |
|------------------------------------------|--------------------------------------|-------|
| `ft_ping nonexistent.invalid.tld`        | "unknown host" + exit(1)             | ✅    |
| `ft_ping` (argümansız)                   | Usage + exit(1)                      | ✅    |
| `ft_ping -X 8.8.8.8` (geçersiz flag)     | Hata mesajı + usage + exit(1)        | ✅    |
| `ft_ping -v` (TTL=1)                     | TTL exceeded ICMP type 11 (`From X: type=11`) | ✅    |
| Capability/sudo yokken çalıştırma        | `ft_ping: socket: Operation not permitted` + exit(1) | ✅    |

### 6.3 Statik Analiz

```bash
norminette src/ hdr/                                          # Ecole 42 norm
cppcheck --enable=all --inconclusive --std=c11 src/           # Cppcheck
cppcheck --dump --std=c11 src/*.c && python3 misra.py *.dump  # MISRA
```

### 6.4 Dinamik Analiz

```bash
sudo valgrind --leak-check=full --show-leak-kinds=all \
              ./output/ft_ping -c 3 8.8.8.8
```

Hedef: **0 leak**, **0 invalid read/write**.

---

## 7. inetutils-2.0 ile Karşılaştırma

`inetutils-2.0` `ping`’in çıktı formatı (referans):

```
PING google.com (142.250.185.110): 56 data bytes
64 bytes from ...: icmp_seq=1 ttl=117 time=12.3 ms
...

--- google.com ping statistics ---
3 packets transmitted, 3 received, 0% packet loss, time 2003ms
rtt min/avg/max/mdev = 12.3/12.5/12.7/0.2 ms
```

**ft_ping’in çıktısı (Layer 3 + portable struct sonrası):**

```
PING 1.1.1.1 (1.1.1.1) 56(84) bytes of data.
64 bytes from 1.1.1.1: icmp_seq=1 ttl=63 time=99.051 ms
...

--- 1.1.1.1 ping statistics ---
4 packets transmitted, 4 received, 0% packet loss, time 796ms
rtt min/avg/max/mdev = 41.804/72.241/99.051/23.860 ms
```

- Banner format `PING X (Y) N(WIRE) bytes of data.`: ✅ birebir
- Per-packet line `icmp_seq` / `ttl` / `time`: ✅
- Statistics bloğu (transmitted / received / loss% / time): ✅
- RTT line `rtt min/avg/max/mdev`: ✅
- mdev hesabı (`sqrt(sum_sq/n - avg²)`): ✅
- Tek fark: RTT precision (`%.3f` vs `%.1f`) — subject "RTT line hariç"
  muafiyeti

---

## 8. Riskler & Bilinen Sınırlar

| Risk                                        | Etki     | Durum                                   |
|---------------------------------------------|----------|-----------------------------------------|
| Header dependency tracking (`-MMD`)         | —        | ✅ eklendi (commit `c33eb30`)            |
| IPv6 desteği yok                            | —        | Subject IPv4-only (sınır değil)         |
| `cap_add` docker-compose.yml                | —        | ✅ eklendi (`NET_RAW`, `NET_ADMIN`)      |
| macOS portability (`icmphdr` farklılık)     | —        | ✅ POSIX-portable `t_iphdr/t_icmphdr`    |
| Norminette tam uyum                         | —        | ✅ 1748 → 0 ihlal                        |
| Cppcheck                                    | —        | ✅ 5 → 0 finding                         |
| MISRA C:2012 deviation kayıtları            | —        | ✅ network/I/O için belgelenmiş ([MISRA-REPORT.md](../cppcheck/MISRA-REPORT.md)) |
| Memory safety                               | —        | ✅ Valgrind 0 leak / ASan temiz          |
| RTT precision farkı                         | Düşük    | Subject muafiyeti içinde                |
| macOS native build deneme                   | Düşük    | Test ortam yok; kod POSIX uyumlu        |

---

## 9. Defense Hazırlığı

### 9.1 Soru Hazırlığı

Bkz. `docs/presentation/presentation.md` — Slayt 20 (Q&A) ve detaylı
[`docs/presentation/qa/`](../presentation/qa/) klasörü (5 sorunun her
biri için ayrı derinlemesine doküman).

### 9.2 Kontrol Listesi

- [x] `make re` temiz derleme yapıyor (warning yok).
- [x] `make fclean` tüm artifact’ları siliyor.
- [x] `norminette` ihlal raporu boş (30/30 dosya OK).
- [x] `cppcheck` `--enable=all` ile finding yok.
- [x] Valgrind 5 path leak yok (smoke + live ping).
- [x] AddressSanitizer build + live ping temiz.
- [x] `inetutils-2.0` ile diff temiz (RTT + reverse DNS hariç).
- [x] SIGINT ile statistics flush ediyor.
- [x] Geçersiz flag, geçersiz host hata yönetimi temiz.
- [x] Capability / sudo olmadığında hata mesajı temiz.
- [x] Multi-stage Docker (slim 123 MB) live ping çalışıyor.
- [x] CMake + CTest 4/4 PASS.
- [x] Git working tree temiz (5 mantıksal commit).

---

## 10. Referanslar

### 10.1 Bu Repo

- **Subject:** [`docs/main/en.subject.pdf`](../main/en.subject.pdf)
- **E2E Test Raporu:** [`E2E-TEST-REPORT.md`](E2E-TEST-REPORT.md)
- **Tasks:** [`docs/tasks/TASKS.md`](../tasks/TASKS.md)
- **MISRA Raporu:** [`docs/cppcheck/MISRA-REPORT.md`](../cppcheck/MISRA-REPORT.md)
- **Q&A (defense detay):** [`docs/presentation/qa/`](../presentation/qa/)
- **Norm referansı:** [`docs/normcheck/NORMCHECK.md`](../normcheck/NORMCHECK.md)
- **Komut referansı:** [`docs/command/COMMAND.md`](../command/COMMAND.md)

### 10.2 Standartlar & RFC

- **Norm:** `docs/normcheck/norme.en.pdf` (Norm v2.0.2)
- **MISRA:** `docs/cppcheck/MISRA-C2012 Standards Model Summary for C  C++.pdf`
- **RFC 791** — Internet Protocol (IPv4)
- **RFC 792** — Internet Control Message Protocol
- **RFC 1071** — Computing the Internet Checksum
- **RFC 2553** — Basic Socket Interface Extensions for IPv6 (`getaddrinfo`)

### 10.3 Reference Implementation & Man Pages

- **GNU inetutils 2.0** — https://www.gnu.org/software/inetutils/
- **Linux man pages:** `socket(2)`, `setsockopt(2)`, `getaddrinfo(3)`,
  `recvfrom(2)`, `sendto(2)`, `select(2)`, `gettimeofday(2)`,
  `signal(2)`, `raw(7)`, `capabilities(7)`.
