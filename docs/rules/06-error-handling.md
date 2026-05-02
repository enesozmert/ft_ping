# 06 — Error Handling Rules

> Subject (R-01): "Hata yönetimini dikkatli yapmalısın. Programın hiçbir
> şekilde beklenmedik biçimde sonlanmasına izin verilmez (segfault, bus
> error, double free, vb.)."

---

## EH-01: Sistem Çağrıları (System Calls)

Tüm system call dönüşleri **kontrol edilmelidir**. ft_ping için kritik
olanlar:

| Çağrı                | Hata değeri        | Aksiyon                                |
|----------------------|--------------------|----------------------------------------|
| `socket()`           | `-1`               | `errno` → `perror`, exit(1)            |
| `setsockopt()`       | `-1`               | `errno` → `perror`, kaynak temizle     |
| `bind()`             | `-1`               | `errno` → `perror`, socket close       |
| `sendto()`           | `-1`               | log, retry veya stat++ (paket kaybı)   |
| `recvfrom()`         | `-1`               | `EINTR` → retry, diğer → log + stat++  |
| `select()`           | `-1`               | `EINTR` → retry, diğer → fatal         |
| `gettimeofday()`     | `-1`               | RTT hesabı atla, log                   |
| `getaddrinfo()`      | `≠ 0`              | `gai_strerror`, exit(1)                |
| `malloc()` / `calloc`| `NULL`             | error mesajı, kaynak temizle, exit(1)  |

---

## EH-02: Memory Safety

- `malloc` ardından **mutlaka NULL kontrolü**.
- `free` sonrası pointer **NULL’a setlenmelidir** (double-free koruması).
- Allocate edilen her bellek için **owner** belirlenmelidir; ownership
  belirsizse free edilmemelidir.
- `realloc` dönüşü **geçici değişkende** tutulmalıdır:
  ```c
  void *tmp = realloc(buf, new_size);
  if (!tmp) { free(buf); return -1; }
  buf = tmp;
  ```

---

## EH-03: Buffer Bounds

- Tüm buffer erişimleri **bounds check**’li olmalıdır.
- `memcpy` / `memmove` / `strncpy` boyut argümanı **destination
  capacity**’i aşmamalıdır.
- `recvfrom` dönüş değeri (alınan byte sayısı) **kontrol edilmeli**;
  beklenen header boyutundan kısaysa packet **discard** edilmelidir.

---

## EH-04: Signal Handling

- `SIGINT` (Ctrl+C) yakalanmalı; statistics flush edilip programdan
  çıkılmalıdır.
- `SIGALRM` ile periyodik gönderim yönetiliyorsa, handler **async-signal-safe**
  olmalı (sadece `volatile sig_atomic_t` flag set).
- Handler içinde `printf` çağrısı **technically unsafe**’tir; flag set
  edilip ana döngüde flush yapılmalıdır.

---

## EH-05: Pointer Discipline

- Fonksiyon parametresi olarak gelen pointer’lar başında **NULL kontrolü**
  yapılmalıdır.
- Dereference’tan **önce** validity kontrolü zorunludur.
- `void *` cast’leri **alignment**’a saygı duymalıdır.

---

## EH-06: Network Specific

| Senaryo                          | Davranış                                       |
|----------------------------------|------------------------------------------------|
| DNS resolution başarısız         | "unknown host" mesajı, exit(2)                 |
| Hostname IPv6’a resolve oldu     | Subject IPv4 only → hata mesajı                |
| ICMP echo reply gelmedi (timeout)| stat++, sonraki paket gönder                   |
| ICMP type ≠ 0 (echo reply)       | type/code log + (gerekirse) verbose ile göster |
| Checksum mismatch                | packet discard + counter++                     |
| Sequence number mismatch         | "out of order" log                             |
| Permission denied (EPERM)        | "ft_ping: ... Operation not permitted" + exit  |

---

## EH-07: Resource Cleanup

- `atexit()` ile global cleanup handler register edilebilir.
- Tüm açık file descriptor’lar exit öncesinde **kapatılmalıdır**.
- Allocate edilmiş tüm bellek free edilmelidir (Valgrind --leak-check=full
  sıfır rapor vermelidir).

---

## EH-08: Defansif Programlama Sınırı

> ⚠️ MISRA ile çelişebilir: çağrılamayacak yerlerde defansif kontrol
> eklemek **dead code** üretir.
>
> **Kural:** Yalnızca **system boundary** noktalarında (user input,
> network packet, syscall return) defansif olun. **Internal** call
> path’lerde invariant’a güvenin.

---

## EH-09: Exit Codes (BSD `ping` ile uyumlu)

| Exit Code | Anlam                                         |
|-----------|-----------------------------------------------|
| `0`       | En az 1 echo reply alındı.                    |
| `1`       | Hiç reply alınmadı (tüm paketler düştü).      |
| `2`       | Hata (DNS, socket, parametre, ...).           |

---

## EH-10: Error Mesaj Formatı

`inetutils-2.0` ile uyumlu olmak için:

```
ft_ping: <kısa hata mesajı>: <strerror(errno)>
```

Örnek:
```
ft_ping: socket: Operation not permitted
ft_ping: unknown host
ft_ping: invalid argument: -X
```

`getopt`/`argv` parsing hatalarında **usage** çıktısı verilmeli ve
exit(2) yapılmalıdır.
