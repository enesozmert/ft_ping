# Q3 — RTT Nasıl Hesaplanıyor?

> *"`time=12.345 ms` çıktısındaki sayı nereden geliyor? Ne zamanı
> ölçüyorsun, hangi clock kullanıyorsun?"*

---

## Kısa Cevap

**RTT** (Round-Trip Time): paketin gönderildiği an ile reply alındığı
an arasındaki süre. ft_ping `gettimeofday(2)` ile **mikrosaniye**
çözünürlüğünde iki timestamp alır, **end - start** farkını
**milisaniye** olarak yazar.

ft_ping kodu:
[`src/create_send_request.c`](../../../src/create_send_request.c) (start),
[`src/compute_total_ms.c`](../../../src/compute_total_ms.c) (delta),
[`src/create_socket_recvfrom.c`](../../../src/create_socket_recvfrom.c) (use).

---

## Akış

```
                          ╔══════════════════╗
                          ║  send_one(ping)  ║
                          ╚════════╤═════════╝
                                   │
                  ┌────────────────┴────────────────┐
                  ▼                                 ▼
   refresh_icmp_header(ping)               create_send_request(ping)
   - sequence++                            - gettimeofday(&start)  ◀── T0
   - checksum recalc                       - sendto(packet)
                                                   │
                                                   ▼
                                          create_socket_select(ping)
                                          - select(timeout=1s)
                                                   │
                                                   ▼
                                          create_socket_recvfrom(ping)
                                          - recvfrom(reply)
                                          - process_echo_reply():
                                              compute_elapsed_ms(start, end)
                                              - gettimeofday(&end)  ◀── T1
                                              - rtt_ms = (T1-T0)*1000
                                              - print "time=R ms"
```

---

## Kod Alıntıları

### Start timestamp — `create_send_request.c`

```c
int	create_send_request(t_ping *ping)
{
    ping->result->sent_packets++;
    gettimeofday(&ping->time.start_time, NULL);   ◀── T0
    if (sendto(ping->sock_fd, ping->packet->packet,
            ping->packet->packet_len, 0,
            (struct sockaddr *)ping->target_addr,
            sizeof(*ping->target_addr)) < 0)
    { ... return (-1); }
    ...
}
```

### End timestamp + delta — `compute_total_ms.c`

```c
double	compute_elapsed_ms(const struct timeval *start, struct timeval *end)
{
    double	ms;

    gettimeofday(end, NULL);                       ◀── T1
    ms = (end->tv_sec - start->tv_sec) * 1000.0;
    ms += (end->tv_usec - start->tv_usec) / 1000.0;
    return (ms);
}
```

### Reply işleme — `create_socket_recvfrom.c`

```c
static void	process_echo_reply(t_ping *ping, ssize_t len,
            const t_iphdr *ip_hdr, const t_icmphdr *icmp_hdr)
{
    double	rtt;

    ping->result->received_packets++;
    rtt = compute_elapsed_ms(&ping->time.start_time, &ping->time.end_time);
    ping->result->rtt += rtt;
    update_rtt_stats(ping->result, rtt);            ◀── min/max/sum_sq
    printf("%d bytes from %s: icmp_seq=%u ttl=%d time=%.3f ms\n",
        ..., rtt);
}
```

---

## `gettimeofday` Detayları

`gettimeofday(struct timeval *tv, NULL)` POSIX-2001'de tanımlıdır:

```c
struct timeval {
    time_t      tv_sec;     // saniye (Unix epoch'tan)
    suseconds_t tv_usec;    // mikrosaniye [0, 999999]
};
```

| Özellik           | Değer                                          |
|-------------------|-----------------------------------------------|
| Çözünürlük        | mikrosaniye (gerçekte ~1 µs Linux'ta)         |
| Monotonik mi?     | **Hayır** — wall-clock'tur, NTP/kullanıcı saat ayarından etkilenir |
| Thread-safe       | Evet                                          |
| Async-signal-safe | Evet (POSIX)                                  |

> ⚠️ **Modern alternatif**: `clock_gettime(CLOCK_MONOTONIC, ...)`
> nanosaniye çözünürlük + monotonik (saat değişiminden etkilenmez).
> ft_ping `inetutils-2.0` ile birebir uyum için `gettimeofday` kullanır;
> bonus olarak `clock_gettime` geçişi düşünülebilir.

---

## Mantıksal Akıl Yürütme

`tv_sec` ve `tv_usec`'i **`double` ms**'ye çevirme:

```
ms = (end.tv_sec - start.tv_sec) * 1000.0
   + (end.tv_usec - start.tv_usec) / 1000.0
```

| Koşul                  | Sonuç                                |
|------------------------|--------------------------------------|
| start = 5.500000 s     | "1 s 500000 µs"                      |
| end   = 5.523456 s     | "1 s 523456 µs"                      |
| diff_sec = 0           |                                      |
| diff_usec = 23456      |                                      |
| **ms** = 0*1000 + 23456/1000 = **23.456 ms** |                       |

Negatif `diff_usec` durumu (start.tv_usec > end.tv_usec):

```
start = 1.999000  → tv_sec=1, tv_usec=999000
end   = 2.001000  → tv_sec=2, tv_usec=001000

ms = (2-1)*1000 + (1000-999000)/1000
   = 1000 + (-998)
   = 2 ms ✓
```

Aritmetik **doğru sonucu** verir çünkü iki bileşen `double` arithmetic
ile birleşir; manual normalize gerekmez.

---

## RTT Statistics — Online Hesaplama

ft_ping ping loop'unda her başarılı reply için:

[`src/create_socket_recvfrom.c`](../../../src/create_socket_recvfrom.c) —
`update_rtt_stats`:

```c
static void	update_rtt_stats(t_ping_result *r, double rtt)
{
    if (r->received_packets == 1 || rtt < r->rtt_min)
        r->rtt_min = rtt;
    if (rtt > r->rtt_max)
        r->rtt_max = rtt;
    r->rtt_sum_sq += rtt * rtt;
}
```

`r->rtt` (toplam) zaten her reply'da `+= rtt` ile güncelleniyor.

### `mdev` (mean deviation) Hesabı

`inetutils ping` tarzı `rtt min/avg/max/mdev`:

[`src/interrupt_handler.c`](../../../src/interrupt_handler.c) —
`compute_mdev`:

```c
static double	compute_mdev(const t_ping_result *r)
{
    double	avg;
    double	variance;

    if (r->received_packets <= 0)
        return (0.0);
    avg = r->rtt / (double)r->received_packets;
    variance = (r->rtt_sum_sq / (double)r->received_packets) - (avg * avg);
    if (variance < 0.0)
        variance = 0.0;
    return (sqrt(variance));
}
```

**Welford** veya **two-pass** algoritması yerine **online formula**:

```
Var(X) = E[X²] - (E[X])²

avg     = sum(rtt) / n
variance = sum(rtt²)/n - avg²
mdev    = sqrt(variance)
```

> Pedantik nokta: bu **standard deviation** (σ), iletişimde "mean
> deviation" (MAD) terim olarak yanlış. Ama `inetutils ping` "mdev"
> yazıyor, biz de aynı isim ile gidiyoruz (output uyumu).

> **Floating-point safety**: `sum_sq/n - avg²` aritmetiği nadir
> durumlarda `variance < 0` üretebilir (catastrophic cancellation).
> ft_ping `if (variance < 0.0) variance = 0.0;` ile clamp eder.

---

## Subject Toleransı

Subject (R-04):
> "A delay of +/- 30ms is tolerated on the reception of a packet."

ft_ping yerel testlerde **0.043 ms** (loopback), uzak host'larda
**20–80 ms** RTT verir. Sistem ping ile karşılaştırma diff'inde 30 ms
toleransı içinde kalır.

---

## RTT Çıktı Format Detayları

ft_ping format:

```
64 bytes from 1.1.1.1: icmp_seq=1 ttl=63 time=49.431 ms
```

inetutils-2.0 format:

```
64 bytes from 1.1.1.1: icmp_seq=1 ttl=63 time=49.4 ms
```

**Tek fark**: precision (`%.3f` vs `%.1f`). Subject "RTT line ve reverse
DNS hariç" indentation uyumu istediği için bu kabul edilebilir. Bonus
olarak adaptive precision (RTT < 1ms → 3 decimal, > 1ms → 1 decimal)
eklenebilir.

---

## Defense'ta Olası Follow-up'lar

**Q:** *Niçin `time(NULL)` veya `clock()` değil?*
A: `time` saniye çözünürlüklü (ms ölçemez). `clock()` CPU time döner —
real time değil. `gettimeofday` mikrosaniye + wall-clock + her UNIX'te
mevcut.

**Q:** *Sistem saati değişirse ne olur?*
A: `gettimeofday` wall-clock'tur — NTP düzeltmesi sırasında atlamalar
olabilir, RTT negatif veya saçma çıkabilir. `CLOCK_MONOTONIC`
(`clock_gettime`) daha güvenli ama subject scope dışı.

**Q:** *Timestamp'i payload'a niye yazmıyorsun?*
A: BSD ping payload'a `gettimeofday` yazıp reply'da geri alır — paket
sırası karıştığında bile doğru RTT hesabı yapar (echo reply içinde
orijinal gönderim zamanı taşınır). ft_ping bunu yapmıyor; ardışık
gönderim varsayar. `-c 1 -i 0` flood'da hata payı oluşturabilir.

**Q:** *RTT < 1 ms durumunda precision kayıp olur mu?*
A: `gettimeofday` µs çözünürlük → `tv_usec / 1000.0` `double`'a
çevrilir, 0.001 ms (1 µs) çözünürlük korunur. `%.3f` print mask 1 µs
gösterebilir.

**Q:** *Niçin loopback RTT'si 0.04 ms ve 8.8.8.8 RTT'si 50 ms?*
A: Loopback kernel-internal (RAM kopyalama, ~µs); 8.8.8.8 fiziksel
internet (CPU işleme + Ethernet + ISP routing + Google PoP). Tipik
internet RTT: ülke içi 5-30 ms, kıtalar arası 100-300 ms.

---

## Referanslar

- **RFC 792** — ICMP Echo and Echo Reply
- **`man 2 gettimeofday`** — POSIX timer
- **`man 2 clock_gettime`** — Modern alternatif
- **`man 3 sqrt`** — math.h fonksiyonu
- inetutils `ping/ping_common.c` — `tvsub`, `tvadd` implementasyonları
- Welford, B.P. (1962) — *Note on a Method for Calculating Corrected
  Sums of Squares and Products* (online variance algoritması)
