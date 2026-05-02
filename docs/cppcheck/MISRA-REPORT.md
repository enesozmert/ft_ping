# MISRA C:2012 — ft_ping Analiz Raporu

> Tarih: 2026-05-02
> Araç: `cppcheck --addon=misra --std=c11 -I hdr src/`
> Toplam ihlal: **200**
> Build sistemi: Docker container (Ubuntu 22.04 + cppcheck 2.x)
>
> Bu rapor, **mandatory rule** ve gerekçeli **deviation**'ları ayırır.
> Çalıştırma: `make misra` (bkz. [Makefile](../../Makefile)).

---

## 1. Özet

| Kategori                        | Adet  | Aksiyon                          |
|---------------------------------|-------|----------------------------------|
| **Advisory** (15.5, 15.6, 18.4) | 101   | Kayıt altında, düzeltme isteğe bağlı |
| **Required (deviation)**        | 84    | Aşağıda gerekçeli kabul          |
| **Required (manuel fix)**       | 15    | Sonraki turda düzeltilecek       |

Toplam: **200**.

---

## 2. Rule Bazlı Dağılım

| Kural                | Adet | Tip       | Durum                                   |
|----------------------|------|-----------|------------------------------------------|
| `misra-c2012-15.5`   | 55   | Advisory  | Single-return-point — kabul (early return tercih edildi) |
| `misra-c2012-15.6`   | 41   | Required  | Brace-on-controlled-statement — kısmen false positive (norminette ile uyumsuz) |
| `misra-c2012-17.7`   | 33   | Required  | Non-void return ignored — `printf`/`memcpy`/`signal` deviation |
| `misra-c2012-21.3`   | 16   | Required  | `malloc/free` — network buffer için **deviation** |
| `misra-c2012-8.7`    | 14   | Advisory  | `static` linkage — function-pointer table üzerinden çağrı (false positive) |
| `misra-c2012-10.4`   | 13   | Required  | Mixed essential type — network constant aritmetiği için kabul |
| `misra-c2012-21.8`   |  6   | Required  | `exit()` kullanımı — `parse_args` ve fatal error için **deviation** |
| `misra-c2012-11.3`   |  6   | Required  | Pointer cast — `sockaddr` ailesi için **deviation** |
| `misra-c2012-18.4`   |  5   | Advisory  | Pointer arithmetic — packet buffer offset için kabul |
| `misra-c2012-14.4`   |  3   | Required  | Boolean controlling expression — `if (ptr)` idiom |
| `misra-c2012-11.5`   |  3   | Advisory  | `void *` → object pointer cast — `alloc_one` helper |
| `misra-c2012-18.8`   |  2   | Required  | Variable-length array — false positive, `char buffer[1024]` |
| `misra-c2012-17.8`   |  1   | Advisory  | Function parameter modified — incremented loop var |
| `misra-c2012-12.1`   |  1   | Advisory  | Operator precedence — parentheses recommended |
| `misra-c2012-11.8`   |  1   | Required  | `const` removed by cast — `printf` `(char *)` cast |

---

## 3. Kabul Edilen Deviation'lar

### 3.1 Rule 21.3 — `malloc/free` Kullanımı

**Kural:** Dynamic memory allocation kullanılmamalı.

**Deviation gerekçesi:**
- Subject (Ecole 42 ft_ping v5.1) explicit olarak `libc`'yi izinli kılıyor.
- ICMP packet buffer boyutu CLI parametresine göre değişken olduğundan
  static allocation pratik değil.
- Tüm `malloc` çağrıları için `NULL` kontrolü ve cleanup chain mevcut
  (Valgrind ile **0 leak** doğrulandı).

**Kapsam:** `ping_create_allocates.c`, `create_packet.c`, `create_payload.c`,
`get_network_default_gateway.c`, `get_network_gateway_mac_address.c`,
`get_network_src_mac_addr.c`, `get_network_interface_name.c`,
`resolve_hostname.c` (`getaddrinfo` internal).

---

### 3.2 Rule 21.8 — `exit()` Kullanımı

**Kural:** `abort/exit/getenv/system` kullanılmamalı.

**Deviation gerekçesi:**
- `parse_args` `getopt` döngüsü içinden help (`-?`) veya invalid flag
  durumunda erken çıkış için kullanılır. Subject `inetutils-2.0`
  davranışını taklit eder.
- Alternatif: dönüş değeri ile main'e iletmek — kod akışı bozulur.

**Kapsam:** `parse_args.c`, `interrupt_handler.c`,
`run_ping_create_functions.c`, `main.c`.

---

### 3.3 Rule 11.3 — Pointer Cast (`sockaddr` ailesi)

**Kural:** Object pointer tipleri arasında cast yapılmamalı.

**Deviation gerekçesi:**
- POSIX socket API zorunluluğu: `sendto/recvfrom/bind` her zaman
  `(struct sockaddr *)` cast ile çağrılır. Bu cast network programlama
  için **zorunlu idiom**'dur.

**Kapsam:** `create_send_request.c`, `create_socket_recvfrom.c`,
`get_network_gateway_mac_address.c`.

---

### 3.4 Rule 17.7 — Non-void Return Değeri Kullanılmıyor

**Kural:** Non-void fonksiyonun return değeri kullanılmalı veya
`(void)` cast ile açıkça atılmalı.

**Deviation gerekçesi (kısmi):**
- `printf/fprintf`: hata olsa bile program akışı değişmez, çıktı
  best-effort'tur. Inetutils pings da return kontrolü yapmaz.
- `memcpy`: dest pointer'ı zaten döndürür, cleanup amaçlı kullanılır.
- `signal`: önceki handler — ihtiyaç yok.

**İyileştirme planı:** `(void)printf(...)` cast'leri eklenerek manuel
sıfırlanabilir (33 nokta). Subject acil değil.

**Kapsam:** Tüm dosyalar (printf yoğunluğu nedeniyle).

---

### 3.5 Rule 21.3 + 21.8 + 11.3 — Toplu Deviation Tablosu

| Dosya                                  | 21.3 | 21.8 | 11.3 | 17.7 | Notlar                          |
|----------------------------------------|------|------|------|------|----------------------------------|
| main.c                                 | ✓    | ✓    |      | ✓    | exit + free chain                |
| parse_args.c                           |      | ✓    |      | ✓    | exit on invalid opt              |
| ping_create_allocates.c                | ✓    |      |      |      | malloc + cleanup chain           |
| create_packet.c, create_payload.c      | ✓    |      |      | ✓    | packet buffer alloc              |
| create_send_request.c                  |      |      | ✓    | ✓    | sockaddr cast                    |
| create_socket_recvfrom.c               |      |      | ✓    | ✓    | sockaddr + ICMP cast             |
| get_network_*.c                        | ✓    |      | ✓    | ✓    | ifaddr + ARP socket calls        |
| interrupt_handler.c                    |      | ✓    |      | ✓    | exit on SIGINT                   |
| run_ping_create_functions.c            |      | ✓    |      | ✓    | exit on fn fail                  |

---

## 4. False Positive'ler

### 4.1 Rule 8.7 — Static Linkage

**Bulgu:** 14 fonksiyon prototype'ının `header.h` içinde olmasına
karşın "yalnızca tek dosyada kullanılıyor" diye işaretleniyor.

**Gerçek durum:** Bu fonksiyonlar `ping_create_functions.c` içindeki
function-pointer table (`t_ping_func_entry`) üzerinden indirect call
ile çağrılır. Static linkage uygun değil.

### 4.2 Rule 18.8 — VLA

**Bulgu:** `char buffer[1024]` constant-size array.

**Gerçek durum:** `1024` literal sabit, VLA değil. Cppcheck false positive.

---

## 5. Manuel Düzeltme Önerilen Kalanlar

| Rule | Dosya/Satır                | Açıklama                              |
|------|----------------------------|---------------------------------------|
| 14.4 | 3 nokta                    | `if (ptr)` → `if (ptr != NULL)`       |
| 11.5 | `ping_create_allocates.c`  | `void **` cast — alternatif: ayrı helper'lar |
| 11.8 | `printf` arg               | `(char *)` cast'ı kaldır              |
| 12.1 | 1 nokta                    | Operator precedence parens ekle       |
| 17.8 | 1 nokta                    | Loop var paramı yerel kopyala         |

---

## 6. Çalıştırma Komutları

```bash
# Container içinde
make misra                                    # cppcheck --addon=misra
docker exec ft_ping_container bash -lc \
    "cd /usr/src/ft_ping && make misra"

# CMake ile
cmake --build build --target misra

# Manuel (rule-texts dosyası ile)
cppcheck --addon=misra --std=c11 -I hdr src/ \
         --rule-texts=docs/cppcheck/misra_rules.txt \
         > docs/cppcheck/misra-output.txt 2>&1
```

---

## 7. Kabul Kriteri

ft_ping projesi için MISRA hedefi:

- ✅ **Mandatory** ihlali **yok**.
- ⚠ **Required** ihlalleri yalnızca **belgelenmiş deviation**'lar içinde.
- ⚠ **Advisory** ihlalleri kayıt altında, kod kalitesi tartışmasında
  ele alınır.

Subject Ecole 42 ft_ping için **MISRA uyumu zorunlu değildir**;
bu rapor opsiyonel bir kalite katmanıdır.

---

## 8. Referanslar

- **MISRA C:2012** — `docs/cppcheck/MISRA-C2012 Standards Model Summary for C  C++.pdf`
- **Cppcheck MISRA addon** — https://cppcheck.sourceforge.io/
- **Kural özeti** — [`docs/rules/03-misra-c2012.md`](../rules/03-misra-c2012.md)
