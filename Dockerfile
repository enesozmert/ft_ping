# Use a minimal Ubuntu image
FROM ubuntu:latest

# Install essential packages
RUN apt-get update && \
    apt-get install -y build-essential iputils-ping iproute2 openssh-server && \
    rm -rf /var/lib/apt/lists/*

# Configure SSH
RUN mkdir /var/run/sshd && \
    echo 'root:password' | chpasswd && \
    sed -i 's/#PermitRootLogin prohibit-password/PermitRootLogin yes/' /etc/ssh/sshd_config && \
    sed -i 's/#PasswordAuthentication yes/PasswordAuthentication yes/' /etc/ssh/sshd_config

# Expose SSH port
EXPOSE 22

# Set working directory
WORKDIR /usr/src/ft_ping

# Copy project files
COPY . .

# Start SSH service and keep the container running
CMD service ssh start && tail -f /dev/null
