# Linux — Build & Run

> Hedef: Native Linux ortamında (Debian 7.0+ veya Ubuntu 22.04 önerilir)
> ft_ping’in derlenmesi ve çalıştırılması.

---

## 1. Bağımlılıklar (Dependencies)

```bash
# Debian / Ubuntu
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    gcc \
    make \
    iputils-ping \
    iproute2 \
    libc6-dev

# Fedora / RHEL
sudo dnf install -y gcc make glibc-devel iputils iproute

# Arch
sudo pacman -S --needed base-devel iputils iproute2
```

---

## 2. Build (Make)

```bash
# Tam build
make

# Yeniden build (clean + build)
make re

# Sadece object dosyaları temizle
make clean

# Tüm artifact'ları temizle
make fclean
```

Çıktı: `output/ft_ping`

---

## 3. Çalıştırma — Capability Yöntemi (Önerilen)

Raw socket için `CAP_NET_RAW` capability **bir kez** binary’ye uygulanır:

```bash
sudo setcap cap_net_raw+ep output/ft_ping

# Sonrasında sudo gerekmez:
./output/ft_ping google.com
./output/ft_ping 8.8.8.8
./output/ft_ping -v 1.1.1.1
./output/ft_ping -?
```

`re` veya `make` her çağrıldığında binary yeniden oluşacağı için
`setcap` da tekrar uygulanmalıdır.

---

## 4. Çalıştırma — Sudo Yöntemi

```bash
sudo ./output/ft_ping google.com
sudo ./output/ft_ping -v 8.8.8.8

# Makefile shortcut
make run SUDO=1 ARGS="google.com"
make run SUDO=1 ARGS="-v 8.8.8.8"
```

---

## 5. Test Senaryoları

```bash
# Mandatory: IPv4 doğrudan adres
./output/ft_ping 8.8.8.8

# Mandatory: hostname
./output/ft_ping google.com

# Mandatory: FQDN
./output/ft_ping www.example.com

# Mandatory: -v verbose
./output/ft_ping -v 8.8.8.8

# Mandatory: -? help
./output/ft_ping -?

# TTL hatası testi (verbose ile)
./output/ft_ping -v --ttl=1 8.8.8.8

# Unknown host
./output/ft_ping nonexistent.invalid.tld

# Ctrl+C (SIGINT) ile statistics flush testi
./output/ft_ping 8.8.8.8
# birkaç ping sonrası Ctrl+C
```

---

## 6. Reference (`inetutils-2.0`) ile Karşılaştırma

```bash
# Sistem ping çıktısı
ping -V                                  # versiyon kontrolü
ping -c 4 google.com > /tmp/ref.txt
./output/ft_ping google.com > /tmp/our.txt
# birkaç paket sonra Ctrl+C

# Indentation karşılaştırması (RTT ve reverse DNS hariç)
diff -u /tmp/ref.txt /tmp/our.txt
```

---

## 7. Debug & Analiz Araçları

```bash
# Memory leak / segfault tespiti
sudo valgrind --leak-check=full \
              --show-leak-kinds=all \
              --track-origins=yes \
              ./output/ft_ping 8.8.8.8

# Address sanitizer build
make CFLAGS="-Wall -Wextra -Werror -g -fsanitize=address -Ihdr"

# Strace ile syscall takibi
sudo strace -e trace=network ./output/ft_ping 8.8.8.8

# tcpdump ile paket gözlemleme (başka terminalde)
sudo tcpdump -n -i any icmp

# Wireshark — GUI alternatif
sudo wireshark
```

---

## 8. Statik Analiz

```bash
# Norminette
norminette src/ hdr/

# Cppcheck
cppcheck --enable=all --inconclusive --std=c11 \
         --force --quiet src/ 2> cppcheck.txt

# MISRA (cppcheck addon)
cppcheck --dump --std=c11 src/*.c
python3 /usr/share/cppcheck/addons/misra.py src/*.dump > misra.txt
```

---

## 9. Linux Kernel Sürüm Kontrolü

Subject (R-04): `Linux kernel > 3.14` zorunlu.

```bash
uname -r
```

Çıktı `3.14`’ten yüksek olmalıdır (Debian 7 + güncellenmiş kernel veya
herhangi bir modern dağıtım).
