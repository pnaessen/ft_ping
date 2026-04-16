# ft_ping

> A low-level network diagnostic tool written in C, replicating the behavior of the standard `ping` utility.

## 📖 Overview

This project is a deep dive into **low-level network programming** and the **OSI Model (Layer 3 & 4)**. It bypasses standard high-level network abstractions by utilizing **Raw Sockets** to manually forge, send, and receive network packets at the byte level.

## 🛠️ Key Technical Features

* **Raw Sockets (`SOCK_RAW`)**: Bypassing kernel defaults to interact directly with the network stack (requires `root` privileges).
* **Packet Forging**: Manual construction of **IPv4** and **ICMP (RFC 792)** headers.
* **Algorithmic Integrity**: Custom implementation of the Internet Checksum algorithm to ensure data integrity.
* **Network Byte Order**: Strict handling of Endianness (`htons`, `ntohs`) for cross-architecture compatibility.
* **DNS Resolution**: Forward and reverse DNS lookups handling.
* **Signal Handling**: Graceful interruption (SIGINT) with accurate statistics reporting (packet loss, min/avg/max latency).
