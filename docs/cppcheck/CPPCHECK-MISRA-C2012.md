Aşağıdaki C projesini MISRA C:2012 ve Cppcheck açısından incele.

Hedefler:
1) Kodda olası bug, undefined behavior, null dereference, bounds, resource leak, uninitialized variable, dead code ve portability sorunlarını tespit et.
2) MISRA C:2012 açısından ihlal olabilecek noktaları bul.
3) Çıktıyı kısa ve uygulanabilir bir rapor halinde ver.

Kurallar:
- Önce dosya/dizin yapısını kısaca özetle.
- Sonra her bulguyu şu formatta yaz:
  - Dosya:
  - Satır:
  - Tür: (bug / warning / style / portability / misra)
  - Kural veya kategori: (örn. c2012-17.7, nullPointer, unreadVariable)
  - Açıklama:
  - Risk seviyesi: düşük / orta / yüksek
  - Önerilen düzeltme:
- Yanlış pozitif olabilecek bulguları ayrıca “Muhtemel false positive” diye işaretle.
- En sonda:
  - “Öncelikli düzeltilecek 10 konu”
  - “MISRA odaklı özet”
  - “Derleme ve analiz komutları”
başlıklarıyla özet ver.

Cppcheck komutları:
1) Genel analiz:
   cppcheck --enable=all --inconclusive --std=c11 --force --quiet src/ 2> cppcheck.txt

2) MISRA analizi:
   cppcheck --dump --std=c11 src/*.c
   python addons/misra.py src/*.dump > misra.txt

3) Eğer rule texts dosyası varsa:
   python addons/misra.py --rule-texts=misra_rules.txt src/*.dump > misra.txt

İnceleme çıktısında özellikle şunlara dikkat et:
- pointer arithmetic
- cast işlemleri
- signed/unsigned karışımı
- switch fallthrough
- implicit conversion
- magic numbers
- global state
- recursion
- macro güvenliği
- function return value kontrolü
- malloc/free eşleşmesi
- stdin/stdout/stderr hata kontrolü
- lex/yacc veya flex/bison ile üretilen entegrasyon kodlarında güvenlik riskleri

Ben sana kodları parça parça veya tüm proje olarak vereceğim. Önce analiz et, sonra düzeltme patch’lerini öner.
---

# MISRA C:2012 & Cppcheck Analiz Raporu

**Tarih:** 2025-04 (Son güncelleme)
**Proje:** B Programming Language → x86-32 Assembly Compiler
**Araç:** Cppcheck 2.x + Manuel MISRA C:2012 İncelemesi

## Dosya/Dizin Yapısı Özeti

```
main.c                  — Ana giriş noktası (yyparse çağrısı)
src/codegen.c           — Temel emit fonksiyonları (header, function, const)
src/codegen_arith.c     — Aritmetik: add, sub, mul, div, mod
src/codegen_bit.c       — Bit: neq, band, bor, lshift, rshift
src/codegen_call.c      — Fonksiyon çağrısı: indirect call, stack reverse
src/codegen_cmp.c       — Karşılaştırma: less, greater, eq
src/codegen_ctrl.c      — Kontrol akışı: label, jump, return
src/codegen_def.c       — Tanımlama: extern def, vector def
src/codegen_extrn.c     — Extrn: auto alloc, compound load
src/codegen_post.c      — Postfix: inc, dec, subscript
src/codegen_stremit.c   — String: strip quotes, escaped print, inline emit
src/codegen_string.c    — Section switch: text, rodata, invalidate
src/codegen_unary.c     — Unary: negate, not, deref, prefix inc/dec
src/codegen_var.c       — Değişken: var load/addr, param load/addr, store
src/codegen_vec.c       — Vector: value, string value, name value, end
src/symtable.c          — Sembol tablosu: add_auto, add_param, add_extrn, reset
src/symtable_lookup.c   — Sembol arama: lookup_offset, lookup_class, exists
src/lexer/lexer.l       — Flex lexer (tokenization)
src/parser/parser.y     — Bison parser (syntax-directed codegen)
hdr/codegen.h           — Codegen prototipler
hdr/codegen_string.h    — String/section prototipler
hdr/lexer.h             — Lexer extern tanımları
hdr/parser.h            — Parser extern tanımları
hdr/symtable.h          — Sembol tablosu yapıları ve prototipler
```

## Tespit Edilen Bulgular ve Uygulanan Düzeltmeler

### 1. malloc NULL Dereference (DÜZELTİLDİ ✅)

- **Dosya:** src/codegen_stremit.c (cg_strip_quotes)
- **Tür:** bug / nullPointer
- **Kural:** MISRA D.4.1, R.22.1
- **Açıklama:** `malloc()` dönüş değeri kontrol edilmeden kullanılıyordu
- **Risk:** Yüksek
- **Düzeltme:** NULL kontrolü ve hata mesajı eklendi

### 2. strdup NULL Dereference (DÜZELTİLDİ ✅)

- **Dosya:** src/symtable.c (sym_add_auto, sym_add_auto_vector, sym_add_param, sym_add_extrn)
- **Tür:** bug / nullPointer
- **Kural:** MISRA D.4.1, D.4.7
- **Açıklama:** 4 ayrı `strdup()` çağrısının dönüş değeri kontrol edilmiyordu
- **Risk:** Yüksek
- **Düzeltme:** Her strdup sonrası NULL check + fprintf(stderr) + exit(1) eklendi

### 3. Redundant Condition (DÜZELTİLDİ ✅)

- **Dosya:** src/codegen_stremit.c:73
- **Tür:** style / knownConditionTrueFalse
- **Kural:** Cppcheck — gereksiz koşul
- **Açıklama:** `if (n && quoted[n-1] == '"' && n > 0)` — `n > 0` zaten `n` ile garanti altında
- **Risk:** Düşük
- **Düzeltme:** Koşul `if (n > 0 && quoted[n-1] == '"')` olarak sadeleştirildi

### 4. atoi() Kullanımı (DÜZELTİLDİ ✅)

- **Dosya:** src/lexer/lexer.l
- **Tür:** misra / portability
- **Kural:** MISRA R.21.7 — atoi/atol/atof kullanılmamalı
- **Açıklama:** `atoi(yytext)` hata kontrolü yapmaz, taşma durumlarında undefined behavior
- **Risk:** Orta
- **Düzeltme:** `(int)strtol(yytext, NULL, 10)` ile değiştirildi

### 5. Array Bounds Overflow (DÜZELTİLDİ ✅)

- **Dosya:** src/parser/parser.y (if_labels[256], arg_counts[256])
- **Tür:** bug / bufferAccessOutOfBounds
- **Kural:** MISRA D.4.1 — runtime failures shall be minimized
- **Açıklama:** `if_depth++` ve `arg_depth++` sınır kontrolü olmadan artırılıyordu
- **Risk:** Yüksek
- **Düzeltme:** `MAX_NEST_DEPTH` define + bounds check + YYABORT eklendi

### 6. Variable Scope (DÜZELTİLDİ ✅)

- **Dosya:** src/codegen_call.c (emit_call_indirect)
- **Tür:** style / variableScope
- **Kural:** MISRA R.8.9 — değişken mümkün olduğunca dar kapsamda tanımlanmalı
- **Açıklama:** `i`, `lo`, `hi` fonksiyon başında tanımlanmış ama sadece bir blokta kullanılıyor
- **Risk:** Düşük
- **Düzeltme:** Değişkenler kullanıldıkları blok kapsamına taşındı

### 7. fprintf Return Value Unchecked (DÜZELTİLDİ ✅)

- **Dosya:** main.c, src/symtable.c, src/parser/parser.y, src/lexer/lexer.l
- **Tür:** misra
- **Kural:** MISRA R.17.7 — non-void dönüşlü fonksiyonun değeri kullanılmalı
- **Açıklama:** `fprintf(stderr, ...)` dönüş değeri yok sayılıyordu
- **Risk:** Düşük
- **Düzeltme:** Tüm hata çıkışlarında `(void)fprintf(...)` eklendi

### 8. yylex_destroy Return Value (DÜZELTİLDİ ✅)

- **Dosya:** main.c
- **Tür:** misra
- **Kural:** MISRA R.17.7
- **Açıklama:** `yylex_destroy()` dönüş değeri kullanılmıyordu
- **Risk:** Düşük
- **Düzeltme:** `(void)yylex_destroy()` eklendi

---

## Muhtemel False Positive'ler

| Bulgu | Dosya | Neden |
|-------|-------|-------|
| unusedFunction (tüm emit fonksiyonları) | src/codegen_*.c, src/symtable*.c | Cppcheck parser.y/lexer.l tarafından üretilen kodu görmüyor; bu fonksiyonlar bison/flex çıktısından çağrılır |
| missingIncludeSystem | tüm dosyalar | Cppcheck sisteme ait header path'leri bulamıyor; derleme hatası yok |

---

## Bilinçli MISRA Sapmaları (Deviation)

| Kural | Açıklama | Gerekçe |
|-------|----------|---------|
| R.21.3 | malloc/free kullanımı | Compiler projesinde dinamik bellek zorunlu (strdup, string buffer) |
| R.21.6 | stdio.h kullanımı (printf) | Compiler çıktısı stdout'a yazılır, projenin doğası gereği |
| R.21.8 | exit() kullanımı | Fatal hatalarda (sembol tablo taşması, bellek hatası) güvenli çıkış |
| R.15.1 | goto — B dilinde var | Compiler'ın kendisinde goto yok; B programlarda goto derleniyor |
| D.4.6 | Basic type (int, char) kullanımı | 42/Ecole normu int/char tercih eder, stdint.h kullanılmaz |

---

## Öncelikli Düzeltilecek 10 Konu

| # | Durum | Konu | Risk | Kural |
|---|-------|------|------|-------|
| 1 | ✅ Düzeltildi | malloc NULL check (cg_strip_quotes) | Yüksek | D.4.1 |
| 2 | ✅ Düzeltildi | strdup NULL check (symtable 4 fonksiyon) | Yüksek | D.4.1/D.4.7 |
| 3 | ✅ Düzeltildi | Array bounds (if_labels/arg_counts) | Yüksek | D.4.1 |
| 4 | ✅ Düzeltildi | atoi → strtol (lexer.l) | Orta | R.21.7 |
| 5 | ✅ Düzeltildi | Redundant condition (stremit) | Düşük | Cppcheck |
| 6 | ✅ Düzeltildi | Variable scope (codegen_call) | Düşük | R.8.9 |
| 7 | ✅ Düzeltildi | fprintf (void) cast | Düşük | R.17.7 |
| 8 | ✅ Düzeltildi | yylex_destroy (void) cast | Düşük | R.17.7 |
| 9 | N/A | printf (void) cast (codegen files) | Çok düşük | R.17.7 — bilinçli sapma |
| 10 | N/A | static label_buf thread safety | Çok düşük | N/A — single-threaded compiler |

## MISRA Odaklı Özet

**Uyumluluk Seviyesi:** Yüksek (uygulanan düzeltmeler sonrası)

- **Mandatory kurallar:** D.4.1 (runtime failure minimize) ve R.9.1 (uninit read) uyumlu
- **Required kurallar:** R.17.7 (return value), R.21.7 (atoi), R.22.1 (resource release) düzeltildi
- **Advisory kurallar:** R.8.9 (scope), R.15.5 (single exit) büyük ölçüde uyumlu
- **Sapmalar:** R.21.3, R.21.6, R.21.8 — compiler doğası gereği document edildi
- **Header koruma:** Tüm header dosyalarında `#ifndef` guard mevcut (D.4.10 ✅)
- **Switch default:** Tüm switch'lerde tüm enum case'leri işlenmiş
- **Null pointer:** Tüm malloc/strdup sonrası kontrol eklendi

## Derleme ve Analiz Komutları

```bash
# Derleme (Docker üzerinde)
docker exec bdev bash -c "cd /workspace && make re"

# Test çalıştırma
docker exec bdev bash -c "cd /workspace && sh tests/run_tests.sh"

# Cppcheck genel analiz
docker exec bdev bash -c "cd /workspace && cppcheck --enable=all --inconclusive --std=c11 --force --quiet src/ main.c 2>&1"

# Cppcheck MISRA analizi (dump + addon)
docker exec bdev bash -c "cd /workspace && cppcheck --dump --std=c11 src/*.c && python3 /usr/share/cppcheck/addons/misra.py src/*.dump"

# GCC strict warnings (zaten Makefile'da)
# gcc -Wall -Wextra -Werror -g -Ihdr
```