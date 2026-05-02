# =============================================================================
# ft_ping — Dockerfile (multi-stage)
# Subject: Ecole 42 ft_ping v5.1
#
# Targets:
#   builder   — full toolchain (gcc, make, cmake, norminette, cppcheck, valgrind)
#   runtime   — slim image (libc + iputils + libcap), production sized
#   dev       — full dev image (default), 900+ MB, all tools available
#
# Build:
#   docker build -t ft_ping:dev      --target dev     -f docker/Dockerfile .
#   docker build -t ft_ping:slim     --target runtime -f docker/Dockerfile .
# =============================================================================

# -----------------------------------------------------------------------------
# Stage 1: builder — compile binary
# -----------------------------------------------------------------------------
FROM ubuntu:22.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential \
        gcc \
        make \
        libcap2-bin \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /build
COPY Makefile ./
COPY src ./src
COPY hdr ./hdr

RUN make all && \
    setcap cap_net_raw+ep output/ft_ping || true

# -----------------------------------------------------------------------------
# Stage 2: runtime — slim production image (target < 100 MB)
# -----------------------------------------------------------------------------
FROM ubuntu:22.04 AS runtime

ENV DEBIAN_FRONTEND=noninteractive \
    LANG=C.UTF-8 \
    LC_ALL=C.UTF-8

RUN apt-get update && apt-get install -y --no-install-recommends \
        iputils-ping \
        libcap2-bin \
        ca-certificates \
    && rm -rf /var/lib/apt/lists/*

COPY --from=builder /build/output/ft_ping /usr/local/bin/ft_ping
RUN setcap cap_net_raw+ep /usr/local/bin/ft_ping

WORKDIR /work
ENTRYPOINT ["/usr/local/bin/ft_ping"]
CMD ["-?"]

# -----------------------------------------------------------------------------
# Stage 3: dev — full toolchain (default target for compose)
# -----------------------------------------------------------------------------
FROM ubuntu:22.04 AS dev

ENV DEBIAN_FRONTEND=noninteractive \
    LANG=C.UTF-8 \
    LC_ALL=C.UTF-8

RUN apt-get update && apt-get install -y --no-install-recommends \
        # Build chain
        build-essential \
        gcc \
        make \
        cmake \
        ninja-build \
        pkg-config \
        # ICMP / network reference
        iputils-ping \
        iproute2 \
        net-tools \
        tcpdump \
        # Capability tooling
        libcap2-bin \
        # Static / dynamic analysis
        cppcheck \
        valgrind \
        # Norminette (Python-based)
        python3 \
        python3-pip \
        python3-setuptools \
        # Misc dev utilities
        ca-certificates \
        curl \
        git \
        sudo \
        openssh-server \
    && rm -rf /var/lib/apt/lists/*

# Norminette (PEP 668 fallback)
RUN pip3 install --no-cache-dir --break-system-packages norminette || \
    pip3 install --no-cache-dir norminette

# SSH (development only — credentials NOT for production)
RUN mkdir -p /var/run/sshd && \
    echo 'root:password' | chpasswd && \
    sed -i 's/#PermitRootLogin prohibit-password/PermitRootLogin yes/' /etc/ssh/sshd_config && \
    sed -i 's/#PasswordAuthentication yes/PasswordAuthentication yes/' /etc/ssh/sshd_config

EXPOSE 22

WORKDIR /usr/src/ft_ping
COPY . .

RUN if [ -f Makefile ] && [ -d src ]; then \
        make all || true; \
        if [ -f output/ft_ping ]; then \
            setcap cap_net_raw+ep output/ft_ping || true; \
        fi; \
    fi

# Sanity check (non-fatal)
RUN echo "==> ft_ping dev tooling" && \
    gcc --version | head -1 && \
    cmake --version | head -1 && \
    cppcheck --version && \
    valgrind --version && \
    (norminette --version 2>/dev/null || echo "norminette: pip-installed")

CMD ["bash", "-lc", "service ssh start && tail -f /dev/null"]
