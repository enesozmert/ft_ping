# macOS — Build & Run

> ft_ping macOS (Darwin) üzerinde **kısmen** uyumludur. macOS, BSD
> stack’ini kullandığından çoğu syscall ve header (`<sys/socket.h>`,
> `<netinet/ip_icmp.h>`) mevcuttur; ancak bazı detaylar Linux’tan
> ayrılır:
>
> - Linux: `IPPROTO_ICMP` ile `SOCK_RAW` root/`CAP_NET_RAW` ister.
> - macOS: aynı socket için **root** veya `SOCK_DGRAM` (ICMP echo)
>   varyantı kullanılır.
> - `IP_HDRINCL` davranışı farklılık gösterir.
> - `setcap` **yoktur**; ya `sudo` ya da `chmod u+s` kullanılır.

---

## 1. Bağımlılıklar

```bash
# Xcode Command Line Tools (clang + make)
xcode-select --install

# Homebrew (önerilir)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# GCC (Apple clang yerine GNU GCC ile derlemek istersen)
brew install gcc make

# Norminette
brew install python
pip3 install norminette
```

---

## 2. Compiler Seçimi

macOS’ta `gcc` aslında **Apple clang**’a sembolik link’tir. Subject
"GCC" demese de "C ile yazılacak ve Makefile sunulacak" der; ikisi de
geçerlidir.

```bash
# Default (Apple clang)
gcc --version       # → Apple clang

# GNU GCC (Homebrew ile)
gcc-13 --version    # → GNU GCC

# Makefile içinde override:
make CC=gcc-13
```

---

## 3. Build

```bash
make
# veya
make CC=clang
make CC=gcc-13
```

---

## 4. Çalıştırma

```bash
# Sudo ile (en pratik)
sudo ./output/ft_ping google.com
sudo ./output/ft_ping -v 8.8.8.8

# setuid bit (kalıcı, ama güvenlik açısından geliştirme dışı)
sudo chown root:wheel output/ft_ping
sudo chmod u+s output/ft_ping
./output/ft_ping google.com
```

> Subject `setuid` istemez; geliştirme sırasında `sudo` veya `make run
> SUDO=1` yeterlidir.

---

## 5. macOS-Spesifik Notlar

### 5.1 ICMP Header Tipleri

Linux: `<netinet/ip_icmp.h>` → `struct icmphdr`
macOS:  `<netinet/ip_icmp.h>` → `struct icmp` (BSD style)

Cross-platform için kendi header struct’ınızı tanımlamak (POSIX-portable)
en güvenli yoldur. ft_ping `hdr/` altında bunu yapıyorsa, macOS build’i
sorunsuz olur.

### 5.2 `IP_HDRINCL`

Linux’ta `IP_HDRINCL` raw socket için IP header’ı kullanıcının yazdığını
söyler. macOS’ta default `IP_HDRINCL=0`’dır ve kernel header’ı
ekler. ft_ping kernel’ın IP header’ı eklediğine güvenmelidir (subject
zaten bunu zorlamaz).

### 5.3 Capability

macOS’ta `setcap` yoktur. **Sudo zorunlu**dur.

### 5.4 Kernel Network Behaviour

macOS’ta yerel firewall (PF) outbound ICMP’yi default izin verir; ancak
`Little Snitch` veya `LuLu` gibi araçlar kuruluysa onay gerekebilir.

---

## 6. Test

```bash
# Sistem ping ile karşılaştırma
ping -c 4 google.com           # macOS BSD ping (ft_ping referansı: GNU)
sudo ./output/ft_ping google.com

# Apple ping (BSD) ile inetutils farkı vardır:
# - inetutils format → ft_ping referansı (Linux)
# - BSD ping format → macOS native
# Subject 'inetutils-2.0' ile uyumlu istediği için, macOS'ta birebir
# format eşlenmesi BSD ping ile değil GNU ping (inetutils) ile yapılır.
```

`inetutils` Homebrew ile yüklenebilir:

```bash
brew install inetutils
gping --version    # gnu-prefix
sudo gping -c 4 google.com
```

---

## 7. Statik Analiz

```bash
# Norminette
norminette src/ hdr/

# Cppcheck
brew install cppcheck
cppcheck --enable=all --inconclusive --std=c11 \
         --force --quiet src/ 2> cppcheck.txt

# Address Sanitizer (Apple clang)
make CFLAGS="-Wall -Wextra -Werror -g -fsanitize=address -Ihdr"
```

---

## 8. Debug

```bash
# lldb (Apple debugger)
sudo lldb -- ./output/ft_ping google.com
(lldb) run
(lldb) bt          # backtrace
(lldb) quit

# Apple Instruments (memory profiling)
xcrun xctrace record --template "Leaks" --launch -- ./output/ft_ping google.com
```

> macOS’ta **Valgrind** Apple Silicon’da çalışmaz; Intel mac’lerde de
> sınırlıdır. AddressSanitizer + lldb + Instruments tercih edilir.

---

## 9. Önemli Sınırlamalar

| Konu                        | Durum                                            |
|-----------------------------|--------------------------------------------------|
| Build (clang/gcc)           | ✅ Tam uyum                                      |
| Run (sudo)                  | ✅                                               |
| `setcap`                    | ❌ Yok — sudo gerekli                            |
| Linux-spesifik `icmphdr`    | ⚠ POSIX-portable struct kullanın                |
| Valgrind                    | ❌ (Apple Silicon) / ⚠ (Intel)                   |
| Norminette                  | ✅ pip ile                                       |
| Sistem ping ile karşılaştırma | ⚠ BSD ping ≠ inetutils GNU ping                |
