# Q1 — Raw Socket Nedir, Niçin Gerekiyor?

> *"ICMP echo request göndermek için neden `SOCK_STREAM` (TCP) veya
> `SOCK_DGRAM` (UDP) kullanmıyorsun?"*

---

## Kısa Cevap

ICMP **transport layer**'da değil, **network layer**'a (Layer 3)
oturur. Kernel'ın varsayılan TCP/UDP socket'ları bu protokole erişim
vermez. **`SOCK_RAW` + `IPPROTO_ICMP`** ile kullanıcı uzayı doğrudan
ICMP paketi yazar/okur — `ping`'in tek yolu budur.

```c
int sock_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
```

ft_ping kodu: [`src/create_raw_socket.c`](../../../src/create_raw_socket.c)

---

## Network Stack'te ICMP'nin Yeri

```
Application Layer  (HTTP, SSH, DNS)
Transport  Layer  (TCP, UDP)         ← SOCK_STREAM, SOCK_DGRAM burada
Network    Layer  (IPv4, IPv6, ICMP) ← SOCK_RAW burada
Link       Layer  (Ethernet, Wi-Fi)  ← AF_PACKET (Linux), BPF (BSD)
```

**ICMP** (Internet Control Message Protocol — RFC 792) IP'nin yanında
çalışır; TCP/UDP gibi port kavramı yoktur, **type/code** alanları ile
mesaj kategorisi belirtilir (Echo, Destination Unreachable, Time
Exceeded, Redirect, vb.).

| Socket Type   | Layer        | Kernel Davranışı                          |
|---------------|--------------|--------------------------------------------|
| `SOCK_STREAM` | Transport    | TCP segment kontrolü                       |
| `SOCK_DGRAM`  | Transport    | UDP datagram                               |
| `SOCK_RAW`    | Network      | Tüm protokol header'ları kullanıcıya teslim|
| `SOCK_PACKET` | Link (Linux) | Ethernet frame seviyesi (deprecated)       |

---

## `SOCK_RAW` ile Ne Olur?

### Outgoing (sendto)

Kernel:
1. **IP header**'ı **otomatik ekler** (kullanıcı uzayında oluşturulmaz).
2. Routing table'dan çıkış arayüzünü seçer.
3. Source IP'yi belirler (`SO_BINDTODEVICE` veya routing).
4. ARP/NDP ile next-hop MAC adresini bulur (Layer 2).
5. Frame'i Ethernet üzerinde yollar.

Kullanıcı sadece **ICMP header + payload** verir:

```c
sendto(sock_fd, packet /* icmp_hdr + payload */, len, 0,
       (struct sockaddr *)&dest_addr, sizeof(dest_addr));
```

**`IP_HDRINCL`** socket option açılırsa kernel IP header eklemez —
kullanıcı `IP + ICMP + payload`'u tam yazar. ft_ping bunu **kullanmıyor**
(gereksiz karmaşıklık, kernel zaten doğru header üretir).

### Incoming (recvfrom)

Kernel `SOCK_RAW` üzerinden gelen ICMP paketini **IP header dahil**
teslim eder:

```
[ IP header (20 byte) | ICMP header (8 byte) | payload ]
```

ft_ping `recvfrom` çağrısı sonrası IP header'ı parse eder, `ihl` alanı
(ip header length) ile ICMP header offset'ini hesaplar:

```c
const t_iphdr   *ip_hdr   = (const t_iphdr *)recv_buf;
const t_icmphdr *icmp_hdr = (const t_icmphdr *)
    (recv_buf + (ip_hdr->vhl & 0x0F) * 4);
```

(`vhl` = `version<<4 | ihl`; düşük 4 bit ihl, 32-bit word cinsinden
header uzunluğu.)

Kod: [`src/create_socket_recvfrom.c`](../../../src/create_socket_recvfrom.c)

---

## Niçin Privileged?

`SOCK_RAW` herhangi bir IP protokolünü dinleyebilir/spoof edebilir —
güvenlik riski. Linux'ta iki ayrıcalık modu var:

### 1. `CAP_NET_RAW` Capability (önerilen)

```bash
sudo setcap cap_net_raw+ep ./output/ft_ping
./output/ft_ping google.com   # sudo'suz çalışır
```

Sadece bu binary için raw socket izni verir; sistem genelinde root
gerekmez. **Linux kernel ≥ 2.6** desteği.

### 2. `setuid root` veya `sudo`

```bash
sudo ./output/ft_ping google.com
```

Tüm program root yetkisiyle çalışır — daha geniş etki alanı.

### 3. Linux Alternatifi: `IPPROTO_ICMP` Datagram Socket

```c
socket(AF_INET, SOCK_DGRAM, IPPROTO_ICMP);
```

Linux 3.0+ `net.ipv4.ping_group_range` sysctl ile listed group'lar bu
socket'i ayrıcalıksız açabilir. **Kernel ICMP id field'ını kontrol
eder** (yarı-raw, sadece echo). Bizim Docker compose'da:

```yaml
sysctls:
  - net.ipv4.ping_group_range=0 2147483647
```

Bu setting tüm UID'lere izin verir. Gerçek production'da `0 0`
(sadece root) veya `1000 1010` (specific group) tercih edilir.

---

## ft_ping'de Capability Hatası

Capability/sudo yokken:

```
$ ./ft_ping 8.8.8.8
ft_ping: socket: Operation not permitted
```

Kernel `EPERM` döner; ft_ping bunu yakalayıp `ft_ping: socket: ...`
mesajıyla exit(1) yapar. inetutils ping ile birebir uyumlu.

---

## Subject Bağlamı

Subject Chapter IV (Mandatory Part):
> "You will take as reference the ping implementation from inetutils-2.0."

`inetutils-2.0` **`SOCK_RAW`** kullanır (`ping/ping.c` içinde
`socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)` çağrısı). Aynı semantik için
ft_ping da SOCK_RAW kullanır. Datagram alternatifi (kernel-validated
ICMP) bonus seviyesinde önerilebilir ama mandatory için raw daha uygun.

---

## Defense'ta Olası Follow-up'lar

**Q:** *Niçin `IP_HDRINCL` kullanmadın?*
A: Kernel routing/ARP/source-IP seçimini yapıyor; manuel IP header
yazmak gereksiz. `IP_HDRINCL` sadece IP option'ları test ederken veya
özel header field'larını manipüle ederken anlamlıdır.

**Q:** *Niçin `PF_PACKET` (Layer 2 raw) ile yapmadın?*
A: Yapmıştım — ilk versiyonda `PF_PACKET + ETH_P_ALL` kullandım, manuel
ARP + Ethernet frame oluşturuyordum. Ama bu Linux-only (BSD'de yok),
container'larda ARP query başarısız oluyordu, ve gereksiz karmaşıktı.
Layer 3 refactor sonrası `AF_INET + IPPROTO_ICMP` ile temizledim.
Detay: [`refactor: switch to Layer 3...`](../../../docs/tasks/TASKS.md#layer-3-refactor-notu-2026-05-02)

**Q:** *`SOCK_RAW` IPv6 için nasıl?*
A: `socket(AF_INET6, SOCK_RAW, IPPROTO_ICMPV6)`; ICMPv6 farklı bir
protocol number (58) ve farklı message type'larıdır (RFC 4443). Subject
sadece IPv4 zorunlu kıldığı için ft_ping IPv4-only.

**Q:** *Raw socket okurken neden başka süreçlerin paketleri de geliyor?*
A: Kernel filter olmadan tüm `IPPROTO_ICMP` paketleri match eder. ft_ping
ICMP header'ın `id` field'ında `getpid() & 0xFFFF` kullanır:

```c
if (icmp_hdr->type == ICMP_ECHOREPLY
    && ntohs(icmp_hdr->id) == (getpid() & 0xFFFF))
    process_echo_reply(...);
```

Aksi halde başka `ping` instance'ının reply'ini sayar.

---

## Referanslar

- **RFC 792** — Internet Control Message Protocol
- **RFC 1812** — Requirements for IP Version 4 Routers
- **`man 7 raw`** — Linux raw socket semantics
- **`man 7 ip`** — IPv4 socket option reference
- **`socket(2)`**, **`setsockopt(2)`**, **`sendto(2)`**, **`recvfrom(2)`**
- inetutils ping kaynağı: https://www.gnu.org/software/inetutils/
- Linux capabilities: **`man 7 capabilities`**
