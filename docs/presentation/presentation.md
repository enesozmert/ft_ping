# ft_ping — Sunum (Presentation)

> Hedef: Projeyi defense / peer-evaluation / topluluk önünde **anlatmak**
> için bire bir subject ile uyumlu, akış halinde bir sunum dokümanı.
>
> Format: Tek slayt = tek başlık (`---` ile ayrılır). İçerik **Türkçe**,
> teknik kavramlar **İngilizce** bırakılmıştır.

---

## Slayt 1 — Kapak

# **ft_ping**
### Ecole 42 — Network Programming Project
**Version:** 5.1
**Konu:** *Recoding the `ping` command*

> "Ping is the name of a command that allows you to test the
> accessibility of another machine through the IP network."
> *— en.subject.pdf, Chapter II*

---

## Slayt 2 — Foreword: Ettore Majorana

- **Ettore Majorana** (5 Ağustos 1906 — 1959 sonrası kayıp): İtalyan
  **theoretical physicist**.
- **Particle physics** ve özellikle **neutrino theory** alanlarındaki
  çalışmalarıyla tanınır.
- 1938 baharındaki gizemli kayboluşu, intihar veya gönüllü kayboluş
  gibi pek çok spekülasyona yol açmıştır.
- Galilei ve Newton seviyesinde **deha** (genius) olarak anılır
  (Enrico Fermi ile çalıştığı dönemden).

> Subject Foreword (Chapter I) — bu projeye ruhunu veren tarihsel
> kişilik. Bilimsel titizlik ve sezgisel kavrayışın simgesi.

---

## Slayt 3 — Introduction: Ping Nedir?

- **Ping**, bir **IP network** üzerinden başka bir makinenin
  **accessibility**’sini (erişilebilirlik) test eden komuttur.
- **Round-trip time (RTT)** ölçer: paketin gidip dönmesi için geçen
  süre.
- ICMP **Echo Request** ↔ ICMP **Echo Reply** mantığına dayanır
  (RFC 792).

```
Host A  ── ICMP Echo Request ──▶  Host B
Host A  ◀── ICMP Echo Reply  ──   Host B
```

---

## Slayt 4 — Projenin Amacı

Subject Chapter II: **"This project is about recoding the ping
command."**

Yani:
1. **Linux ping**’in (referans: `inetutils-2.0`) davranışını
   **C dilinde** sıfırdan yeniden yazmak.
2. **Sistem ping**’i veya **standart bir ping**’in kaynak kodlarını
   **kullanmamak**.
3. **`libc`** dışındaki kütüphanelerden **kaçınmak**.

---

## Slayt 5 — General Instructions (Chapter III)

### Geliştirme Ortamı
- **Debian (≥ 7.0)** üzerinde **virtual machine**.
- VM, projeyi tamamlamak için gerekli yazılımları **kurulu** ve
  **konfigüre** içermelidir.
- VM, **cluster computer** üzerinden kullanılabilmelidir.

### Teknik Kurallar
- Dil: **C**.
- Build: **`Makefile`** (zorunlu).
- Compile flags: `-Wall -Wextra -Werror`.
- Hatalar: program **segfault / bus error / double free** ile
  sonlanamaz.
- Kütüphane: yalnızca **libc**.

> ⚠️ **DİKKAT:** Sistem `ping` komutunu çağırmak veya standart bir
> ping implementasyonunun kaynaklarını kullanmak **yasaktır**.

---

## Slayt 6 — Mandatory Part (Chapter IV)

| ID  | Gereksinim                                                     |
|-----|----------------------------------------------------------------|
| M1  | Executable adı **`ft_ping`** olmalıdır.                        |
| M2  | Referans implementasyon **`inetutils-2.0`** (`ping -V`).       |
| M3  | **`-v`** (verbose) opsiyonu yönetilmelidir.                    |
| M4  | **`-?`** (help / usage) opsiyonu yönetilmelidir.               |
| M5  | Parametre olarak **IPv4** (address/hostname) yönetilmelidir.   |
| M6  | **FQDN** desteklenmelidir; paket dönüşünde **DNS resolution yok**. |

> `-v` modu paket hatalarını/anomalilerini de göstermelidir; ama bu
> **programı durdurmaya zorlamaz**. (örn. `--ttl=1` ile zorlanabilir.)

---

## Slayt 7 — Bonus Part (Chapter V)

> ⚠️ Bonus, **mandatory part PERFECT** ise değerlendirilir.

İlginç bonus fikirleri:

- **Ek bayraklar:** `-f`, `-l`, `-n`, `-w`, `-W`, `-p`, `-r`, `-s`,
  `-T`, `--ttl`, `--ip-timestamp`, ...

> Bonus sayılmayanlar: `-V`, `--usage`, `--echo`.
> Aynı özellik için iki bayrak (örn. `-t` + `--type`) **tek bonus**
> sayılır.

---

## Slayt 8 — Submission & Evaluation (Chapter VI)

- Teslim, **Git repository** üzerinden yapılır.
- VM kernel: **Linux > 3.14** (değerlendirme **Debian 7.0 stable**
  üzerine tasarlanmıştır).
- Çıktı **indentation**’u, **`inetutils-2.0`** ile **birebir** olmalıdır
  (RTT satırı ve reverse DNS hariç).
- Paket alımında **±30 ms** tolerans.

---

## Slayt 9 — Mimari Genel Bakış

```
┌─────────────────────┐
│      main.c         │  ← entry point, argument parse
└──────────┬──────────┘
           ▼
┌─────────────────────┐
│   parse_args.c      │  ← getopt, -v, -?, host
└──────────┬──────────┘
           ▼
┌─────────────────────┐
│ resolve_hostname.c  │  ← getaddrinfo (DNS)
└──────────┬──────────┘
           ▼
┌─────────────────────┐
│ create_raw_socket.c │  ← socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)
└──────────┬──────────┘
           ▼
┌─────────────────────┐  ┌────────────────────┐
│  create_packet.c    │→ │  create_icmp_hdr.c │
│  create_payload.c   │  │  create_ip_hdr.c   │
│  checksum.c         │  └────────────────────┘
└──────────┬──────────┘
           ▼
┌─────────────────────┐
│ create_send_request.c │ ← sendto loop + signal
└──────────┬──────────┘
           ▼
┌─────────────────────────────┐
│ create_socket_select.c      │ ← select() timeout
│ create_socket_recvfrom.c    │ ← recvfrom + parse reply
│ calculate_rtt.c             │ ← gettimeofday delta
└─────────────────────────────┘
```

---

## Slayt 10 — Kullanılan System Calls

| Syscall            | Amaç                                  |
|--------------------|---------------------------------------|
| `socket()`         | Raw ICMP socket aç                    |
| `setsockopt()`     | TTL, timeout ayarı                    |
| `getaddrinfo()`    | Hostname → IPv4 resolve               |
| `getnameinfo()`    | IPv4 → hostname (yalnızca başlangıçta)|
| `sendto()`         | ICMP Echo Request gönder              |
| `recvfrom()`       | ICMP Echo Reply al                    |
| `select()`         | Timeout’lu bekleme                    |
| `gettimeofday()`   | RTT ölçümü                            |
| `signal()`         | SIGINT (Ctrl+C) yakalama              |

---

## Slayt 11 — Demo Senaryoları

### 1. Direct IP

```bash
$ sudo ./ft_ping 8.8.8.8
PING 8.8.8.8 (8.8.8.8): 56 data bytes
64 bytes from 8.8.8.8: icmp_seq=1 ttl=117 time=12.4 ms
...
```

### 2. Hostname + FQDN

```bash
$ sudo ./ft_ping google.com
$ sudo ./ft_ping www.example.com
```

### 3. Verbose ve Hata Durumu

```bash
$ sudo ./ft_ping -v --ttl=1 8.8.8.8
# TTL exceeded → ICMP type 11 görünür
```

### 4. Help

```bash
$ ./ft_ping -?
```

### 5. Unknown Host

```bash
$ ./ft_ping nonexistent.invalid.tld
ft_ping: unknown host
```

---

## Slayt 12 — Karşılaştırma: ft_ping vs inetutils-2.0

```
$ ping -V
ping (GNU inetutils) 2.0
...

$ ping -c 3 google.com
PING google.com (142.250.185.110): 56 data bytes
64 bytes from ...: icmp_seq=1 ttl=117 time=12.3 ms
...
--- google.com ping statistics ---
3 packets transmitted, 3 received, 0% packet loss, time 2003ms
rtt min/avg/max/mdev = 12.3/12.5/12.7/0.2 ms
```

**ft_ping çıktısı (RTT ve reverse DNS hariç) birebir aynı indentation’a
sahiptir.**

---

## Slayt 13 — Hata Yönetimi

| Hata Türü                  | Davranış                                  |
|----------------------------|-------------------------------------------|
| Segmentation fault         | **Yok** (subject yasaklıyor)              |
| Bus error                  | **Yok**                                   |
| Double free                | **Yok**                                   |
| `socket: EPERM`            | "Operation not permitted" + exit          |
| `unknown host`             | error mesajı + exit(2)                    |
| `recvfrom timeout`         | counter++ + sonraki paket                 |
| Checksum mismatch          | discard + counter++                       |
| `SIGINT` (Ctrl+C)          | statistics flush + exit                   |

> Doğrulama: **Valgrind** `--leak-check=full` ile **0 leak**.

---

## Slayt 14 — Test Stratejisi

### Statik Analiz

- **Norminette** → `norminette src/ hdr/`
- **Cppcheck** → `cppcheck --enable=all --std=c11 src/`
- **MISRA C:2012** → `cppcheck --addon=misra src/*.dump`

### Dinamik Analiz

- **Valgrind** → memory leak / invalid read.
- **AddressSanitizer** → out-of-bounds, use-after-free.
- **Strace** → syscall doğruluğu.
- **tcpdump / Wireshark** → wire-level paket doğrulaması.

### Karşılaştırma

- `inetutils-2.0` referansıyla **diff**.

---

## Slayt 15 — Geliştirme Ortamı

```
┌────────────────────────────────────────┐
│  Host: Linux / macOS / Windows         │
│  ┌───────────────────────────────────┐ │
│  │  Docker Desktop / Engine          │ │
│  │  ┌─────────────────────────────┐  │ │
│  │  │  ft_ping_container          │  │ │
│  │  │  Ubuntu 22.04 + build-essen │  │ │
│  │  │  CAP_NET_RAW eklendi        │  │ │
│  │  │  SSH (port 2222 forward)    │  │ │
│  │  └─────────────────────────────┘  │ │
│  └───────────────────────────────────┘ │
└────────────────────────────────────────┘
```

`docker compose up -d` → container hazır → `make && ./ft_ping host`.

---

## Slayt 16 — Build Sistemi

```makefile
CC      = gcc
CFLAGS  = -Wall -Wextra -Werror -g -Ihdr
SRCDIR  = src
OBJDIR  = obj
NAME    = output/ft_ping

all: $(NAME)
$(NAME): $(OBJS) ; $(CC) $(CFLAGS) -o $@ $^
$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@
clean: ; rm -f $(OBJS)
fclean: clean ; rm -f $(NAME)
re: fclean all
```

> `all`, `clean`, `fclean`, `re` — subject zorunlu rules.

---

## Slayt 17 — Kullanılan Kural Setleri

1. **Subject** (en.subject.pdf v5.1)
2. **Ecole 42 Norminette** (Norm v2.0.2)
3. **MISRA C:2012**
4. **Docker iyi pratikleri**
5. **Network protocol**: RFC 791 (IP), RFC 792 (ICMP), RFC 1071
   (checksum)

> Her kural seti `docs/rules/` altında ayrı dosyada belgelenmiştir.

---

## Slayt 18 — Öğrenilenler (Lessons Learned)

- **Raw socket** ve **CAP_NET_RAW** — ayrıcalık modeli (POSIX vs Linux).
- **ICMP checksum** — 1’s complement sum, RFC 1071.
- **IPv4 byte order** — `htons` / `ntohs` zorunluluğu.
- **`select()` timeout** ile non-blocking I/O.
- **`signal()` async-safety** — handler içinde printf yasak.
- **inetutils-2.0** çıktı formatının **karakter düzeyinde** uyumu.
- **Norminette + MISRA** birlikte uygulamanın trade-off’ları.

---

## Slayt 19 — Sınırlar (Limitations)

- IPv6 desteği **yok** (subject sadece IPv4 ister).
- Reverse DNS **yalnızca başlangıçta** yapılır (subject zorunluluğu).
- Multicast / broadcast hedefler test edilmemiştir.
- `setcap` olmayan ortamlarda **sudo** zorunlu.
- macOS’ta `<linux/icmp.h>` mevcut değildir; portability için
  POSIX-portable struct kullanılmıştır.

---

## Slayt 20 — Soru-Cevap (Q&A)

> **Sık sorulan defense soruları:**

1. *Raw socket nedir, niçin gerekiyor?*
   → ICMP, transport layer **dışında**dır. Kernel’ın TCP/UDP gibi
   açacağı default socket bunu ele alamaz; `SOCK_RAW` ile **protocol
   number 1 (ICMP)** doğrudan dinlenir/gönderilir.

2. *Checksum nasıl hesaplanır?*
   → RFC 1071: 16-bit chunk’lar over **1’s complement sum**, sonra
   sonucun complement’i. Detay: `src/checksum.c`.

3. *RTT nasıl hesaplanıyor?*
   → Paketin payload’ına gönderim anında `gettimeofday()` yazılır;
   reply alındığında fark hesaplanır.

4. *Niye getaddrinfo, gethostbyname değil?*
   → `gethostbyname` deprecated; `getaddrinfo` reentrant + IPv6-ready.

5. *DNS resolution paket dönüşünde niye yapılmıyor?*
   → Subject (R-02) açıkça yasaklıyor; ayrıca lookup latency RTT
   ölçümünü bozar.

---

## Slayt 21 — Kapanış

# Teşekkürler.

**ft_ping** — Ecole 42 Network Programming
*Submitted under subject v5.1 — Mandatory + (opsiyonel) Bonus*

📁 Repository: `ft_ping/`
📄 Documentation: [`docs/`](../)
🐳 Container: `docker compose up -d`
🔧 Build: `make && sudo ./output/ft_ping <host>`

> *"Galilei ve Newton gibi dehalar... Majorana onlardan biriydi."*
