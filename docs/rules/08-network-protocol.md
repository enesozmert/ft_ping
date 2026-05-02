# 08 — Network Protocol Rules (ICMP / IPv4)

> ft_ping, **ICMP Echo Request / Reply** üzerine kuruludur.
> Bu dosya, RFC 791 (IP) ve RFC 792 (ICMP) ile uyumluluk için zorunlu
> protokol kurallarını tanımlar.

---

## NP-01: IPv4 Header (RFC 791 §3.1)

| Field             | Boyut    | Notlar                                          |
|-------------------|----------|-------------------------------------------------|
| Version           | 4 bit    | `4` (IPv4)                                      |
| IHL               | 4 bit    | `5` (header length, 32-bit words)               |
| Type of Service   | 8 bit    | Genelde `0`                                     |
| Total Length      | 16 bit   | Header + payload (network byte order)           |
| Identification    | 16 bit   | Unique per packet                               |
| Flags             | 3 bit    | DF/MF flag’leri                                 |
| Fragment Offset   | 13 bit   | Genelde `0`                                     |
| TTL               | 8 bit    | Default `64` (Linux); subject `--ttl` ile       |
| Protocol          | 8 bit    | `1` (ICMP)                                      |
| Header Checksum   | 16 bit   | Header üzerinde 1’s complement sum              |
| Source Address    | 32 bit   | Outgoing interface IP                           |
| Destination Addr  | 32 bit   | Hedef IP                                        |

---

## NP-02: ICMP Header (RFC 792)

| Field         | Boyut    | Echo Request / Reply Değeri                       |
|---------------|----------|---------------------------------------------------|
| Type          | 8 bit    | `8` (request) / `0` (reply)                       |
| Code          | 8 bit    | `0`                                               |
| Checksum      | 16 bit   | ICMP header + data üzerinde 1’s complement sum    |
| Identifier    | 16 bit   | Genelde `getpid() & 0xFFFF` (network byte order)  |
| Sequence      | 16 bit   | 0’dan başlar, her paketle artar                   |

ICMP payload genelde:
- 8 byte timestamp (`gettimeofday`) — RTT hesabı için
- Ardından pattern data (default 48 byte; toplam 56 byte payload)

---

## NP-03: Checksum Algoritması (RFC 1071)

```c
uint16_t checksum(const void *data, size_t len)
{
    const uint16_t *buf = data;
    uint32_t sum = 0;

    while (len > 1) { sum += *buf++; len -= 2; }
    if (len == 1) sum += *(const uint8_t *)buf;

    while (sum >> 16) sum = (sum & 0xFFFF) + (sum >> 16);
    return (uint16_t)~sum;
}
```

- Hesap **network byte order** datası üzerinde yapılır.
- Önce checksum field’ı `0` olarak set edilir.
- Sonuç field’a yazılır (zaten network byte order’da).

---

## NP-04: Raw Socket

```c
int sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
```

- **`CAP_NET_RAW`** capability gerektirir (veya root).
- `IP_HDRINCL` socket option **kullanılmaz** (kernel header’ı oluşturur).
- `setsockopt(IP_TTL)` ile TTL set edilebilir (`--ttl` bonus).
- `SO_RCVTIMEO` ile timeout ayarlanabilir.

> Alternatif: `SOCK_DGRAM + IPPROTO_ICMP` (Linux’ta `ping_group_range`
> sysctl ile capability gerektirmez). Subject **explicit** raw socket
> ister mi? Subject sadece "ping recoder" olduğunu söyler; her ikisi de
> kabul edilebilir, ama gerçek `inetutils-2.0` davranışı için **raw**
> tercih edilmelidir.

---

## NP-05: Hostname Resolution

- **Forward resolution** (hostname → IP) için `getaddrinfo()` kullanılır.
  - `AF_INET` (IPv4) zorla.
  - `SOCK_RAW` hint.
- **Reverse resolution** (IP → hostname) `getnameinfo()` ile yapılır.
- Subject (R-02): "FQDN’i yöneteceksin ama paket dönüşünde DNS resolution
  **yapmayacaksın**." → Reverse resolution **yalnızca başlangıçta**
  yapılır, her paket dönüşünde değil.

---

## NP-06: RTT Hesabı

```c
struct timeval start, end;
gettimeofday(&start, NULL);
/* paket gönder */

/* paket al */
gettimeofday(&end, NULL);

double rtt_ms = (end.tv_sec - start.tv_sec) * 1000.0
              + (end.tv_usec - start.tv_usec) / 1000.0;
```

- Subject (R-04): RTT için **±30 ms** tolerans.
- Daha iyi precision için `clock_gettime(CLOCK_MONOTONIC, ...)` tercih
  edilebilir (sistem saatine bağımlı değil).

---

## NP-07: Statistics

`inetutils-2.0` çıktı formatı:

```
PING example.com (93.184.216.34) 56(84) bytes of data.
64 bytes from ...: icmp_seq=1 ttl=56 time=12.3 ms
...

--- example.com ping statistics ---
N packets transmitted, M received, X% packet loss, time T ms
rtt min/avg/max/mdev = a/b/c/d ms
```

Tutulması gereken sayaçlar:
- `transmitted` — gönderilen toplam paket
- `received` — başarılı reply
- `errors` — error response
- `min`, `max`, `sum`, `sum_sq` — RTT istatistikleri için
- `mdev` (mean deviation) hesabı:
  ```
  variance = sum_sq/n - (sum/n)^2
  mdev = sqrt(variance)
  ```

---

## NP-08: Error Cases

| ICMP Type / Code | Anlam                          | ft_ping Davranışı                |
|------------------|--------------------------------|----------------------------------|
| 3 / 0            | Network unreachable            | log + counter++                  |
| 3 / 1            | Host unreachable               | log + counter++                  |
| 3 / 3            | Port unreachable               | log + counter++                  |
| 3 / 4            | Fragmentation needed (DF set)  | log + counter++ (PMTU)           |
| 11 / 0           | TTL exceeded in transit        | log (traceroute benzeri davranış)|
| 11 / 1           | Fragment reassembly time exc.  | log + counter++                  |
| 0 / 0            | Echo Reply                     | normal flow                      |

> `-v` (verbose) modunda tüm ICMP type/code log’lanmalıdır.
