# =====================================================================
# Main Makefile for PSIS Project Part B
# =====================================================================
# Dependencies:
#   sudo apt update
#   sudo apt install build-essential libsdl2-dev libzmq3-dev libconfig++-dev \
#     protobuf-c-compiler libprotobuf-c-dev pkg-config
#
# On macOS: brew install sdl2 protobuf-c libconfig zmq

.PHONY: all clean help shared new-universe-server new-trash-ship-client dashboard

# Default target
all: shared new-universe-server new-trash-ship-client

# Build shared library components (Communication, display, universe_data, protobuf)
shared:
	@echo "Building shared components..."
	$(MAKE) -C shared

# Build universe server
new-universe-server: shared
	@echo "Building new-universe-server..."
	$(MAKE) -C new-universe-server

# Build new-trash-ship-client
new-trash-ship-client: shared
	@echo "Building new-trash-ship-client..."
	$(MAKE) -C new-trash-ship-client

# Build dashboard (when implemented)
dashboard: shared
	@echo "Building dashboard..."
	$(MAKE) -C Dashboard

# Clean all build artifacts
clean:
	@echo "Cleaning all components..."
	$(MAKE) -C shared clean
	$(MAKE) -C new-universe-server clean
	$(MAKE) -C new-trash-ship-client clean
	$(MAKE) -C Dashboard clean 2>/dev/null || true
	$(MAKE) -C libconfig clean

# Display help information
help:
	@echo "=========================================="
	@echo "Makefile targets:"
	@echo "=========================================="
	@echo "  make all              - Build all components (default)"
	@echo "  make shared           - Build only shared components"
	@echo "  make new-universe-server  - Build universe server"
	@echo "  make new-trash-ship-client - Build trash-ship client"
	@echo "  make dashboard        - Build dashboard"
	@echo "  make clean            - Clean all build artifacts"
	@echo "  make help             - Show this help message"
	@echo "=========================================="
