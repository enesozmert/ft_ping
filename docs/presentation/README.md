# Presentation — Sunum Klasörü

Bu klasör, ft_ping projesinin defense / peer-evaluation / topluluk
sunumu için hazırlanmış slayt formatında dokümanları içerir.

| Dosya                                  | Kapsam                              |
|----------------------------------------|-------------------------------------|
| [presentation.md](presentation.md)     | Ana sunum (21 slayt) — Türkçe + EN  |

## Kullanım

- `presentation.md` markdown formatında yazılmıştır; `---` her slayt
  ayracıdır.
- **Marp**, **Reveal.js**, **Pandoc** gibi araçlarla slayt-deck olarak
  render edilebilir:

```bash
# Marp ile
marp presentation.md -o presentation.pdf
marp presentation.md -o presentation.html

# Pandoc + Reveal.js ile
pandoc -t revealjs -s presentation.md -o presentation.html

# Sade markdown render
pandoc presentation.md -o presentation.pdf
```

- Defense sırasında doğrudan **markdown viewer** ile de kullanılabilir
  (VSCode preview, Typora, Obsidian).
