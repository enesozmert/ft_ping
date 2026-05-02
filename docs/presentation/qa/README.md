# Q&A — Defense Sıkça Sorulan Sorular (Detaylı)

> Sunum slayt 20'deki Q&A maddelerinin **derinlemesine** açıklamaları.
> Her dosya tek bir soru üzerine odaklanır; bizim koddan örnekler, RFC
> referansları, alternatif yaklaşımlar ve takip soruları içerir.

---

## Soru Listesi

| # | Soru                                                  | Dosya                                                        |
|---|-------------------------------------------------------|--------------------------------------------------------------|
| 1 | Raw socket nedir, niçin gerekiyor?                    | [01-raw-socket.md](01-raw-socket.md)                         |
| 2 | Checksum nasıl hesaplanıyor? (RFC 1071)               | [02-checksum.md](02-checksum.md)                             |
| 3 | RTT nasıl hesaplanıyor?                               | [03-rtt-calculation.md](03-rtt-calculation.md)               |
| 4 | Niçin `getaddrinfo`, `gethostbyname` değil?           | [04-getaddrinfo-vs-gethostbyname.md](04-getaddrinfo-vs-gethostbyname.md) |
| 5 | DNS resolution paket dönüşünde niçin yapılmıyor?      | [05-dns-resolution-policy.md](05-dns-resolution-policy.md)   |

---

## Her Dosyanın Şablonu

- **Kısa Cevap** — savunmada 30 saniyede söylenebilecek özet.
- **Detaylı Açıklama** — teknik kavramlar (English term'ler), bağlam,
  trade-off'lar.
- **Kod Alıntıları** — ft_ping kaynak dosyalarından doğrudan satırlar
  (markdown link ile dosyaya yönlendirme).
- **Karşılaştırma Tabloları** — alternatif yaklaşımlar, neyin niçin
  seçildiği.
- **Subject Bağlamı** — Ecole 42 ft_ping v5.1 subject'inin ilgili
  maddesi.
- **Olası Follow-up'lar** — değerlendiricinin sorabileceği derinlemesine
  sorular ve hazır cevapları.
- **Referanslar** — RFC, POSIX, man page, kaynak kod.

---

## Bağlantılı Dokümanlar

- Sunum ana dosyası: [`presentation.md`](../presentation.md)
- Subject PDF: [`docs/main/en.subject.pdf`](../../main/en.subject.pdf)
- Subject kuralları: [`docs/rules/01-subject-rules.md`](../../rules/01-subject-rules.md)
- Network protocol kuralları: [`docs/rules/08-network-protocol.md`](../../rules/08-network-protocol.md)
- Proje raporu: [`docs/report/report.md`](../../report/report.md)

---

## Defense Hazırlığı için Öneriler

1. Her Q&A dosyasını **hızlıca özet** olarak çıkar — savunmada uzun
   anlatım yerine 2-3 cümle özet.
2. **Kod referanslarını** kafanda tut — dosya adı + fonksiyon ismi
   yetir, satır numarası gerekirse "src/checksum.c'de checksum
   fonksiyonu" gibi belirsiz değil, **kesin** referans ver.
3. **RFC numaralarını** ezberle (792, 1071, 791, 2553); değerlendirici
   "hangi RFC?" diye sorabilir.
4. **Alternatif yaklaşımları** bil — niçin `IP_HDRINCL` yok, niçin
   `clock_gettime` yerine `gettimeofday`, niçin `gethostbyname` değil.
5. **Sınırlarını** kabul et — IPv6 yok, reverse DNS yok, BSD'de native
   test yok. Subject tarafından yasaklı veya scope dışı olduğunu
   söyle.
