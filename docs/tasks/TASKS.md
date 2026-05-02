# Task — ft_ping Görev Listesi

> Subject: [`docs/main/en.subject.pdf`](../main/en.subject.pdf) (v5.1)
>
> Bu doküman, ft_ping projesinin **tamamlanması için yapılması gereken
> tüm görevlerin** kontrol listesidir. Her madde **subject** veya
> **kalite** kriterine bağlıdır.

---

## Durum Lejantı

- ☐ — Yapılacak
- 🔄 — Devam ediyor
- ✅ — Tamamlandı
- ⚠ — Eksik / iyileştirme gerekli
- ❌ — Bloke / sorun var

---

## Faz 1 — Subject Mandatory (R-01, R-02)

### 1.1 Build & Make

- ✅ T-101 — `Makefile` oluşturuldu
- ✅ T-102 — `all` rule mevcut
- ✅ T-103 — `clean` rule mevcut
- ✅ T-104 — `fclean` rule mevcut
- ✅ T-105 — `re` rule mevcut
- ✅ T-106 — `-Wall -Wextra -Werror` flag’leri set edildi
- ✅ T-107 — Header dependency tracking (`-MMD -MP`) eklendi
- ✅ T-108 — `-std=c11` açıkça belirtildi
- ✅ T-109 — `strict` target ile `-Wpedantic -Wshadow -Wconversion -Wcast-align` aktive edilebiliyor

### 1.2 Executable

- ✅ T-110 — Executable adı **`ft_ping`** (veya `output/ft_ping`)
- ☐ T-111 — Final teslimde subject path’i kontrol et (`./ft_ping` mi yoksa `output/ft_ping` mi?)

### 1.3 Argument Parsing

- ✅ T-120 — `-v` (verbose) yönetiliyor
- ✅ T-121 — `-?` (help / usage) yönetiliyor
- ✅ T-122 — IPv4 address parametresi alınıyor
- ✅ T-123 — Hostname parametresi alınıyor
- ✅ T-124 — FQDN parametresi alınıyor
- ✅ T-125 — Argümansız çalıştırma → usage + exit(2)
- ✅ T-126 — Geçersiz flag → hata + usage + exit(2)

### 1.4 Network Core

- ✅ T-130 — Raw socket aç (`AF_INET / SOCK_RAW / IPPROTO_ICMP`)
- ✅ T-131 — `getaddrinfo` ile hostname → IPv4 resolve
- ✅ T-132 — `getnameinfo` reverse resolution **yalnızca başlangıçta**
- ✅ T-133 — IPv4 header oluştur
- ✅ T-134 — ICMP Echo Request header oluştur
- ✅ T-135 — Payload + timestamp ekle
- ✅ T-136 — Checksum (RFC 1071) hesapla
- ✅ T-137 — `sendto` ile paket gönder
- ✅ T-138 — `select` ile timeout’lu bekleme
- ✅ T-139 — `recvfrom` ile reply al
- ✅ T-140 — RTT hesabı (`gettimeofday`)
- ✅ T-141 — `inetutils-2.0` ile birebir indentation (Layer 3 refactor sonrası)
- ✅ T-142 — `inetutils-2.0` ile diff temiz (1.1.1.1 ile karşılaştırma yapıldı)

### 1.5 Statistics

- ✅ T-150 — Transmitted / received / loss% sayaçları
- ✅ T-151 — RTT min / avg / max / mdev hesabı
- ✅ T-152 — `SIGINT` handler — istatistik flush + exit(0)

---

## Faz 2 — Hata Yönetimi (R-01)

- ✅ T-201 — `socket()` dönüş kontrolü
- ✅ T-202 — `setsockopt()` dönüş kontrolü
- ✅ T-203 — `getaddrinfo()` dönüş kontrolü (`gai_strerror`)
- ✅ T-204 — `sendto()` / `recvfrom()` dönüş kontrolü
- ✅ T-205 — `EINTR` retry mantığı
- ✅ T-206 — `malloc` NULL kontrolü
- ✅ T-207 — Tüm açık FD’ler exit öncesi kapatılıyor
- ✅ T-208 — Tüm allocate edilen bellek free ediliyor
- ✅ T-209 — Valgrind **0 leak** (5 path: no-args, -?, -X, unknown host, **live ping 127.0.0.1**)
- ✅ T-210 — AddressSanitizer build temiz; live ping `LD_PRELOAD=$(gcc -print-file-name=libasan.so)` ile çalışıyor
- ✅ T-211 — `ft_ping: socket: Operation not permitted` + exit 1 (non-root user testi)

---

## Faz 3 — Kalite Denetimi

### 3.1 Norminette (Ecole 42)

- ✅ T-301 — `norminette src/ hdr/` **0 ERROR** (1748 → 0, %100 temiz)
  - 30/30 dosya `OK!`. Tek `Notice: GLOBAL_VAR_DETECTED` (`g_ping`) — subject zorunlu (signal handler).
  - Süreç: otomatik düzeltici ([opt/norm_autofix.py](../../opt/norm_autofix.py)) ile 1748 → 345; ardından 5 batch manuel düzeltme (her batch 5 dosya) ile 345 → 0.
- ✅ T-302 — Tüm 30 dosyaya 42 header eklendi (login: `ozmerte`)
- ✅ T-303 — Tüm fonksiyonlar 25 satır limit içinde (TOO_MANY_LINES 0)
- ✅ T-304 — Tüm fonksiyonlar 4 parametre limit içinde
- ✅ T-305 — `.c` başına 5 fonksiyon limit içinde
- ✅ T-306 — Tüm satırlar 80 column içinde (LINE_TOO_LONG 0)
- ✅ T-307 — Tüm header’larda include guard mevcut
- ✅ T-308 — `s_`, `t_` prefix kullanılıyor; `g_ping` global prefix doğru

### 3.2 MISRA C:2012

- ✅ T-310 — Cppcheck MISRA addon raporu alındı (200 finding)
- ✅ T-311 — Mandatory rule ihlali **yok**
- ✅ T-312 — Required rule deviation’ları belgelendi ([MISRA-REPORT.md](../cppcheck/MISRA-REPORT.md))
- ✅ T-313 — Network/I/O deviation listesi yazıldı (21.3, 21.8, 11.3, 17.7)
- ✅ T-314 — `goto` kullanımı yok (Rule 15.1)
- ✅ T-315 — Recursion yok (Rule 17.2)
- ⏳ T-316 — Magic number kontrolü (manuel inceleme; constant macro yok ama sayılar makul)

### 3.3 Cppcheck (Genel)

- ✅ T-320 — `--enable=all --inconclusive --std=c11` raporu **0 finding** (Layer 3 refactor sonrası)
- ✅ T-321 — `nullPointer` uyarısı yok
- ✅ T-322 — `uninitvar` uyarısı yok
- ✅ T-323 — `memleak` uyarısı yok (Valgrind 5 path 0 leak, ASan temiz)
- ✅ T-324 — `arrayIndexOutOfBounds` uyarısı yok

---

## Faz 4 — Geliştirme Ortamı

- ✅ T-401 — `Dockerfile` mevcut (Ubuntu 22.04, build + tooling)
- ✅ T-402 — `docker-compose.yml` mevcut
- ✅ T-403 — `cap_add: [NET_RAW, NET_ADMIN]` eklendi
- ✅ T-404 — `.dockerignore` oluşturuldu
- ✅ T-405 — Docker multi-stage: `slim` runtime **123 MB** (% 87 azalma), `dev` 926 MB
- ✅ T-406 — `make all && make test` container içinde sorunsuz (4/4 smoke test OK)
- ✅ T-407 — `setcap` container içinde mevcut (libcap2-bin yüklü)
- ✅ T-408 — Make + CMake + CTest container ortamında doğrulandı (alternative: WSL2 dış)
- ✅ T-409 — macOS portability: POSIX-portable wire-format struct'lar (`t_iphdr`, `t_icmphdr`) + `<getopt.h>` kaldırıldı; tüm header'lar POSIX/C11. Linux'ta build/test temiz; macOS'ta build edilebilir (gerçek test ortam gerektirir).
- ✅ T-410 — `CMakeLists.txt` mevcut (paralel build + IDE entegrasyonu)
- ✅ T-411 — Makefile’da `norm / cppcheck / misra / test / valgrind / asan` target’ları
- ✅ T-412 — CMake’de `norm / cppcheck / misra / check / valgrind` custom target’ları
- ✅ T-413 — Docker image norminette + cppcheck + valgrind + cmake içeriyor
- ✅ T-414 — `.gitignore` build artifact’larını (`obj/ output/ build/ *.dump`) içeriyor

---

## Faz 5 — Test & Doğrulama

### 5.1 Pozitif Test

- ✅ T-501 — `ft_ping 8.8.8.8` → reply alınıyor (Layer 3 refactor sonrası)
- ✅ T-502 — `ft_ping 1.1.1.1` → reply alınıyor
- ✅ T-503 — `ft_ping 127.0.0.1` → reply alınıyor (loopback)
- ✅ T-504 — `ft_ping -v` → verbose banner (`id 0xXXXX = N`)
- ✅ T-505 — `ft_ping -?` → usage
- ✅ T-506 — SIGINT → statistics flush + exit(0)

### 5.2 Negatif Test

- ✅ T-505 — `ft_ping -?` → usage + exit(0)  *(make test, ctest)*
- ✅ T-510 — `ft_ping nonexistent.invalid.tld` → unknown host + exit(non-zero)
- ✅ T-511 — `ft_ping` (argümansız) → usage + exit(non-zero)
- ✅ T-512 — `ft_ping -X 8.8.8.8` → hata + usage + exit(non-zero) *(parse_args getopt fix)*
- ✅ T-513 — Capability/sudo yokken → `ft_ping: socket: Operation not permitted` + exit 1 (T-211 ile)
- ✅ T-514 — TTL=1 ile verbose mode'da TTL exceeded ICMP type=11 raporu (`From <ip>: icmp_seq=N type=11 code=0`)

### 5.3 Karşılaştırma

- ✅ T-520 — `inetutils-2.0` çıktı formatı **birebir uyumlu** (Layer 3 refactor sonrası)
  - Banner: `PING <host> (<ip>) <N>(<wire>) bytes of data.` ✓
  - Per-packet: `<bytes> bytes from <ip>: icmp_seq=<n> ttl=<t> time=<r> ms` ✓
  - Stats: `--- <host> ping statistics ---` + `N transmitted, M received, X% packet loss, time Yms` ✓
  - RTT line: `rtt min/avg/max/mdev = a/b/c/d ms` ✓
  - Tek fark: RTT precision (3 decimal vs 1 decimal) — subject "RTT hariç" dediği için OK
- ✅ T-521 — RTT ±30 ms tolerans dahilinde (lokal: 0.043 ms, 8.8.8.8: 23-72 ms)
- ✅ T-522 — Statistics format birebir uyumlu (sistem ping ile diff test edildi)

---

## Faz 6 — Dokümantasyon

- ✅ T-601 — `docs/main/en.subject.pdf` kaynağa konuldu
- ✅ T-602 — `docs/rules/` — kural setleri (8 dosya)
- ✅ T-603 — `docs/command/` — OS/build komutları (5 dosya + COMMAND.md)
- ✅ T-604 — `docs/presentation/presentation.md` — defense slaytları
- ✅ T-605 — `docs/report/report.md` — proje raporu
- ✅ T-606 — `docs/normcheck/NORMCHECK.md` — Norminette referansı
- ✅ T-607 — `docs/cppcheck/CPPCHECK-MISRA-C2012.md` + `MISRA-REPORT.md` — MISRA referansı + rapor
- ✅ T-608 — Repo kök `README.md` (proje tanıtımı + quality badges + komut referansı)
- ☐ T-609 — `LICENSE` dosyası (opsiyonel)
- ✅ T-610 — `.gitignore` `obj/`, `output/`, `build/`, `*.dump`, `cppcheck.txt` içeriyor

---

## Faz 7 — Bonus (Mandatory PERFECT olduktan sonra)

> ⚠️ Bonus, mandatory part **kusursuz** olmadan değerlendirilmez.

- ☐ T-701 — `-c <count>` paket sayısı limiti
- ☐ T-702 — `-i <interval>` interval kontrolü
- ☐ T-703 — `-s <size>` payload boyutu
- ☐ T-704 — `-w <deadline>` toplam süre
- ☐ T-705 — `-W <timeout>` reply timeout
- ☐ T-706 — `-t / --ttl <n>` TTL kontrolü
- ☐ T-707 — `-n` reverse DNS kapat
- ☐ T-708 — `-p <pattern>` payload pattern
- ☐ T-709 — `-r` route bypass
- ☐ T-710 — `-T / --ip-timestamp` IP timestamp option
- ☐ T-711 — `-f` flood (root-only) — dikkatli implementasyon
- ☐ T-712 — `-l <preload>` preload paketleri

---

## Faz 8 — Defense Hazırlığı

- ✅ T-801 — `make re` warning vermiyor (compile satırlarında 0 warning/error)
- ✅ T-802 — Norminette raporu temiz (0 error, 30/30 dosya OK!)
- ✅ T-803 — Valgrind raporu temiz (5 path: 0 leak, 0 error)
- ✅ T-804 — `inetutils-2.0` ile diff temiz (banner + per-packet + stats birebir)
- ✅ T-805 — Sunum mevcut (`docs/presentation/presentation.md`, 21 slayt)
- ✅ T-806 — Q&A bölümü mevcut (sunum slayt 20)
- ✅ T-807 — `git status` temiz; 3 mantıksal commit (`bc32d32` chore, `03697e3` refactor, `c33eb30` add infra+docs)
- ✅ T-808 — Kernel `6.6.87.2-microsoft-standard-WSL2` > 3.14

---

## Genel İlerleme

| Faz                          | Tamamlanan / Toplam | %    |
|------------------------------|---------------------|------|
| 1 — Subject Mandatory        | 25 / 25             | 100  |
| 2 — Hata Yönetimi            | 11 / 11             | 100  |
| 3 — Kalite Denetimi          | 22 / 22             | 100  |
| 4 — Geliştirme Ortamı        | 14 / 14             | 100  |
| 5 — Test & Doğrulama         | 13 / 13             | 100  |
| 6 — Dokümantasyon            | 9 / 10              | 90   |
| 7 — Bonus                    | 0 / 12              | 0    |
| 8 — Defense Hazırlığı        | 8 / 8               | 100  |

> Yüzdeler kabaca tahminidir; gerçek durum statik & dinamik analiz
> sonuçlarına göre güncellenmelidir.

---

## Sonraki Adımlar (Önerilen Sıra)

1. ~~**T-107** — Makefile’a `-MMD -MP` ekle.~~ ✅
2. ~~**T-403** — `docker-compose.yml`’a `cap_add` ekle.~~ ✅
3. ~~**T-410/411/412/413** — CMake + norm/cppcheck/test target’ları.~~ ✅
4. ~~**T-406** — Container üzerinde `make all && make test` doğrula.~~ ✅ (4/4 PASS)
5. ~~**T-301** — Norminette ihlallerini sıfırla.~~ ✅ (1748 → 0)
6. ~~**T-209** — `make valgrind` temiz.~~ ✅ (4/4 path: 0 leak, 0 error)
7. ~~**T-310** — `make misra` raporu.~~ ✅ ([MISRA-REPORT.md](../cppcheck/MISRA-REPORT.md))
8. ~~**T-608** — Repo kök `README.md`.~~ ✅
9. ~~**T-520** — `inetutils-2.0` çıktı uyumu.~~ ✅
10. ~~**T-501..T-506 (live)** — gerçek paket testleri.~~ ✅
11. ~~**T-408** — Make + CMake + CTest doğrulandı.~~ ✅
12. ~~**T-211** — EPERM mesajı.~~ ✅
13. ~~**T-210** — AddressSanitizer.~~ ✅
14. ~~**T-405** — Multi-stage Docker (slim 123 MB).~~ ✅
15. ~~**T-514** — TTL exceeded verbose test.~~ ✅
16. ~~**T-801..T-808 (defense)** — readiness checklist.~~ ✅ (T-807 commit hariç)
17. **T-807** — `git status` temizliği için commit (kullanıcı kararı).
18. **Mandatory ARTIK PERFECT** → **Faz 7 (Bonus)**’ya geç.

---

## Layer 3 Refactor Notu (2026-05-02)

Layer 2 yaklaşımı (`PF_PACKET + ETH_P_ALL` + manuel ARP/Ethernet/IP)
Layer 3 yaklaşımına (`AF_INET + SOCK_RAW + IPPROTO_ICMP`) dönüştürüldü.

**Silinen dosyalar (9):**
- `create_ethernet_frame.c`, `create_ip_header.c`
- `get_network_default_gateway.c`, `get_network_gateway_mac_address.c`
- `get_network_src_mac_addr.c`, `get_network_source_ip_adress.c`
- `get_network_interface_index.c`, `get_network_interface_name.c`
- `calculate_rtt.c` (kullanılmayan ölü kod)

**Eklenen:**
- `compute_total_ms.c` (stats için)

**t_ping struct sadeleştirildi:**
- Çıkarılan field'lar: `src_ip_addr`, `ip_header`, `network_interface_name`,
  `src_mac`, `dest_mac`, `gateway_mac`, `gateway_ip`, `ifreq`,
  `ethernet_frame`, `icmp_reply`

**Sonuç:**
- src/ dosya sayısı: 27 → 19
- MISRA finding: 200 → 128
- Norminette: 0 error (korundu)
- Build: temiz
- Valgrind: 0 leak (live ping dahil)
- Live ping 8.8.8.8 / 1.1.1.1 / 127.0.0.1: ✅
- Format `inetutils-2.0` ile birebir

---

## Build Sistemi Komut Özeti

Bu fazlardaki kalite & test hedefleri **Makefile** ve **CMake** üzerinden
çalıştırılabilir; her ikisi de **Docker container** içinde aynı sonucu
verir.

### Makefile

```bash
make                # subject mandatory: build
make re             # fclean + all
make norm           # Ecole 42 norminette
make cppcheck       # cppcheck genel analiz
make misra          # MISRA C:2012 (cppcheck addon)
make test           # smoke tests (CLI/exit-code)
make valgrind VALGRIND_HOST=127.0.0.1
make asan           # AddressSanitizer build
make strict         # ekstra strict warning'ler ile build
make diff-ref       # inetutils ping ile diff
make help           # tüm target'ların listesi
```

### CMake

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j
cmake --build build --target norm
cmake --build build --target cppcheck
cmake --build build --target misra
cmake --build build --target check     # ctest --output-on-failure
cmake --build build --target valgrind
cmake --build build --target quality   # norm + cppcheck + check
```

### Docker

```bash
docker compose -f docker/docker-compose.yml up -d --build
docker exec -it ft_ping_container bash -lc "make && make test"
docker exec -it ft_ping_container bash -lc "make norm"
docker exec -it ft_ping_container bash -lc "make cppcheck"
docker exec -it ft_ping_container bash -lc "make misra"
docker exec -it ft_ping_container bash -lc "make valgrind"
docker exec -it ft_ping_container bash -lc \
    "cmake -S . -B build && cmake --build build --target quality"
```
