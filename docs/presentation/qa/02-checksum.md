# Q2 — Checksum Nasıl Hesaplanıyor? (RFC 1071)

> *"ICMP paketinin checksum field'ını nasıl dolduruyorsun? Neden 1's
> complement sum?"*

---

## Kısa Cevap

**RFC 1071**: paketin **16-bit word**'leri network byte order'da
**1's complement (one's complement) sum** ile toplanır; carry'ler
geri eklenir; sonucun **bit-wise NOT**'u alınır. Tek byte kalırsa
sıfır-padded high byte olarak işlem görür. Hem checksum hesaplanırken
hem doğrulanırken aynı algoritma çalışır.

ft_ping kodu: [`src/checksum.c`](../../../src/checksum.c)

```c
unsigned short	checksum(void *buffer, int length)
{
	unsigned short	*data;
	unsigned long	sum;

	data = buffer;
	sum = 0;
	while (length > 1)
	{
		sum += *data++;
		length -= 2;
	}
	if (length == 1)
		sum += *(unsigned char *)data;
	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);
	return ((unsigned short)(~sum));
}
```

---

## 1's Complement Sum Nedir?

İki türlü tamsayı toplama vardır:

| Aritmetik    | Davranış                                       |
|--------------|------------------------------------------------|
| **2's complement** (modern CPU) | overflow → en yüksek bit kaybolur |
| **1's complement** (RFC 1071)   | overflow → carry **geri eklenir** (end-around carry) |

Örnek (4-bit gösterim):

```
  1100 + 0101 = 10001  (5-bit, taşma)

2's complement:
  → 0001  (en yüksek bit atılır)

1's complement:
  → 0001 + 1 = 0010  (carry başa eklenir)
```

Bu özellik checksum'ı **byte-order bağımsız** yapar (host endianness'ten
etkilenmez) ve **idempotent** olur (paket içinde checksum=0 yazıp tüm
paketi sum yapınca, gelen değerin bit-wise NOT'u checksum'dır).

---

## Adım Adım Algoritma

```
Input  : byte dizisi (header + payload), uzunluk L
Output : 16-bit checksum

1. sum = 0 (32+ bit accumulator)
2. while L >= 2:
       sum += *(uint16_t *)data
       data += 2; L -= 2
3. if L == 1:           # tek byte kaldıysa
       sum += data[0]   # zero-padded high byte
4. # carry'leri geri ekle
   while sum >> 16:
       sum = (sum & 0xFFFF) + (sum >> 16)
5. return ~sum (16-bit)
```

ft_ping'de step 4 şu satırlarla:

```c
sum = (sum >> 16) + (sum & 0xFFFF);  /* ilk fold */
sum += (sum >> 16);                  /* fold sonrası taşma için ikinci */
```

> **Niçin iki fold?** İlk fold sonucu da 17 bit olabilir (ör. `0xFFFF +
> 0x0001 = 0x10000`). İkinci `sum += (sum >> 16)` bu artık carry'i de
> geri ekler. Üçüncü fold gereksizdir.

---

## ICMP Header'da Checksum

```c
typedef struct s_icmphdr
{
    uint8_t   type;
    uint8_t   code;
    uint16_t  checksum;   ← burası
    uint16_t  id;
    uint16_t  sequence;
}   t_icmphdr;
```

**Hesap sırası**:

1. `checksum = 0` set et.
2. `[icmp_header + payload]` üzerinde `checksum()` çağır.
3. Dönen değeri `checksum` field'ına yaz (zaten network byte order'da
   kalır — operasyon byte-order bağımsız).

ft_ping `ping_loop`'ta her paket için yeniden hesaplar (sequence
artar, checksum değişir):

[`src/ping_loop.c`](../../../src/ping_loop.c) — `refresh_icmp_header`:

```c
static void	refresh_icmp_header(t_ping *ping)
{
    ping->icmp_header->type = ICMP_ECHO;
    ping->icmp_header->code = 0;
    ping->icmp_header->id = htons(getpid() & 0xFFFF);
    ping->icmp_header->sequence = htons(ping->seq);
    ping->icmp_header->checksum = 0;                      ← önce sıfırla
    ping->icmp_header->checksum = checksum(ping->icmp_header,
            sizeof(t_icmphdr) + ping->payload->payload_size);
}
```

---

## Doğrulama (Receiver Tarafı)

Alıcı **aynı algoritmayı** uygular: `[icmp_header + payload]` üzerinde
sum hesaplanır. Header'daki `checksum` zaten gerçek değerin
**bit-wise NOT**'u olduğu için sum **`0xFFFF`** verir → `~0xFFFF = 0`
veya direkt: hesaplanan checksum **0** çıkmalıdır.

> ft_ping receiver tarafında **explicit checksum doğrulaması yapmıyor**
> — kernel zaten ICMP echo reply için bunu doğrular. Manuel doğrulama
> bonus olabilir.

---

## TCP/UDP Pseudo-header

TCP/UDP checksum'ı **pseudo-header** içerir (src IP, dst IP, protokol,
length). ICMP **pseudo-header kullanmaz** — sadece ICMP message üzerinde
hesap yapılır. Bu ICMP'yi daha basit kılar.

| Protokol | Pseudo-header | Checksum kapsamı                     |
|----------|---------------|---------------------------------------|
| ICMP     | ❌            | ICMP header + payload                 |
| UDP      | ✅            | UDP pseudo-header + UDP header + data |
| TCP      | ✅            | TCP pseudo-header + TCP header + data |

---

## MISRA & Norm Notları

**Type aliasing safety:** `unsigned short *data = buffer;` — `void *`
cast `t_icmphdr *` benzeri yapıların olduğu durumda **alignment**
problemi yaratabilir. Pratikte `t_icmphdr` 8-byte aligned, malloc
16-byte aligned döndürdüğü için sorun olmaz.

**Endianness:** 1's complement sum byte-order **bağımsızdır**. Big-
veya little-endian aynı sonucu verir; bu yüzden `htons` gereksizdir
(checksum field'ı zaten doğru şekilde ekleniyor).

**Cppcheck strict mode:** `-Wcast-align` uyarısı verirse `(uint16_t *)`
yerine `memcpy(&word, data, 2)` ile kopyala — ft_ping default flag'lerle
uyarısız çalışır.

---

## Defense'ta Olası Follow-up'lar

**Q:** *Çift fold gerçekten gerekli mi?*
A: Evet — 32-bit accumulator 65535'e kadar tek byte taşma içerebilir;
ilk fold (sum = (sum>>16) + (sum&0xFFFF)) sonucu 17 bit olabilir; ikinci
`sum += (sum>>16)` bunu da temizler. Edge case kanıtı:
`0xFFFF * N` accumulator'a basınca.

**Q:** *Tek byte kaldığında niçin yüksek byte 0'lı?*
A: RFC 1071: "If the total length is odd, the received data is padded
with one octet of zeros for computing the checksum." Yani tek byte
"low byte"; high byte 0 olarak ele alınır. ft_ping'de:
```c
if (length == 1)
    sum += *(unsigned char *)data;
```
`unsigned char` cast'i sayesinde sadece düşük byte eklenir.

**Q:** *Checksum field'ı ile diğer field'lar arasında order bağımlılığı?*
A: Yok. Toplama operasyonu **commutative ve associative** olduğu için
sıra önemsiz. Tek koşul: hesap sırasında `checksum` field'ı **0**
olmalı (kendisini saymamalı).

**Q:** *Kötü niyetli paket'in checksum'u doğru olabilir mi?*
A: Evet — checksum **error detection** için, **integrity** için değil.
Sadece *kazara* bozulmaları yakalar. Cryptographic integrity için
HMAC veya digital signature gerekir (ICMP scope dışı).

---

## Referanslar

- **RFC 1071** — Computing the Internet Checksum (1988)
- **RFC 1141** — Incremental Updating of the Internet Checksum
- **RFC 1624** — Computation of the Internet Checksum via Incremental Update
- **RFC 792** — ICMP (`Checksum` field tanımı)
- **`man 3 inet_checksum`** (BSD) — sistem helper
- inetutils `ping/ping_common.c` — `in_cksum` referans implementasyonu
