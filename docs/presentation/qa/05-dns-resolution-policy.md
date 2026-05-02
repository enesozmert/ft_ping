# Q5 — DNS Resolution Paket Dönüşünde Niçin Yapılmıyor?

> *"`ping google.com` yazıyorum, ama her reply satırında `1.1.1.1`
> görüyorum, hostname görmüyorum. Hatta `inetutils ping` reverse DNS
> yapıyor. ft_ping niçin yapmıyor?"*

---

## Kısa Cevap

İki neden:

1. **Subject açıkça yasaklıyor** (Chapter IV — Mandatory Part):
   > "You will have to manage FQDN without doing the **DNS resolution
   > in the packet return**."

2. **Performans bozulması**: her reply için reverse DNS query (PTR
   record) ek RTT ekler — tam ölçmek istediğimiz şeyin üzerine bir
   kat daha gecikme bindirir.

ft_ping reverse DNS yalnızca **başlangıçta**, hostname → IP yönünde
yapar (Q4'teki `getaddrinfo`). Her paket dönüşünde IP string olarak
basılır.

---

## DNS'in İki Yönü

| Lookup tipi    | Adı           | Record  | Ne yapar              |
|----------------|---------------|---------|-----------------------|
| **Forward**    | hostname → IP | A, AAAA | `google.com → 1.2.3.4`|
| **Reverse**    | IP → hostname | PTR     | `1.2.3.4 → google.com`|

`inetutils-2.0` ping default modda her reply'de **reverse DNS**
çağırır:

```
ping google.com
PING google.com (142.250.185.110) 56(84) bytes of data.
64 bytes from ams15s23-in-f14.1e100.net (142.250.185.110): icmp_seq=1 ...
                ^^^^^^^^^^^^^^^^^^^^^^^^                  ^^^^^^^^^^^^^^^^
                ← reverse DNS (PTR)                       ← original IP
```

**`-n`** flag'i ile reverse DNS kapatılabilir (sadece IP gösterir).
ft_ping default'u zaten `-n` davranışıdır:

```
PING 1.1.1.1 (1.1.1.1) 56(84) bytes of data.
64 bytes from 1.1.1.1: icmp_seq=1 ttl=63 time=49.431 ms
```

---

## Subject Yasağı — Niçin?

### 1. Asıl Ölçüm Bozulması

ICMP echo amaçlı network round-trip ölçer. Eğer her reply için **PTR
lookup** yapılırsa:

```
[ICMP echo request → reply] = T_network
[PTR query → response]      = T_dns        ← extra delay
[printf format]             = T_format
```

`time=R ms` çıktısı ICMP RTT'yi yansıtmalı; reverse DNS gecikmesini
karıştırırsa kullanıcı "internet yavaş" sanırken aslında DNS yavaştır.

### 2. Reverse DNS Genelde Eksik veya Yavaş

ISP veya CDN IP'lerinin çoğunun **PTR record**'u tanımlı değildir veya
generic'tir:

```
8.8.8.8         → dns.google.        (kısa, OK)
1.2.3.4         → 1-2-3-4.somenet.x. (anlamsız)
random.cloud.ip → NXDOMAIN           (10s timeout sonrası)
```

NXDOMAIN response için DNS resolver birkaç **retry + timeout** yapar
(default 5 sn). 4 paket göndereceksen 20 sn ekstra gecikme.

### 3. Network Trafiği

Her reply için ek 2 paket (DNS query + response). Yüksek paket-loss
testlerinde DNS de loss olabilir, sonsuz timeout'a girer.

### 4. Defense / Test Determinism'i

Subject (R-04): "Tolerance: ±30 ms on the reception of a packet."
Reverse DNS rastgele ekstra gecikme yaratırsa bu tolerans aşılabilir.

---

## Forward Resolution (`getaddrinfo`) — İzinli

Subject yasağı yalnızca **paket dönüşündeki reverse DNS** içindir.
**Forward** (hostname → IP) lookup zorunludur — kullanıcı `google.com`
yazınca `getaddrinfo` ile IP bulunur. Bu **bir kez**, başlangıçta
yapılır:

[`src/resolve_hostname.c`](../../../src/resolve_hostname.c):

```c
result = getaddrinfo(hostname, NULL, &hints, &res);
if (result != 0)
{
    fprintf(stderr, "ping: %s: %s\n", hostname, gai_strerror(result));
    return (1);
}
addr = (struct sockaddr_in *)res->ai_addr;
inet_ntop(AF_INET, &addr->sin_addr, ip_str, ip_str_len);
freeaddrinfo(res);
```

[`src/main.c`](../../../src/main.c):

```c
if (resolve_hostname(hostname, ip_buf, ip_buf_len) != 0)
    return (1);
ping->dest_ip_addr = ip_buf;       /* IP string — printf için */
ping->dest_hostname = hostname;    /* asıl yazılan hostname — banner için */
```

Sonuç:
- `dest_hostname` banner'da gösterilir: `PING google.com (...)`
- `dest_ip_addr` her reply line'da gösterilir: `64 bytes from 1.1.1.1: ...`

---

## Reverse DNS Bonus Olarak Eklenebilir

`-n` flag'i (no DNS) standart; **default** `inetutils-2.0` reverse DNS
yapar. Bonus implementasyon:

```c
/* hint:
 * char host[NI_MAXHOST];
 * getnameinfo((struct sockaddr *)&from, sizeof(from),
 *             host, sizeof(host), NULL, 0, 0);
 */
```

Subject açıkça yasakladığı için ft_ping mandatory'de **yapmıyor**;
bonus part'a `-r` (reverse DNS aç) bayrağı eklemek mümkün — ama subject
bonus listesinde değil, custom bayrak olur.

---

## DNS Resolution Akışı (`getaddrinfo`)

```
Application        getaddrinfo("google.com", "...", hints, &res)
        │
        ▼
nsswitch (configurable)
        │
        ├──> /etc/hosts  (local override; OK ise dön)
        │
        └──> DNS resolver (libresolv)
                │
                ▼
          resolv.conf içindeki nameserver'lar
                │
                ▼
          UDP 53 query → DNS server
          ┌────────────┐
          │ DNS server │
          └────────────┘
                │
                ▼
          A record (IPv4) veya AAAA (IPv6) cevap
                │
                ▼
        struct addrinfo *res döner
```

`/etc/resolv.conf` örneği:

```
nameserver 1.1.1.1
nameserver 8.8.8.8
options timeout:2 attempts:3
```

---

## Test Senaryoları

| Komut                            | Davranış                                          |
|----------------------------------|---------------------------------------------------|
| `ft_ping 8.8.8.8`                | `getaddrinfo` numerik IP'yi tanır, DNS atlamaz    |
| `ft_ping google.com`             | Forward DNS (A record), hostname banner'da kalır  |
| `ft_ping nonexistent.invalid.tld`| `EAI_NONAME` → `Name or service not known` + exit 1 |
| `ft_ping ::1`                    | `AF_INET` filtre nedeniyle reddedilir (IPv4-only) |
| `ft_ping 127.0.0.1`              | Loopback; DNS yok, anında IP                      |

---

## Defense'ta Olası Follow-up'lar

**Q:** *Forward DNS de yavaş olabilir; bunu cache'liyor musun?*
A: `getaddrinfo` glibc düzeyinde nscd (Name Service Cache Daemon) ile
cache'lenir (sistem-genel). ft_ping kendi içinde cache yapmaz — `ping
host` ile bir kez çağırır, sonra IP'yi `dest_ip_addr` field'da tutar.
Tekrar lookup yapmaz.

**Q:** *Hostname'i banner'da, IP'yi reply line'larda gösteriyorsun;
inetutils tam tersi?*
A: inetutils default'ta her ikisi de hostname (banner + reply'lerde
reverse DNS sonrası), `-n` ile her ikisi de IP. ft_ping default `-n`
modu — banner'da kullanıcının verdiği hostname, reply'lerde IP. Subject
"reverse DNS yapma" dediği için bu davranış doğru.

**Q:** *IPv6 hostname'i girmek ne olur?*
A: `getaddrinfo` `AF_INET` hint nedeniyle yalnızca A record arar; AAAA
varsa atlar. Eğer hostname **sadece** AAAA record'a sahipse →
`EAI_NONAME`. Subject IPv4-only olduğu için bu beklenen davranış.

**Q:** *DNS resolution timeout'u nasıl yönetiyorsun?*
A: `getaddrinfo` blocking; `/etc/resolv.conf`'ta `options timeout:N
attempts:M` ile sistem geneli. ft_ping kendi timeout koymaz — DNS
yavaşsa `getaddrinfo` blokes.

**Q:** *DNS poisoning'e karşı?*
A: ft_ping zaten **trust-the-system** modeli — sistem resolver'ına
güvenir. DNSSEC veya DoH/DoT bizim scope'umuz dışı. Banner'da hostname
+ IP birlikte gösterildiği için kullanıcı resolved IP'yi görür ve
manuel doğrulayabilir.

---

## Referanslar

- **Subject** Chapter IV — Mandatory Part: "FQDN without doing the DNS
  resolution in the packet return"
- **RFC 1035** — Domain Names: Implementation and Specification
- **RFC 1912** — Common DNS Operational and Configuration Errors
- **RFC 3596** — DNS Extensions to Support IPv6 (AAAA, PTR for v6)
- **`man 3 getaddrinfo`**, **`man 3 getnameinfo`** (reverse — used by
  `-n` opt-out variants)
- **`man 5 resolv.conf`**, **`man 5 nsswitch.conf`**
- **`man 8 nscd`** — Name Service Cache Daemon
- inetutils `ping/ping_common.c` — `pr_addr()` reverse DNS implementasyonu
