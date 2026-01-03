# 🚀 Trash Ship Simulator

A distributed simulation system featuring a universe simulator and trash-collecting ships with client-server architecture using Protocol Buffers for communication.

## 📋 Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Architecture](#architecture)
- [Project Structure](#project-structure)
- [Prerequisites](#prerequisites)
- [Installation](#installation)
- [Usage](#usage)
- [Configuration](#configuration)
- [Building](#building)
- [Contributing](#contributing)

## 🌌 Overview

This project implements a distributed space simulation where multiple trash ships navigate through a universe, collecting debris. The system consists of:

- **Universe Simulator**: Server-side physics engine managing the environment
- **Trash Ship Simulator**: Client-side ship controllers that connect to the universe
- **Communication Layer**: Protocol Buffers-based messaging system for client-server interaction

The project is divided into two parts (PartA and PartB), likely representing different iterations or feature sets.

## ✨ Features

- **Real-time Physics Simulation**: Universe with physics rules and collision detection
- **Multi-client Support**: Multiple ships can connect and operate simultaneously
- **SDL2 Graphics**: Visual display of the universe and ship movements
- **Protocol Buffers**: Efficient binary serialization for network communication
- **Configuration System**: External configuration files using libconfig
- **Cross-platform**: Supports Linux and macOS (separate makefiles provided)

## 🏗️ Architecture

```
┌─────────────────┐         ┌─────────────────┐
│  Trash Ship     │◄───────►│   Universe      │
│  Simulator      │  ProtoBuf│   Simulator     │
│  (Client)       │         │   (Server)      │
└─────────────────┘         └─────────────────┘
        │                           │
        └───── SDL2 Display ────────┘
```

### Components

- **Universe Simulator**: Manages the game world, physics, and coordinates all ships
- **Trash Ship Simulator**: Individual ship client with movement controls and display
- **Communication Module**: Handles network messaging between clients and server
- **Display Module**: Renders the universe using SDL2
- **Physics Engine**: Applies physics rules to all entities

## 📁 Project Structure

```
SisPrograming/
├── PartA/                          # Part A implementation
│   ├── libconfig/                  # Configuration library
│   │   ├── config.c/h             # Config parser
│   │   ├── init.conf              # Configuration file
│   │   └── makefile               # Build config lib
│   ├── Trash-ship-simulator/      # Ship client
│   │   ├── head/                  # Header files
│   │   │   ├── Communication.h    # Network protocol
│   │   │   ├── display.h          # Graphics interface
│   │   │   ├── ship_movement.pb-c.h # ProtoBuf definitions
│   │   │   └── universe_data.h    # Universe structures
│   │   ├── src/                   # Source files
│   │   │   ├── Universe-client.c  # Main client program
│   │   │   ├── Universe-server.c  # Main server program
│   │   │   └── ...
│   │   ├── ship_movement.proto    # ProtoBuf schema
│   │   └── makefile              # Build script
│   └── Universe-simulator/        # Physics engine
│       ├── head/                  # Header files
│       ├── src/                   # Source files
│       └── makefile              # Build script
└── PartB/                          # Part B implementation
    └── (Similar structure)
```

## 🔧 Prerequisites

### Required Dependencies

- **GCC** or compatible C compiler
- **SDL2** library and development headers
- **SDL2_timer**
- **Protocol Buffers C** (protobuf-c)
- **libconfig** (included in project)
- **make** build tool

### Installing Dependencies

#### macOS (using Homebrew)

```bash
brew install sdl2 protobuf-c
```

#### Ubuntu/Debian

```bash
sudo apt-get update
sudo apt-get install libsdl2-dev protobuf-c-compiler libprotobuf-c-dev
```

#### Fedora

```bash
sudo dnf install SDL2-devel protobuf-c-devel protobuf-c-compiler
```

## 📦 Installation

1. **Clone the repository**

   ```bash
   git clone https://github.com/Zurcaf/SisPrograming.git
   cd SisPrograming
   ```

2. **Build libconfig** (for macOS use makefile.mac)

   ```bash
   cd PartA/libconfig
   make        # Linux
   # or
   make -f makefile.mac  # macOS
   cd ../..
   ```

3. **Generate Protocol Buffer files** (if not already generated)

   ```bash
   cd PartA/Trash-ship-simulator
   protoc-c --c_out=. ship_movement.proto
   ```

4. **Build the Universe Simulator**

   ```bash
   cd PartA/Universe-simulator
   make
   ```

5. **Build the Trash Ship Simulator**
   ```bash
   cd PartA/Trash-ship-simulator
   make
   ```

## 🚀 Usage

### Starting the Universe Server

```bash
cd PartA/Trash-ship-simulator
./universe-simulator
```

The server will start and wait for client connections.

### Connecting Ship Clients

In separate terminal windows:

```bash
cd PartA/Trash-ship-simulator
./trash-ship-simulator
```

When prompted, enter a unique character (a-z) to identify your ship. The ship will connect to the server at `localhost` and begin operation.

### Configuration

Edit the configuration file to customize simulation parameters:

```bash
vim PartA/libconfig/init.conf
```

## 🔨 Building

### Build All Components

```bash
# Build libconfig
cd PartA/libconfig && make && cd ../..

# Build Universe Simulator
cd PartA/Universe-simulator && make && cd ../..

# Build Trash Ship Simulator
cd PartA/Trash-ship-simulator && make && cd ../..
```

### Clean Build Files

```bash
make clean  # In each component directory
```

## 🎮 Controls

- Ships are controlled through keyboard input (implementation-specific)
- Universe display shows real-time positions and movements
- Multiple ships can operate simultaneously

## 🐛 Troubleshooting

### SDL2 Not Found

```bash
# Check SDL2 installation
sdl2-config --version

# If not found, reinstall SDL2 development packages
```

### Protocol Buffer Compilation Errors

```bash
# Regenerate .pb-c files
protoc-c --c_out=. ship_movement.proto
```

### Connection Issues

- Ensure the universe server is running before starting clients
- Check that the hostname/IP in the client matches the server location
- Verify firewall settings allow local connections

## 📝 License

This project is part of academic coursework for Systems Programming.

## 👥 Contributing



## 📧 Contact

- **Repository**: [Zurcaf/SisPrograming](https://github.com/Zurcaf/SisPrograming)
- **Author**: Afonso & Miguel

---

**Note**: This project demonstrates concepts in distributed systems, network programming, graphics rendering, and inter-process communication.
