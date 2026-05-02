# 04 — Docker Standards

> ft_ping projesinde Docker, geliştirme ve test ortamı izolasyonu için
> kullanılır. Aşağıdaki kurallar **`docker/Dockerfile`** ve
> **`docker/docker-compose.yml`** için referanstır.

---

## DK-01: Image Selection

- **Base image** mümkünse **specific tag** ile sabitlenmelidir
  (`ubuntu:22.04` gibi). `latest` tag’ı production için **yasak**;
  geliştirme için kullanılabilir fakat değerlendirme öncesi sabitlenmeli.
- Mümkünse **slim** veya **distroless** varyantları tercih edilmeli.
- Multi-stage build kullanılarak **build artifact** ile **runtime image**
  ayrılmalıdır.

---

## DK-02: Layer Optimization

- `RUN apt-get update && apt-get install -y ...` **tek** RUN içinde
  yapılmalı; layer sayısı azaltılmalıdır.
- `apt-get install` sonrası `rm -rf /var/lib/apt/lists/*` ile cache
  temizlenmeli.
- `--no-install-recommends` flag’i kullanılarak gereksiz paketler
  yüklenmemelidir.
- COPY/ADD instruction’ları, sık değişen dosyalar **en sonda** olacak
  şekilde sıralanmalıdır (cache invalidation’ı azaltır).

---

## DK-03: Security

- Image içinde **root** kullanıcı ile çalışmak production için yasak;
  ancak ft_ping **raw socket** kullandığı için `CAP_NET_RAW` capability
  gerektirir. Bu nedenle:
  - Either: container `--cap-add=NET_RAW` ile başlatılmalı
  - Or: `setcap cap_net_raw+ep` binary üzerine uygulanmalı
- **Hardcoded password** (örn. `root:password`) yalnızca **lokal
  geliştirme** içindir; production’a çıkmamalıdır.
- `EXPOSE 22` (SSH) yalnızca debug amaçlı olmalı; final image’da
  kapatılmalıdır.
- Secret’lar (key, token) **build args** veya **runtime env vars** ile
  yönetilmeli; image’a embed edilmemelidir.

---

## DK-04: Network

- Custom **bridge network** tanımlanmalıdır (default `bridge` yerine).
- Container’ın `cap_add: ["NET_RAW", "NET_ADMIN"]` ihtiyacı
  `docker-compose.yml` içinde açıkça belirtilmelidir.
- Host network mode (`network_mode: host`) yalnızca **gerekli ise**
  kullanılmalı; aksi halde izolasyon kaybolur.

---

## DK-05: Volume & Data

- Geliştirme sırasında **bind mount** (`.:/usr/src/ft_ping`) ile kaynak
  kod canlı bağlanır.
- Production / değerlendirme image’ında bind mount kullanılmaz; kaynak
  `COPY` ile image içine alınır.
- `WORKDIR` her zaman **absolute path** olmalıdır.

---

## DK-06: Entry & CMD

- `CMD` ile `ENTRYPOINT` ayrımı netleştirilmeli:
  - `ENTRYPOINT`: container’ın **ana programı**
  - `CMD`: ana programa **default argümanlar**
- `tail -f /dev/null` gibi **pseudo-keep-alive** komutları yalnızca
  geliştirme içindir; üretimde **gerçek** servis çalıştırılır.
- Signal handling için **exec form** (`CMD ["bash", "-c", "..."]`)
  tercih edilmelidir.

---

## DK-07: docker-compose.yml

- `version` alanı, kullanılan Compose Specification’a uygun olmalıdır
  (modern Compose’da `version` opsiyoneldir).
- `container_name` sabitlenmeli (dev için), production’da bırakılmalı
  (Compose otomatik isimlendirir).
- `restart` policy belirtilmelidir (`unless-stopped` yaygın).
- `healthcheck` eklenmeli; container’ın **gerçekten** ayakta olduğu
  doğrulanmalıdır.

---

## DK-08: ft_ping Spesifik Konfigürasyon

ft_ping, **ICMP raw socket** açtığı için container içinde özel izinler
gerektirir:

```yaml
services:
  ft_ping:
    cap_add:
      - NET_RAW
      - NET_ADMIN
    sysctls:
      - net.ipv4.ping_group_range=0 2147483647
```

| Gereksinim                    | Açıklama                                  |
|-------------------------------|-------------------------------------------|
| `CAP_NET_RAW`                 | Raw socket open için                      |
| `CAP_NET_ADMIN`               | Routing table sorgusu için (opsiyonel)    |
| `iputils-ping`                | Referans karşılaştırma için               |
| `iproute2`                    | `ip route`, `ip link` komutları için      |
| `build-essential`             | `gcc`, `make`                             |

---

## DK-09: .dockerignore

Aşağıdaki içeriklerin image’a girmemesi için `.dockerignore` dosyası
**zorunludur**:

```
.git
.gitignore
docs/
output/
obj/
*.md
*.log
docker-compose.override.yml
```

---

## DK-10: Image Boyut Hedefleri

- Geliştirme image’ı: **< 500 MB**
- Final / değerlendirme image’ı: **< 200 MB**
- Image katman sayısı: **< 15**
- `docker history <image>` ile düzenli kontrol önerilir.
