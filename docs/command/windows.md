# Windows — Build & Run

> ft_ping native Windows ile uyumlu **değildir**: raw ICMP socket için
> POSIX socket API ve `CAP_NET_RAW` benzeri ayrıcalık gerekir; Winsock
> raw socket bu projede **kullanılmaz**.
>
> Windows üzerinde geliştirme **3 yöntemle** yapılır:
> 1. **WSL2** (Windows Subsystem for Linux) — önerilen.
> 2. **Docker Desktop** — bkz. [docker.md](docker.md).
> 3. **MSYS2 / MinGW-w64** — yalnızca **build** içindir, çalıştırma için
>    Linux gerekir.

---

## Yöntem 1: WSL2 (Önerilen)

### 1.1 Kurulum

PowerShell (Administrator):

```powershell
# WSL2 + Ubuntu 22.04 kurulumu
wsl --install -d Ubuntu-22.04

# Mevcut WSL'leri listele
wsl --list --verbose

# Default sürümü WSL2 yap
wsl --set-default-version 2
```

Restart sonrası Ubuntu’yu başlatın ve user oluşturun.

### 1.2 Bağımlılıklar (WSL içinde)

```bash
sudo apt-get update
sudo apt-get install -y build-essential gcc make iputils-ping iproute2
```

### 1.3 Projeye Eriş

WSL içinden Windows dosyalarına erişim:

```bash
# Windows path: C:\Users\Enes Özmert\Desktop\Ecole42\ft_ping
cd "/mnt/c/Users/Enes Özmert/Desktop/Ecole42/ft_ping"
```

> ⚠️ Bind mount edilen `/mnt/c/...` üzerinde build **yavaştır** ve
> Linux file permission’ları taklit edilir. **Daha hızlı** geliştirme için
> projeyi WSL home’a kopyalayın:
>
> ```bash
> cp -r "/mnt/c/Users/Enes Özmert/Desktop/Ecole42/ft_ping" ~/ft_ping
> cd ~/ft_ping
> ```

### 1.4 Build & Run

```bash
make
sudo setcap cap_net_raw+ep output/ft_ping
./output/ft_ping google.com
```

WSL2 NAT mode default’tur; ICMP **çalışır**, ancak bazı hostlar Windows
firewall tarafından bloklanabilir.

---

## Yöntem 2: Docker Desktop

Bkz. [docker.md](docker.md).

PowerShell:

```powershell
docker compose -f docker/docker-compose.yml up -d --build
docker exec -it ft_ping_container bash
# container içinde:
make
./output/ft_ping google.com
```

---

## Yöntem 3: MSYS2 / MinGW-w64 (Yalnızca Build)

> ⚠️ Bu yöntemle build edilen binary **çalışmaz** (Winsock + raw socket
> uyumsuz). Yalnızca **syntax** ve **tip** kontrolü için kullanılır.

### 3.1 MSYS2 Kurulumu

https://www.msys2.org/ adresinden installer indirin.

```bash
# MSYS2 MINGW64 shell
pacman -Syu
pacman -S --needed mingw-w64-x86_64-toolchain make
```

### 3.2 Build (PowerShell)

```powershell
# msys2 shell üzerinden
& "C:\msys64\msys2_shell.cmd" -mingw64 -c "cd /c/Users/Enes\ Özmert/Desktop/Ecole42/ft_ping && make"
```

> Beklenen hata: `<sys/socket.h>`, `<netinet/ip_icmp.h>` bulunamayabilir.
> Bu, Windows’un POSIX socket header’larını taşımadığını gösterir; build
> başarısız olur. Bu **beklenen** davranıştır.

---

## PowerShell Yardımcı Komutlar

```powershell
# Proje root'a git
cd "C:\Users\Enes Özmert\Desktop\Ecole42\ft_ping"

# WSL içinde tek seferlik komut
wsl -d Ubuntu-22.04 -- bash -lc "cd /mnt/c/Users/Enes\ Özmert/Desktop/Ecole42/ft_ping && make"

# Docker container statüsü
docker ps --filter "name=ft_ping_container"

# Docker container'a komut gönder
docker exec ft_ping_container bash -lc "make && ./output/ft_ping -? "
```

---

## Windows Firewall ve ICMP

ft_ping container’dan veya WSL’den outbound ICMP gönderir; outbound
trafik Windows Firewall tarafından **default olarak engellenmez**. Eğer
engelleniyorsa:

```powershell
# ICMP echo outbound izin ver (Administrator)
New-NetFirewallRule -DisplayName "ICMP Out" `
    -Direction Outbound -Protocol ICMPv4 -Action Allow
```

---

## Önemli Sınırlamalar

| Konu                     | Durum                                                    |
|--------------------------|----------------------------------------------------------|
| Native Windows build     | ❌ POSIX socket header eksik                             |
| Native Windows run       | ❌ Raw socket API uyumsuz                                |
| WSL2 build               | ✅ Tam uyum                                              |
| WSL2 run                 | ✅ `setcap` ile tam uyum                                 |
| Docker Desktop build/run | ✅ Tam uyum (`CAP_NET_RAW` gerekli)                      |
| Norminette (Windows)     | ⚠ Python wrapper ile WSL üzerinden                      |
