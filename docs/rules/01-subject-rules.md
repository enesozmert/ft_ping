# 01 — Subject Rules (ft_ping v5.1)

> Kaynak: `docs/main/en.subject.pdf` (Ecole 42 — ft_ping, Version 5.1)
>
> Bu dosya, **subject** dosyasında yer alan kuralların birebir karşılığıdır.
> Hiçbir kural yorumlanmamıştır; sadece Türkçeye çevrilmiş ve teknik
> terimler İngilizce bırakılmıştır.

---

## R-01: General Instructions

- Proje **Debian (>= 7.0)** üzerinde çalışan bir **virtual machine** içinde
  geliştirilmelidir.
- Virtual machine, projeyi tamamlamak için gerekli tüm yazılımları
  içermelidir; bu yazılımlar **kurulu** ve **konfigüre edilmiş** olmalıdır.
- VM, **cluster computer** üzerinden kullanılabilir olmalıdır.
- Proje **yalnızca insanlar tarafından** değerlendirilecektir. Dosya/dizin
  isimlendirme serbesttir, fakat aşağıdaki kurallara uyulması zorunludur.
- Proje **C** ile yazılmalı ve bir **Makefile** sunulmalıdır.
- Makefile, projeyi derlemeli ve **olağan kuralları** (`all`, `clean`,
  `fclean`, `re`) içermelidir. Yalnızca **gerekliyse** yeniden derleme ve
  yeniden bağlama (re-link) yapmalıdır.
- **Hata yönetimi** dikkatli yapılmalıdır. Program hiçbir şekilde
  beklenmedik biçimde sonlanmamalıdır:
  - Segmentation fault — yasak
  - Bus error — yasak
  - Double free — yasak
- **libc fonksiyonları** kullanımına izin verilmiştir.

> ⚠️ **DİKKAT:** Program C dilinde yazılacaktır; tüm **libc** kullanımı
> serbesttir. **Sistem ping komutunu** veya **standart bir ping
> implementasyonunun kaynak kodlarını** herhangi bir şekilde kullanmak
> **yasaktır**.

---

## R-02: Mandatory Part

- Çalıştırılabilir dosyanın adı **`ft_ping`** olmalıdır.
- Referans implementasyon: **`inetutils-2.0`** (`ping -V`).
- Aşağıdaki opsiyonlar yönetilmelidir:
  - `-v` (verbose)
  - `-?` (help / usage)

> `-v` opsiyonu burada, paketlerle ilgili problem veya hata durumlarını
> da gösterir; bu durum mantıksal olarak programın durmasına **zorlamaz**
> (TTL değerini değiştirerek hata oluşturmak mümkündür).

- Parametre olarak basit bir **IPv4 (address/hostname)** yönetilmelidir.
- **FQDN** (Fully Qualified Domain Name) yönetilmelidir; ancak paket
  dönüşünde **DNS resolution** yapılmamalıdır.

> **printf** ailesinin tüm fonksiyonları kullanılabilir.
>
> Akıllı olduğunu sananlar (veya sanmayanlar) için... Açıkça belirtmek
> gerekirse: **gerçek bir `ping` çağırmak yasaktır.**

---

## R-03: Bonus Part

Aşağıdakiler ilginç bonus fikirleridir:

- Ek bayraklar: `-f`, `-l`, `-n`, `-w`, `-W`, `-p`, `-r`, `-s`, `-T`,
  `--ttl`, `--ip-timestamp`, ...

> `-V`, `--usage`, `--echo` bayrakları **bonus sayılmaz**.
>
> Aynı özelliği karşılayan iki bayrak (örneğin `-t` ve `--type`) **iki
> ayrı bonus** olarak değerlendirilmez.
>
> **Bonus part yalnızca mandatory part KUSURSUZ ise** değerlendirilir.
> Kusursuz, mandatory part’ın eksiksiz tamamlanması ve hatasız
> çalışması anlamına gelir. Tüm mandatory gereksinimler geçilmediyse
> bonus part **hiç değerlendirilmez**.

---

## R-04: Submission and Peer-Evaluation

- Teslim, projenin **Git repository**’si üzerinden yapılır. Yalnızca
  repository içindeki çalışma savunmada değerlendirilir.
- Klasör/dosya adlarının doğru olduğundan emin olunmalıdır.
- VM, **Linux kernel > 3.14** olmalıdır. Değerlendirme **Debian 7.0
  stable** üzerinde tasarlanmıştır.
- **RTT satırı** ve **reverse DNS resolution** dışındaki çıktının,
  `inetutils-2.0` implementasyonu ile **birebir aynı indentation**’a
  sahip olması gerekir.

> Paket alımında **±30 ms** gecikme tolere edilir.

---

## Subject Compliance Checklist

| ID    | Kural                                                                | Durum |
|-------|----------------------------------------------------------------------|-------|
| R-01a | C ile yazıldı                                                         | ☐     |
| R-01b | Makefile mevcut (`all`, `clean`, `fclean`, `re`)                      | ☐     |
| R-01c | Yalnızca gerekliyse re-compile / re-link                              | ☐     |
| R-01d | Segfault / bus error / double free yok                                | ☐     |
| R-01e | Sadece libc kullanılıyor (sistem ping yok)                            | ☐     |
| R-02a | Executable adı `ft_ping`                                              | ☐     |
| R-02b | `-v` opsiyonu                                                         | ☐     |
| R-02c | `-?` opsiyonu                                                         | ☐     |
| R-02d | IPv4 address/hostname parametresi                                     | ☐     |
| R-02e | FQDN desteği (paket dönüşünde DNS resolution yok)                     | ☐     |
| R-04a | `inetutils-2.0` ile aynı indentation (RTT ve reverse DNS hariç)       | ☐     |
| R-04b | ±30 ms tolerans                                                       | ☐     |
