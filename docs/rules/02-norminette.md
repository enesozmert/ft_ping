# 02 — Norminette Rules (Ecole 42)

> Kaynak: `docs/normcheck/norme.en.pdf` (The Norm — Version 2.0.2)
>
> Bu dosya, projedeki `.c` ve `.h` dosyaları için Ecole 42 Norminette
> kurallarının özetidir. Detaylı denetim için bkz. `docs/normcheck/NORMCHECK.md`.

---

## NM-01: Denomination (İsimlendirme)

- `struct` isimleri `s_` ön ekiyle başlamalıdır.
- `typedef` isimleri `t_` ön ekiyle başlamalıdır.
- `union` isimleri `u_` ön ekiyle başlamalıdır.
- `enum` isimleri `e_` ön ekiyle başlamalıdır.
- **Global variable** isimleri `g_` ön ekiyle başlamalıdır.
- Değişken / fonksiyon isimleri yalnızca **lowercase**, **digit** ve `_`
  içerebilir (snake_case / Unix Case).
- Tüm tanımlayıcılar **İngilizce** olmalıdır.
- Standart **ASCII** dışındaki karakterler yasaktır.

---

## NM-02: Formatting

- Her dosya **42 header** ile başlamalıdır.
- Bir satır **80 column**’u aşamaz (header dahil).
- Bir fonksiyon **25 line**’ı aşamaz (parantezler hariç).
- Bir fonksiyon **maksimum 4 parameter** alabilir.
- Bir blok **maksimum 5 variable declaration** içerebilir.
- Bir satırda **tek bir instruction** bulunmalıdır.
- Indentation **tab** ile yapılır (4 columns genişliğinde).
- **Trailing whitespace** ve birden fazla ardışık boş satır yasaktır.
- Operator’ler etrafında **tek bir space** bulunmalıdır.

---

## NM-03: Functions

- Bir `.c` dosyası en fazla **5 fonksiyon** içerebilir.
- `for`, `while`, `do...while`, `if`, `else if`, `else`, `switch`, `case`,
  `goto` kullanımı **çok kısıtlıdır** veya bazı projelerde tamamen
  yasaktır.
- **Variable declaration** fonksiyon başında yapılmalı; assignment ile
  birleştirilmemelidir (sadece `const` ve `static` istisna).
- Fonksiyon parametreleri **typed** olmalıdır; `void` parametre yoksa
  açıkça yazılmalıdır.

---

## NM-04: Control Structures

- Tek satırlık `if/else/while/for` blokları bile **brace**’ler ile
  yazılmalıdır.
- `goto` kullanımı yasaktır.
- `switch` içinde her `case` ya `break` ya `return` ile sonlanmalıdır.
- Conditional içinde **assignment** yasaktır.

---

## NM-05: Header Files (`.h`)

- Tüm `.h` dosyaları **include guard** içermelidir:
  ```c
  #ifndef HEADER_NAME_H
  # define HEADER_NAME_H
  /* ... */
  #endif
  ```
- `.h` dosyaları içinde **fonksiyon implementasyonu** yapılmaz; yalnızca
  prototype, macro, typedef, struct/union/enum tanımları bulunur.
- Include directive’leri **alfabetik** veya **mantıksal grup**’lar halinde
  sıralanmalıdır.

---

## NM-06: Macros & Pre-processor

- Macro isimleri **UPPERCASE** olmalıdır.
- Multi-statement macro’lar `do { ... } while (0)` deyimi ile sarılmalıdır.
- Pre-processor instruction’ları indent edilmelidir (`#`’dan sonra space).

---

## NM-07: Forbidden Things (Genelde)

- `for`/`while` döngüsünde **iki noktalı virgül**li boş gövde yasaktır.
- **Multiple assignment** tek satırda yasaktır (`a = b = c`).
- **Implicit type** (eski K&R) tanımları yasaktır.
- **VLA** (variable-length array) genelde yasaktır.
- **Ternary operator** (`?:`) bazı projelerde sınırlandırılmıştır;
  ft_ping’de kullanım gerekçelendirilebilir olmalıdır.

---

## NM-08: Comments

- Yorumlar **fonksiyon gövdesi içinde** olamaz; sadece fonksiyonun
  dışında bulunabilir.
- Yorumlar **İngilizce** olmalıdır.
- Yorumlar **anlamlı** olmalıdır; “bu döngü” gibi yorumlar değer katmaz.

---

## NM-09: Files & Directories

- Dosya / dizin isimleri yalnızca lowercase, digit ve `_` içerebilir.
- Dosya **derlenebilir** olmalıdır (norminette derlemeden de geçirir).
- Bir `.c` dosyası kendi `.h` dosyasını ya da projenin merkezi `.h`
  dosyasını dahil etmelidir.

---

## ft_ping için Norminette Etki Alanı

| Kapsam İçi              | Kapsam Dışı                                |
|-------------------------|--------------------------------------------|
| `src/*.c`               | `obj/*.o` (derleme çıktısı)                |
| `hdr/*.h`               | `output/ft_ping` (executable)              |
| `Makefile` (info)       | `docker/`, `docs/` (norm denetimi yok)     |
