# 07 — C Best Practices

> Subject veya MISRA dışında, ft_ping kod tabanında uyulması beklenen
> genel C iyi uygulamaları.

---

## CB-01: Header Discipline

- Her `.c` dosyası kendi `.h` dosyasına sahip olmalı **veya** projenin
  merkezi header’ına dahil olmalıdır.
- `.h` dosyalarında **yalnızca**:
  - `#include` directive’leri
  - macro tanımları
  - typedef, struct, union, enum tanımları
  - extern global declaration’ları
  - function prototype’ları
  bulunur. **Implementation yok.**
- **Forward declaration** mümkünse tercih edilmeli; gereksiz
  `#include` eklemekten kaçınılmalıdır.

---

## CB-02: Include Ordering

Genel kabul gören sıralama:

```c
/* 1. Project header (kendi header'ı) */
#include "ft_ping.h"

/* 2. Diğer project headerları */
#include "network.h"

/* 3. POSIX / system headerları */
#include <sys/socket.h>
#include <netinet/ip_icmp.h>

/* 4. Standart C library */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
```

---

## CB-03: Naming

- Function isimleri **`snake_case`**, **fiil** veya **fiil_object**:
  `parse_args`, `create_icmp_header`, `calculate_rtt`.
- Struct isimleri `s_` prefix; typedef `t_` prefix (Norminette).
- Macro isimleri **UPPER_SNAKE_CASE**.
- Boolean dönüş değerli fonksiyonlar `is_*`, `has_*`, `can_*` ile
  başlamalı.

---

## CB-04: Function Size & Cohesion

- Tek fonksiyon = tek sorumluluk (Single Responsibility).
- Norminette: **25 satır** üst sınır (parantezler hariç).
- Bir fonksiyon **iki ayrı işi** yapıyorsa, böl.
- Cyclomatic complexity hedefi: **≤ 10**.

---

## CB-05: Constants & Magic Numbers

- Magic number yasaktır. Yerine `#define` veya `enum` kullanın:
  ```c
  #define ICMP_HEADER_SIZE 8
  #define DEFAULT_PAYLOAD_SIZE 56
  #define DEFAULT_TTL 64
  ```
- Network constant’lar `<netinet/...>` header’larından gelmeli; yeniden
  tanımlanmamalıdır.

---

## CB-06: Const Correctness

- Read-only parametreler **`const`** olmalıdır:
  ```c
  uint16_t calculate_checksum(const void *data, size_t len);
  ```
- String literal’ler `const char *` ile saklanmalıdır.
- Output parametre yoksa, struct pointer parametreleri `const`-qualify
  edilmelidir.

---

## CB-07: Endianness & Network Byte Order

- Network protocol field’ları **network byte order**’dadır (big-endian).
- Conversion için **mutlaka**:
  - `htons` / `ntohs` (16-bit)
  - `htonl` / `ntohl` (32-bit)
- Manuel byte shift ile dönüşüm **yasak** (portability ihlali).

---

## CB-08: Bit Manipulation

- IP header’ında `version` ve `ihl` gibi **bit-field**’lar `union`
  veya **explicit shift+mask** ile çözülmelidir.
- Padding ve alignment’a dikkat — `__attribute__((packed))` GCC
  extension’ıdır; MISRA 1.2 ihlalidir.

---

## CB-09: Static Linkage

- Yalnızca tek dosyada kullanılan fonksiyonlar `static` olmalıdır.
- Translation unit’a özel global’ler `static` olmalıdır (MISRA 8.7).

---

## CB-10: Initialization

- Lokal değişkenler tanım anında initialize edilmelidir (mümkünse).
- Struct initialization **designated initializer** ile:
  ```c
  t_icmp_header hdr = {
      .type = ICMP_ECHO,
      .code = 0,
      .checksum = 0,
      .un.echo.id = htons(getpid()),
      .un.echo.sequence = htons(0)
  };
  ```

---

## CB-11: String Handling

- `strcpy`, `strcat`, `sprintf`, `gets` **yasaktır** (buffer overflow).
- Kullanılması gerekenler:
  - `strncpy` (terminator’a dikkat — manual `\0`)
  - `snprintf`
  - `strlcpy` / `strlcat` (BSD)
- Length kontrolü her zaman **destination** boyutuna göre yapılır.

---

## CB-12: Logging & I/O

- Hata mesajları **`stderr`**’e yazılmalıdır.
- Normal çıktı **`stdout`**’a yazılmalıdır.
- Karışık format `inetutils-2.0` ile **birebir** olmalı (subject R-04).

---

## CB-13: Comments

- Yorum yazılırken **niye**’ye odaklanın, **ne**’ye değil.
- Norminette: yorum fonksiyon **dışında** olmalı.
- `// TODO`, `// FIXME`, `// HACK` etiketleri kullanışlıdır; commit
  öncesi **temizlenmeleri** beklenir.

---

## CB-14: Compiler Warnings

- `-Wall -Wextra -Werror` minimum.
- Aşağıdaki ek flag’ler önerilir:
  - `-Wpedantic`
  - `-Wshadow`
  - `-Wconversion`
  - `-Wcast-align`
  - `-Wstrict-prototypes`
  - `-Wmissing-prototypes`
- Warning **suppress** edilirken `#pragma GCC diagnostic` ile **scoped**
  kullanılmalı; `-Wno-*` proje genelinde uygulanmamalıdır.

---

## CB-15: Reentrancy / Thread Safety (ft_ping bağlamında)

ft_ping single-threaded’dır; ancak signal handler’lar nedeniyle
**reentrancy** önemlidir:

- Signal handler içinde **yalnızca** async-signal-safe fonksiyonlar.
- Global state, handler ile main path arasında **`volatile sig_atomic_t`**
  ile paylaşılmalıdır.
- `errno` handler içinde kullanılacaksa **save/restore** edilmelidir:
  ```c
  void handler(int sig) {
      int saved_errno = errno;
      /* ... */
      errno = saved_errno;
  }
  ```
