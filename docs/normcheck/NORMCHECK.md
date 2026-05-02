# Normcheck — Ecole 42 Norm Uyumluluk Kural Seti

> Bu doküman, `norme.en.pdf` (The Norm — Version 2.0.2) referans alınarak
> hazırlanmıştır. Projedeki `.c` ve `.h` dosyalarının Ecole 42 Norminette
> kurallarına uygunluğunu denetlemek için kullanılacak kural setidir.
>
> **Not:** `flex` (`.l`) ve `bison` (`.y`) dosyaları ile otomatik üretilen
> `obj/` altındaki dosyalar (`lex.yy.c`, `y.tab.c`, `y.tab.h`) norm
> denetimi kapsamı **dışındadır**.

---

## Kapsam

| Kapsam İçi | Kapsam Dışı |
|------------|-------------|
| `src/*.c` | `obj/*.c` (flex/bison üretimi) |
| `hdr/*.h` | `obj/*.h` (flex/bison üretimi) |
| `main.c` | `src/lexer/lexer.l` (flex DSL) |
| | `src/parser/parser.y` (bison DSL) |

---

## N-01: İsimlendirme (Denomination) — PDF §II.1

### Zorunlu (Mandatory)

- Struct isimleri `s_` ön ekiyle başlamalıdır.
- Typedef isimleri `t_` ön ekiyle başlamalıdır.
- Union isimleri `u_` ön ekiyle başlamalıdır.
- Enum isimleri `e_` ön ekiyle başlamalıdır.
- Global değişken isimleri `g_` ön ekiyle başlamalıdır.
- Değişken ve fonksiyon isimleri yalnızca **küçük harf**, **rakam** ve `_`
  içerebilir (Unix Case / snake_case).
- Dosya ve dizin isimleri yalnızca **küçük harf**, **rakam** ve `_`
  içerebilir.
- Dosya **derlenebilir** olmalıdır.
- Standart ASCII tablosunda olmayan karakterler **yasaktır**.

### Tavsiye (Advice)

- İsimler mümkün olduğunca açıklayıcı veya anımsatıcı olmalıdır.
  Yalnızca sayaç değişkenleri (`i`, `j`, `k`) serbest adlandırılabilir.
- Kısaltmalar orijinal ismi kısaltmak için tolere edilir, anlaşılır
  kaldığı sürece. Birden fazla kelime `_` ile ayrılmalıdır.
- Tüm tanımlayıcılar (fonksiyon, makro, tip, değişken vb.) **İngilizce**
  olmalıdır.
- Global değişken kullanımı gerekçelendirilebilir olmalıdır.

```c
typedef struct s_symbol
{
	char	*name;
	int		offset;
}	t_symbol;

/* Doğru örnekler */
static int	g_label_counter = 0;   /* g_ ön eki — global */
int			emit_instruction(void); /* snake_case — fonksiyon */

/* Yanlış örnekler */
static int	label_counter = 0;     /* g_ ön eki eksik */
int			EmitInstruction(void); /* camelCase yasak */
```

---

## N-02: Biçimlendirme (Formatting) — PDF §II.2

### Zorunlu (Mandatory)

- Tüm dosyalar 42 standart **header yorum bloğu** ile başlamalıdır
  (dosyanın ilk satırından itibaren).
```c
/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dosya_adi.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: login <login@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: YYYY/MM/DD HH:MM:SS by login        #+#    #+#                 */
/*   Updated: YYYY/MM/DD HH:MM:SS by login       ###   ########.fr          */
/*                                                                            */
/* ************************************************************************** */
```
- Girinti için **4 boşluk genişliğinde gerçek tab karakteri** kullanılmalıdır.
  Boşlukla girinti **yasaktır**.
- Bir fonksiyon **en fazla 25 satır** olabilir; fonksiyonun kendi `{` ve `}`
  süslü parantezleri sayıma **dahil değildir**.
- Bir satır en fazla **80 sütun** genişliğinde olabilir (yorumlar dahil).
  **Uyarı:** Tab, sütun olarak değil temsil ettiği boşluk sayısı kadar sayılır.
- Her satırda **tek bir ifade (instruction)** bulunmalıdır.
- Boş satır gerçekten boş olmalıdır: **boşluk veya tab içermemelidir**.
- Satır sonunda boşluk veya tab **bulunamaz** (trailing whitespace yasak).
- Her süslü parantez (`{`, `}`) veya kontrol yapısı sonunda **yeni satıra**
  geçilmelidir.
- Satır sonu olmadıkça, her virgül veya noktalı virgülden sonra **bir boşluk**
  olmalıdır.
- Her operatör (binary veya ternary) veya operand, **bir ve yalnızca bir**
  boşlukla ayrılmalıdır.
- Her C anahtar kelimesinden sonra **bir boşluk** olmalıdır; **istisna:** tip
  anahtar kelimeleri (`int`, `char`, `float` vb.) ve `sizeof`.
- Değişken tanımlamaları **aynı sütunda hizalanmalıdır**.
- Pointer'lardaki yıldız (`*`) işareti **değişken adına yapışık** olmalıdır.
- Her satırda **tek bir değişken tanımı** yapılmalıdır.
- Tanım ve ilk değer ataması **aynı satırda yapılamaz**; **istisna:** global
  değişkenler ve `static` değişkenler.
- Tanımlamalar fonksiyonun **başında** olmalı ve bir **boş satır** ile
  gövdeden ayrılmalıdır.
- Tanımlar arasında veya ifadeler (implementation) arasında **boş satır
  olamaz**.
- Çoklu atama (multiple assignment) **kesinlikle yasaktır**.
- Bir ifade veya kontrol yapısından sonra yeni satır eklenebilir, ancak
  parantez veya atama operatörüyle girintilenmeli; operatörler **satırın
  başında** olmalıdır.

```c
int		foo(int a, int b)
{
	int	result;
	int	temp;

	temp = a + b;
	result = temp * 2;
	return (result);
}
```

---

## N-03: Fonksiyon Parametreleri (Functions Parameters) — PDF §II.3

### Zorunlu (Mandatory)

- Bir fonksiyon **en fazla 4 isimli parametre** alabilir.
- Parametre almayan fonksiyon, `void` ile **açıkça** prototiplenmelidir:
```c
void	emit_header(void);   /* Doğru */
void	emit_header();        /* Yanlış — norm hatası */
```

---

## N-04: Fonksiyonlar (Functions) — PDF §II.4

### Zorunlu (Mandatory)

- Fonksiyon prototiplerindeki parametreler **isimlendirilmelidir**.
- Her fonksiyon bir sonraki fonksiyondan **bir boş satır** ile ayrılmalıdır.
- Bir blok içinde **en fazla 5 değişken** tanımlanabilir.
- `return` değeri **parantez içinde** olmalıdır:
```c
return (value);    /* Doğru */
return value;      /* Yanlış */
return ;           /* Doğru — void fonksiyon (değer yok) */
```

### Tavsiye (Advice)

- Aynı dosyadaki fonksiyon tanımlayıcıları (dönüş tipi + isim) **hizalı**
  olmalıdır. Header dosyalarındaki prototipler için de aynısı geçerlidir.

---

## N-05: Typedef, Struct, Enum ve Union — PDF §II.5

### Zorunlu (Mandatory)

- Struct, enum veya union tanımlarken **bir tab** girintisi eklenmelidir.
- Struct, enum veya union tipinde değişken tanımlarken tipte **tek bir boşluk**
  bırakılmalıdır.
- Typedef'in iki parametresi arasında **bir tab** olmalıdır.
- Typedef ile struct/union/enum tanımlanırken tüm kurallar geçerlidir;
  typedef ismi ile struct/union/enum ismi **hizalanmalıdır**.
- Bir `.c` dosyasında struct tanımı **yapılamaz** (yalnızca `.h` dosyalarında).

```c
typedef struct s_symbol
{
	char	*name;
	int		offset;
}	t_symbol;
```

---

## N-06: Header Dosyaları (Headers) — PDF §II.6

### Zorunlu (Mandatory)

- Header dosyalarında **yalnızca** şunlara izin verilir:
  header include'ları (sistem veya proje), tanımlamalar (declarations),
  `#define`'lar, prototipler ve makrolar.
- Tüm include'lar (`.c` veya `.h`) dosyanın **en başında** olmalıdır.
- Header'lar **çift dahil etmeye (double inclusion) karşı korunmalıdır**.
  Dosya adı `ft_foo.h` ise koruma makrosu `FT_FOO_H` olmalıdır.
```c
#ifndef CODEGEN_H
# define CODEGEN_H

/* içerik */

#endif
```
- `#pragma once` **kullanılmamalıdır**.
- `#define` satırları `#ifndef`/`#ifdef`/`#if` bloğu içinde **girintili**
  (`# define`) olmalıdır.
- Fonksiyon prototipleri **yalnızca** `.h` dosyalarında bulunmalıdır.
- Kullanılmayan header include'ları **yasaktır**.

### Tavsiye (Advice)

- Tüm header include'ları `.c` ve `.h` dosyalarında **gerekçelendirilmiş**
  olmalıdır.
- Önce `<>` (sistem), sonra `""` (proje) include'ları.
- Her grup içinde **alfabetik sıralama** önerilir.

---

## N-07: Makrolar ve Ön İşlemci (Macros and Pre-processors) — PDF §II.7

### Zorunlu (Mandatory)

- Ön işlemci sabitleri (`#define`) yalnızca **literal ve sabit değerleri**
  ilişkilendirmek için kullanılmalıdır.
- Normu atlatmak ve/veya kodu gizlemek için oluşturulmuş `#define`'lar
  **yasaktır** (bu madde insan tarafından denetlenmelidir).
- Standart kütüphanelerdeki makrolar yalnızca projenin izin verdiği
  kapsamda kullanılabilir.
- Çok satırlı makrolar **yasaktır**.
- Yalnızca makro isimleri **büyük harf** (UPPER_SNAKE_CASE) olmalıdır.
- `#if`, `#ifdef` veya `#ifndef` sonrasındaki karakterler **girintili**
  olmalıdır.

```c
#define MAX_SYMBOLS 1024       /* Kabul — sabit değer */
#define SQUARE(x) ((x)*(x))   /* Yasak — makro fonksiyon */
```

---

## N-08: Yasaklı Yapılar (Forbidden Stuff!) — PDF §II.8

### Zorunlu (Mandatory)

Aşağıdakiler **yasaktır**:
- `for`
- `do...while`
- `switch`
- `case`
- `goto`
- İç içe (nested) ternary operatörü (`?`)
- VLA (Variable Length Arrays)

| Yasak | Alternatif |
|-------|-----------|
| `for` | `while` |
| `do...while` | `while` |
| `switch/case` | `if/else if/else` |
| `goto` | Döngü/kontrol yapıları |
| Nested ternary | Ayrı `if/else` veya tek seviye ternary |
| VLA | `malloc` veya sabit boyut |

```c
/* Yasak */
for (i = 0; i < n; i++)
	do_something(i);

/* Doğru alternatif */
i = 0;
while (i < n)
{
	do_something(i);
	i++;
}

/* Kabul — tek seviye ternary */
x = (a > 0) ? 1 : 0;

/* Yasak — nested ternary */
x = (a > 0) ? (b ? 1 : 2) : 0;
```

---

## N-09: Yorumlar (Comments) — PDF §II.9

### Zorunlu (Mandatory)

- Kaynak dosyalarda yorum **yapılabilir**.
- Fonksiyon gövdesi içinde yorum **yasaktır**.
- Yorumlar tek satırda başlamalı ve tek satırda bitmelidir. Ara satırlar
  hizalı olmalı ve `**` ile başlamalıdır:
```c
/*
** Bu çok satırlı bir yorumdur.
** Her ara satır ** ile başlar.
*/
```
- `//` stili yorum **yasaktır**.

### Tavsiye (Advice)

- Yorumlar **İngilizce** olmalıdır ve faydalı olmalıdır.
- Bir yorum, kötü yazılmış bir fonksiyonu gerekçelendiremez.

---

## N-10: Dosyalar (Files) — PDF §II.10

### Zorunlu (Mandatory)

- Bir `.c` dosyası başka bir `.c` dosyasını **include edemez**.
- Bir `.c` dosyasında **en fazla 5 fonksiyon tanımı** bulunabilir.

---

## N-11: Makefile — PDF §II.11

### Zorunlu (Mandatory)

- `$(NAME)`, `clean`, `fclean`, `re` ve `all` kuralları **zorunludur**.
- Makefile **relink yapmamalıdır** (gereksiz yeniden derleme).
  Relink yapılırsa proje fonksiyonel kabul edilmez.
- Çoklu binary projelerinde tüm binary'leri derleyen bir kural ile
  her binary için ayrı kural bulunmalıdır.
- Bir fonksiyon kütüphanesi çağrılıyorsa (örn: libft), Makefile bu
  kütüphaneyi **otomatik olarak derlemelidir**.
- Derlenmesi gereken tüm kaynak dosyalar Makefile'da **açıkça
  isimlendirilmelidir** (wildcard kullanımı ⚠).

---

## N-12: Proje Dosya Organizasyonu

### N-12a: Dizin Yapısı
```
.
├── Makefile
├── main.c
├── hdr/
│   ├── codegen.h
│   ├── codegen_string.h
│   ├── lexer.h
│   ├── parser.h
│   └── symtable.h
├── src/
│   ├── codegen.c          (≤ 5 fonksiyon)
│   ├── codegen_string.c   (≤ 5 fonksiyon)
│   ├── symtable.c         (≤ 5 fonksiyon)
│   ├── lexer/
│   │   └── lexer.l        (norm dışı)
│   └── parser/
│       └── parser.y       (norm dışı)
└── obj/                    (derleme çıktıları — norm dışı)
```

### N-12b: Dosya Bölme Stratejisi
Bir `.c` dosyası 5 fonksiyon sınırını aşıyorsa:
1. Fonksiyonları mantıksal gruplara ayır.
2. Yeni bir dosya oluştur (örn: `codegen_ops.c`, `codegen_ctrl.c`).
3. Public fonksiyon prototiplerini ilgili `.h` dosyasına ekle.
4. `static` yardımcı fonksiyonlar tanımlandıkları dosyada kalır.
5. Makefile'a yeni kaynak dosyayı ekle.

### N-12c: obj/ Dizini
- `obj/` dizinindeki dosyalar flex/bison tarafından otomatik üretilir.
- Bu dosyalar **versiyon kontrolüne eklenmemeli** (`.gitignore`).
- Norm denetimi uygulanmaz.

---

## N-13: Proje Spesifik İstisnalar

Bu projede aşağıdaki kurallar özel koşullarda esnetilebilir:
- `printf()` kullanımı **serbesttir** (assembly çıktısı stdout'a yazılır).
- `fprintf(stderr, ...)` kullanımı **serbesttir** (hata mesajları).
- `malloc()` / `free()` / `strdup()` gibi bellek fonksiyonları **serbesttir**
  (subject tüm glibc fonksiyonlarını serbest bırakır).
- `exit()` kullanımı **serbesttir** (hata durumlarında çıkış).
- `switch/case` yalnızca `codegen_string.c` gibi karakter dönüşüm
  dosyalarında tolerans gösterilebilir (alternatif: `if/else if` zincirine çevirmek).

---

## Kontrol Listesi (Checklist)

Her dosya için aşağıdaki kontrol listesi uygulanmalıdır:

- [ ] 42 header yorum bloğu mevcut (dosyanın ilk satırından itibaren)
- [ ] Satır uzunluğu ≤ 80 sütun (tab = temsil ettiği boşluk sayısı)
- [ ] Fonksiyon uzunluğu ≤ 25 satır (`{` / `}` hariç)
- [ ] Dosya başına ≤ 5 fonksiyon tanımı
- [ ] Fonksiyon başına ≤ 4 isimli parametre
- [ ] Blok başına ≤ 5 değişken
- [ ] Değişken tanımları fonksiyon/blok başında, boş satırla ayrılmış
- [ ] Tanım satırında atama yok (global/static hariç)
- [ ] Her satırda tek tanım, tek ifade
- [ ] Tanımlar arası ve ifadeler arası boş satır yok
- [ ] Çoklu atama yok
- [ ] `for`, `do...while`, `switch/case`, `goto` kullanılmamış
- [ ] Nested ternary yok, VLA yok
- [ ] `//` yorum stili yok
- [ ] Fonksiyon gövdesinde yorum yok
- [ ] Çok satırlı yorumlarda ara satırlar `**` ile başlıyor
- [ ] Makro fonksiyon yok, çok satırlı makro yok
- [ ] Header guard mevcut ve doğru (`#pragma once` yok)
- [ ] Fonksiyon prototipleri yalnızca `.h` dosyalarında
- [ ] Kullanılmayan include yok
- [ ] `.c` dosyası başka bir `.c` dosyasını include etmiyor
- [ ] snake_case isimlendirme (değişken, fonksiyon, dosya, dizin)
- [ ] Struct → `s_`, typedef → `t_`, union → `u_`, enum → `e_`, global → `g_`
- [ ] Makro isimleri UPPER_SNAKE_CASE
- [ ] Pointer `*` değişken adına yapışık
- [ ] Anahtar kelimelerden sonra boşluk (tip kelimeleri ve `sizeof` hariç)
- [ ] Operatörler birer boşlukla ayrılmış
- [ ] `{` ve `}` kendi satırlarında
- [ ] Trailing whitespace yok, boş satır gerçekten boş
- [ ] `return (value)` formatı
- [ ] Prototiplerde parametreler isimlendirilmiş
- [ ] `void` parametresiz fonksiyonlarda belirtilmiş
- [ ] Makefile: `$(NAME)`, `all`, `clean`, `fclean`, `re` mevcut
- [ ] Makefile relink yapmıyor, kaynaklar açıkça isimlendirilmiş

---

## Not

- Bu kural seti **norme.en.pdf — The Norm Version 2.0.2** referans
  alınarak hazırlanmıştır.
- `flex` (`.l`) ve `bison` (`.y`) dosyaları kendi DSL formatlarına
  sahip olduğundan norm denetimi kapsamı dışındadır.
- Subject'in `Any function from the standard library or glibc is allowed`
  kuralı nedeniyle fonksiyon kısıtlamaları sadece Norminette yapısal
  kurallarındadır; kütüphane fonksiyonu yasağı yoktur.
- Bazı kurallar (42 header zorunluluğu, `g_` ön eki, struct tag prefix)
  projenin teslim gereksinimine göre değişkenlik gösterebilir.
