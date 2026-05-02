# Rules — ft_ping Project Rule Set

> Bu klasör, ft_ping projesinin uyması gereken **tüm kural setlerini**
> tek tek dosyalar halinde toplar. Her dosya bağımsız olarak okunabilir
> ve denetim sırasında ilgili checklist olarak kullanılır.

---

## Dosya Listesi

| #  | Dosya                                                  | Kapsam                                          |
|----|--------------------------------------------------------|-------------------------------------------------|
| 01 | [01-subject-rules.md](01-subject-rules.md)             | Subject PDF (Ecole 42 ft_ping v5.1) zorunlulukları |
| 02 | [02-norminette.md](02-norminette.md)                   | Ecole 42 Norminette (Norm v2.0.2)                |
| 03 | [03-misra-c2012.md](03-misra-c2012.md)                 | MISRA C:2012 standart kuralları                  |
| 04 | [04-docker-standards.md](04-docker-standards.md)       | Docker / docker-compose iyi pratikleri           |
| 05 | [05-makefile-rules.md](05-makefile-rules.md)           | Makefile zorunlulukları + iyi pratikleri         |
| 06 | [06-error-handling.md](06-error-handling.md)           | Error handling, signal, resource cleanup         |
| 07 | [07-c-best-practices.md](07-c-best-practices.md)       | Genel C iyi uygulamaları                         |
| 08 | [08-network-protocol.md](08-network-protocol.md)       | IPv4 + ICMP protokol kuralları (RFC 791/792)     |

---

## Öncelik Sırası

1. **Subject** — projenin kabulü için **vazgeçilmez**.
2. **Norminette** — defense öncesi otomatik araç ile geçilmek zorunda.
3. **Error handling** — segfault / leak yoksa kabul edilebilir.
4. **Network protocol** — protokol uyumu olmadan ping çalışmaz.
5. **Makefile** — değerlendiricinin build alabilmesi için.
6. **C best practices** — kod kalitesi için.
7. **MISRA C:2012** — opsiyonel; ekstra kalite katmanı.
8. **Docker** — geliştirme ortamı; değerlendirme dışı.

---

## Kullanım

- Yeni feature eklerken ilgili kural setini **gözden geçir**.
- Commit öncesi `make norm` ve `make cppcheck` çalıştır.
- PR açarken bu klasördeki ilgili checkbox’ları işaretle.
