# 05 — Makefile Rules

> Subject `ft_ping` (R-01) Makefile için aşağıdaki kuralları zorunlu kılar.

---

## MK-01: Zorunlu Kurallar (Rules)

| Rule      | Açıklama                                                           |
|-----------|--------------------------------------------------------------------|
| `all`     | Default target — projeyi build eder.                               |
| `clean`   | Object file’ları (`*.o`) siler.                                    |
| `fclean`  | `clean` + executable’ı siler.                                      |
| `re`      | `fclean` + `all`.                                                  |

> Hiçbir target, **gerekmedikçe** yeniden derleme yapmamalıdır
> (incremental build). Bu, dependency tracking ile sağlanır.

---

## MK-02: Compiler & Flags

- Compiler: **`gcc`** veya **`cc`**.
- Zorunlu flags:
  - `-Wall` — tüm uyarılar.
  - `-Wextra` — ek uyarılar.
  - `-Werror` — uyarıyı hata olarak ele al.
- Tavsiye edilen:
  - `-std=c11` veya `-std=c17` — modern C standardı.
  - `-pedantic` — ISO uyumluluğu.
  - `-g` — geliştirme sırasında debug symbol.
  - `-Ihdr` — header arama yolu.
- **`-O2`** veya `-O3` final değerlendirme için açılabilir; geliştirme
  sırasında `-O0 -g` tercih edilir.

---

## MK-03: Phony Targets

`.PHONY` listesi **zorunlu**:

```makefile
.PHONY: all clean fclean re run
```

Aksi halde dosya sisteminde aynı isimde bir dosya bulunduğunda Make
hatalı davranır.

---

## MK-04: Pattern Rules & Auto-vars

- Pattern rule kullanılmalı:
  ```makefile
  $(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
  	$(CC) $(CFLAGS) -c $< -o $@
  ```
- **Order-only prerequisite** (`|`) ile dizin oluşturma side-effect’i
  rebuild’i tetiklemez.
- Auto-vars: `$<` (ilk prerequisite), `$@` (target), `$^` (tüm
  prerequisite’lar).

---

## MK-05: Header Dependency Tracking

- Header değişiminde otomatik rebuild için `-MMD -MP` flag’leri
  kullanılmalıdır:
  ```makefile
  CFLAGS += -MMD -MP
  -include $(OBJS:.o=.d)
  ```
- Bu olmadan `.h` değişiklikleri yakalanmaz; `re` her seferinde manuel
  çağrılmak zorunda kalır.

---

## MK-06: Silent / Verbose

- Komutlar **default olarak görünür** olmalı (debug için).
- `@` prefix yalnızca **`mkdir`**, **`echo`**, **`cd`** gibi yardımcı
  komutlarda kullanılmalıdır; `gcc` çağrıları görünür kalmalıdır.
- Renkli çıktı opsiyonel; CI / değerlendirme ortamında bozulmaması için
  `tput` veya `[ -t 1 ]` kontrolü yapılmalıdır.

---

## MK-07: Yapı

- Object’ler **`obj/`** dizinine yazılmalıdır; `src/` kirletilmemelidir.
- Executable **`output/ft_ping`** veya project root altına yazılabilir;
  subject `ft_ping` adını **zorunlu** kılar.
- Dependency dosyaları (`*.d`) `obj/` altında tutulmalı.

---

## MK-08: Standart Olmayan Targets (Opsiyonel)

| Target      | Açıklama                                                    |
|-------------|-------------------------------------------------------------|
| `run`       | Build edip executable’ı çalıştırır.                          |
| `debug`     | `-g -O0 -fsanitize=address` ile rebuild.                     |
| `valgrind`  | Valgrind altında çalıştırır.                                 |
| `norm`      | `norminette src/ hdr/` çalıştırır.                           |
| `cppcheck`  | Cppcheck statik analizini çalıştırır.                        |

> Subject **bu target’ları zorunlu kılmaz**; `all/clean/fclean/re` dışı
> her şey değerlendiriciye yardımcı olmak içindir.

---

## MK-09: Forbidden / Anti-patterns

- `clean` veya `fclean` içinde **`rm -rf /`** veya kontrolsüz
  recursive delete yasak.
- `find ... -delete` yerine **explicit** dosya listesi tercih edilmeli.
- Object file’ları silmeden executable’ı silmek **`fclean`** ile
  çelişir.
- `make` içinde **`sudo`** yasak (subject doğrudan yasaklamasa bile,
  defense sırasında izin/güven sorunu yaratır). ft_ping için sudo
  gerekirse **runtime**’da çağrılır, build sırasında değil.

---

## MK-10: ft_ping Mevcut Makefile Değerlendirmesi

| Kural   | Mevcut Durum | Notlar                                                  |
|---------|--------------|---------------------------------------------------------|
| MK-01   | ✓            | `all clean fclean re` mevcut.                            |
| MK-02   | ⚠            | `-std=` belirtilmemiş; `-pedantic` yok.                  |
| MK-03   | ✓            | `.PHONY` mevcut.                                         |
| MK-04   | ✓            | Pattern rule + order-only prerequisite kullanılmış.      |
| MK-05   | ✗            | `-MMD -MP` yok — header değişimi rebuild tetiklemiyor.   |
| MK-06   | ⚠            | `@mkdir` doğru; gcc çağrıları görünür (✓).               |
| MK-07   | ✓            | `obj/` ve `output/` ayrımı var.                          |
| MK-08   | ✓            | `run` target eklenmiş (SUDO/ARGS desteği ile).           |
| MK-09   | ✓            | Sudo build yok; `clean` kontrollü.                       |
