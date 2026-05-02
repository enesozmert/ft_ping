# Q4 — Niçin `getaddrinfo`, `gethostbyname` Değil?

> *"DNS lookup için iki standart fonksiyon var: eski `gethostbyname` ve
> yeni `getaddrinfo`. ft_ping niçin yeni olanı seçti?"*

---

## Kısa Cevap

`gethostbyname(3)` **POSIX-2001'de deprecated** ilan edildi:

- **Sadece IPv4** döner; IPv6 için ayrı `gethostbyname2` gerekir.
- **Static buffer** kullanır → **non-reentrant**, multi-thread'de
  yarış (race condition).
- **`h_errno`** global hata değişkeni yine thread-unsafe.
- Service (port) çözümlemesi yapmaz; ayrıca `getservbyname` çağırmak
  gerekir.

`getaddrinfo(3)` POSIX-2001'de yeni standart:

- IPv4 + IPv6 + future address family'ler.
- **Reentrant** (kendi memory allocate eder, `freeaddrinfo` ile temizler).
- Hata kodu **return value** (`h_errno` yerine).
- Service+host birlikte çözer; getopt-style hint struct'ı ile filtre.
- DNS, `/etc/hosts`, NIS, mDNS gibi kaynakları nsswitch.conf üzerinden
  zincirler.

ft_ping kodu: [`src/resolve_hostname.c`](../../../src/resolve_hostname.c)

---

## Tarihsel Bağlam

```
1983  4.2BSD  → gethostbyname()       sadece IPv4, static buffer
1995  RFC 2553 → getaddrinfo()        IPv6-aware, reentrant (POSIX-2001)
2001  POSIX   → gethostbyname() obsolete, getaddrinfo() recommended
2008  POSIX-2008 → gethostbyname() removed (man page'de "may be removed")
```

ft_ping subject Linux Debian (>= 7.0) hedefi; her iki API mevcut ama
**modern best practice** `getaddrinfo`.

---

## API Karşılaştırma

### `gethostbyname` (eski)

```c
struct hostent *gethostbyname(const char *name);

struct hostent {
    char  *h_name;          // canonical name
    char **h_aliases;       // alternative names
    int    h_addrtype;      // AF_INET (sadece)
    int    h_length;        // 4 (IPv4)
    char **h_addr_list;     // IP'ler (in network byte order)
};
```

Kullanım:

```c
struct hostent *he = gethostbyname("google.com");
if (he == NULL) {
    fprintf(stderr, "Lookup failed: %s\n",
        hstrerror(h_errno));     // <- thread-unsafe
    return -1;
}
struct in_addr *addr = (struct in_addr *)he->h_addr_list[0];
char *ip_str = inet_ntoa(*addr); // <- ALSO thread-unsafe (static buffer)
```

**Sorunlar:**
- `gethostbyname` **statik bellek** kullanır. İki thread aynı anda
  çağırırsa biri diğerinin sonucunu üzerine yazar.
- `h_errno` thread-local değil (eski POSIX'te). Glibc'de TLS yapılmış
  ama portable değil.
- IPv6 için ayrı `gethostbyname2(name, AF_INET6)` lazım.
- `inet_ntoa` da statik buffer kullanır.

### `getaddrinfo` (yeni)

```c
int getaddrinfo(const char *node, const char *service,
                const struct addrinfo *hints,
                struct addrinfo **res);

struct addrinfo {
    int              ai_flags;       // AI_PASSIVE, AI_CANONNAME, ...
    int              ai_family;      // AF_INET / AF_INET6 / AF_UNSPEC
    int              ai_socktype;    // SOCK_STREAM / SOCK_DGRAM / SOCK_RAW
    int              ai_protocol;    // IPPROTO_*
    socklen_t        ai_addrlen;
    struct sockaddr *ai_addr;        // sockaddr_in veya sockaddr_in6
    char            *ai_canonname;
    struct addrinfo *ai_next;        // linked list
};
```

ft_ping kullanımı:

```c
int	resolve_hostname(const char *hostname, char *ip_str, size_t ip_str_len)
{
    struct addrinfo		hints;
    struct addrinfo		*res;
    struct sockaddr_in	*addr;
    int					result;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;       // IPv4 zorla (subject)
    hints.ai_socktype = SOCK_RAW;    // ICMP raw socket için

    result = getaddrinfo(hostname, NULL, &hints, &res);
    if (result != 0)
    {
        fprintf(stderr, "ping: %s: %s\n", hostname, gai_strerror(result));
        return (1);
    }
    addr = (struct sockaddr_in *)res->ai_addr;
    if (inet_ntop(AF_INET, &addr->sin_addr, ip_str, ip_str_len) == NULL)
    {
        perror("inet_ntop");
        freeaddrinfo(res);
        return (1);
    }
    freeaddrinfo(res);
    return (0);
}
```

**Faydalar:**

| Özellik                  | gethostbyname | getaddrinfo |
|--------------------------|---------------|-------------|
| IPv4 + IPv6              | ❌ (sadece v4)| ✅          |
| Reentrant / thread-safe  | ❌            | ✅          |
| Service (port) lookup    | ❌            | ✅          |
| Family-agnostic          | ❌            | ✅          |
| Hata mesajı              | `hstrerror`   | `gai_strerror` (thread-safe) |
| Memory model             | static        | malloc/free |
| Address family filter    | ❌            | `hints.ai_family` |
| Sockaddr ready           | ❌ manuel     | ✅ `ai_addr` |

---

## `gai_strerror` vs `hstrerror`

`getaddrinfo` dönüş kodları:

```c
#define EAI_BADFLAGS   -1
#define EAI_NONAME     -2  // host bulunamadı
#define EAI_AGAIN      -3  // geçici DNS hatası
#define EAI_FAIL       -4  // kalıcı DNS hatası
#define EAI_FAMILY     -6  // family desteklenmiyor
#define EAI_MEMORY    -10  // out of memory
#define EAI_SYSTEM    -11  // sistem hatası (errno ek bilgi)
...
```

`gai_strerror(errcode)` thread-safe (genellikle static const string'lerin
adresi). `hstrerror(h_errno)` glibc'de TLS, BSD'de değil.

---

## `inet_ntop` vs `inet_ntoa`

ft_ping reply IP'sini **string'e** çevirmek için `inet_ntop` kullanır:

```c
inet_ntop(AF_INET, &addr->sin_addr, ip_str, ip_str_len);
```

| Fonksiyon    | Reentrant | IPv6 | Buffer kontrolü |
|--------------|-----------|------|-----------------|
| `inet_ntoa`  | ❌ static | ❌   | sınırsız static |
| `inet_ntop`  | ✅        | ✅   | kullanıcı buffer + size |

ft_ping `INET_ADDRSTRLEN` (16 byte) buffer veriyor.

---

## DNS Resolution Sırası

`getaddrinfo` `/etc/nsswitch.conf` zincirini takip eder. Tipik
sıralama:

```
hosts: files dns
```

1. **`/etc/hosts`** — local override (örn. `127.0.0.1 localhost`).
2. **DNS** — `/etc/resolv.conf` içindeki nameserver'lar (53/UDP).
3. (Konfigürasyona bağlı) NIS, mDNS, LDAP.

ft_ping bu zincirin tepesinde oturur — kullanıcı `google.com` yazınca
DNS lookup yapılır. Loopback (`127.0.0.1`) için `inet_pton` zaten valid
olduğu için DNS atlanır (`getaddrinfo` numerik IP'yi tanır).

---

## Subject Bağlamı

Subject Chapter IV:
> "You will have to manage FQDN without doing the DNS resolution in the
> packet return."

İki gereklilik:
1. **Başlangıçta** hostname → IP çözümlemesi yapılmalı (`getaddrinfo`).
2. Her **paket dönüşünde** reverse DNS lookup yapılmamalı (RTT'yi
   bozar, network trafiği oluşturur).

ft_ping `getaddrinfo` ile bunu **bir kez** yapar; banner'da `(1.1.1.1)`
formatıyla zaten IP'yi gösterir, hostname'i lookup'sız tutar:

```
PING 1.1.1.1 (1.1.1.1) 56(84) bytes of data.
```

---

## Defense'ta Olası Follow-up'lar

**Q:** *Niçin `hints.ai_socktype = SOCK_RAW` veriyorsun?*
A: `getaddrinfo` aynı IP için birden fazla `addrinfo` döndürebilir
(her socktype için bir). `SOCK_RAW` filtresi sadece bizim ihtiyacımız
olan tek entry'i verir, gereksiz iterasyon olmaz. `AF_INET` zaten IPv6
sonuçlarını ele eler.

**Q:** *`AI_NUMERICHOST` flag'i kullanmıyorsun, niye?*
A: Bu flag DNS bypass eder, sadece numerik IP kabul eder. ft_ping hem
hostname (`google.com`) hem IP (`8.8.8.8`) kabul ediyor — `AI_NUMERICHOST`
açılırsa hostname reddedilir.

**Q:** *Linked list'in ilkini alıyorsun (`res->ai_addr`); ya birden
fazla A record döndüyse?*
A: ft_ping ilk record'u kullanır (genelde en uygun olanı, glibc
`getaddrinfo` RFC 6724 önceliği uygular). `inetutils ping` aynı
yaklaşım. Bonus olarak round-robin veya ICMP probing eklenebilir.

**Q:** *DNS lookup başarısız olursa exit code nedir?*
A: `getaddrinfo` `EAI_NONAME` döner → ft_ping `fprintf(stderr, "ping:
%s: %s\n", hostname, gai_strerror(result));` yazar ve `return 1`.
inetutils ping aynı format ve exit code 2 verir; bizim 1 — minor fark.

**Q:** *`gethostbyname_r` (reentrant variant) niye değil?*
A: `_r` variant glibc-specific, POSIX değil. `getaddrinfo` POSIX
standart ve modern; `_r` history'sinden kaçınmak temiz çözüm.

---

## Referanslar

- **POSIX-2001 Issue 6** — `getaddrinfo` standardizasyonu
- **RFC 2553** — Basic Socket Interface Extensions for IPv6
- **RFC 3493** — Basic Socket Interface (revize)
- **RFC 6724** — Default Address Selection for IPv6
- **`man 3 getaddrinfo`**, **`man 3 gai_strerror`**
- **`man 5 nsswitch.conf`** — Name Service Switch zinciri
- **`man 5 resolv.conf`** — DNS resolver konfigürasyonu
- **`gethostbyname` deprecation:**
  https://pubs.opengroup.org/onlinepubs/9699919799/functions/gethostbyname.html
