# Docker — Build & Run

> ft_ping projesi için **önerilen geliştirme ortamı**. Linux container
> içinde subject’in beklediği davranışın **birebir** yakalanmasını
> sağlar. Host: Linux / macOS / Windows fark etmez.
>
> Konfigürasyon dosyaları: [`docker/Dockerfile`](../../docker/Dockerfile),
> [`docker/docker-compose.yml`](../../docker/docker-compose.yml).

---

## 1. Ön Gereksinimler

- **Docker Engine** ≥ 24.0 (Linux) veya **Docker Desktop** ≥ 4.x
  (macOS / Windows).
- **Docker Compose** v2 (CLI plugin olarak gelir).

```bash
docker --version
docker compose version
```

---

## 2. Image Build

```bash
# Compose ile (önerilen)
docker compose -f docker/docker-compose.yml build

# Veya doğrudan docker
docker build -t ft_ping:latest -f docker/Dockerfile .
```

---

## 3. Container Başlat

```bash
# Detached mode
docker compose -f docker/docker-compose.yml up -d

# Foreground (logs ile)
docker compose -f docker/docker-compose.yml up

# Statü
docker ps --filter "name=ft_ping_container"
```

---

## 4. Container’a Bağlan

```bash
# Bash shell
docker exec -it ft_ping_container bash

# SSH (port 2222 host'ta forward edilmiş)
ssh root@localhost -p 2222
# password: password (yalnızca dev için!)
```

---

## 5. Container İçinde Build & Run

```bash
# Container'a girdikten sonra
cd /usr/src/ft_ping
make

# CAP_NET_RAW container'a verildiyse setcap çalışır:
setcap cap_net_raw+ep output/ft_ping
./output/ft_ping google.com

# Aksi halde root olarak çalıştırılır (container default user = root):
./output/ft_ping google.com
```

---

## 6. Capability Konfigürasyonu

ft_ping, ICMP raw socket açtığı için capability gerektirir.
`docker-compose.yml`’a ekleyin:

```yaml
services:
  ft_ping:
    build:
      context: ..
      dockerfile: docker/Dockerfile
    container_name: ft_ping_container
    cap_add:
      - NET_RAW
      - NET_ADMIN
    sysctls:
      - net.ipv4.ping_group_range=0 2147483647
    stdin_open: true
    tty: true
    volumes:
      - ..:/usr/src/ft_ping
    networks:
      - ft_ping_net

networks:
  ft_ping_net:
    driver: bridge
```

> Mevcut `docker-compose.yml` dosyasında `cap_add` **yok**. Subject
> defansif değerlendirmesinde sorun çıkarmaması için eklenmesi önerilir.

---

## 7. Test Senaryoları (Container İçinde)

```bash
# Mandatory testler
./output/ft_ping 8.8.8.8
./output/ft_ping google.com
./output/ft_ping -v 8.8.8.8
./output/ft_ping -?

# Sistem ping (referans) ile karşılaştırma
ping -c 4 google.com > /tmp/ref.txt
./output/ft_ping google.com > /tmp/our.txt
# Ctrl+C sonrası
diff -u /tmp/ref.txt /tmp/our.txt
```

---

## 8. Static Analiz (Container İçinde)

```bash
apt-get update && apt-get install -y cppcheck python3 valgrind

# Cppcheck
cppcheck --enable=all --inconclusive --std=c11 \
         --force --quiet src/ 2> cppcheck.txt

# Valgrind
valgrind --leak-check=full --show-leak-kinds=all \
         ./output/ft_ping google.com
```

---

## 9. Container Yaşam Döngüsü

```bash
# Durdur
docker compose -f docker/docker-compose.yml stop

# Başlat
docker compose -f docker/docker-compose.yml start

# Yeniden başlat
docker compose -f docker/docker-compose.yml restart

# Tamamen kaldır (volume ile)
docker compose -f docker/docker-compose.yml down -v

# Image'ı sil
docker rmi ft_ping:latest

# Tüm dangling image'ları temizle
docker image prune -f
```

---

## 10. Volume / Bind Mount Davranışı

`docker-compose.yml` içindeki:

```yaml
volumes:
  - .:/usr/src/ft_ping
```

Host’taki kaynak kodu container’a **canlı** bağlar. Host’ta yapılan
düzenleme container’da anında görünür; `make` container’da çalıştığında
yeni kod derlenir.

> ⚠️ Windows host’tan bind mount **performans olarak yavaştır**.
> Alternatif: kaynağı container’a **kopyalayın** (`COPY .` Dockerfile’da
> zaten var) ve volume satırını yorumlayın.

---

## 11. Network Mode

Compose default’ta `bridge` network kullanır; bu çoğu durumda yeterlidir.
Eğer container’dan `ping`’in **gerçek RTT**’sini ölçmek istiyorsanız ve
NAT aşamasından kaçınmak gerekiyorsa:

```yaml
network_mode: host  # Linux host'ta çalışır, Docker Desktop'ta değil
```

---

## 12. Sık Karşılaşılan Hatalar

| Hata                                              | Çözüm                                              |
|---------------------------------------------------|----------------------------------------------------|
| `socket: Operation not permitted`                 | `cap_add: [NET_RAW]` ekle                          |
| `bind: Cannot assign requested address`           | Container network’ünü kontrol et                   |
| `setcap: command not found`                       | `apt-get install -y libcap2-bin`                   |
| `make: command not found`                         | Dockerfile’da `build-essential` yüklenmiş mi?      |
| `Cannot connect to the Docker daemon`             | Docker Desktop / `systemctl start docker` çalışıyor mu? |
| Bind mount’ta dosya değişimi yansımıyor (Windows) | WSL2 backend kullan veya kaynağı container’a kopyala |
