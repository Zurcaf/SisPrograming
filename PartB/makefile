# =====================================================================
# Main Makefile for PSIS Project Part B
# =====================================================================
# Dependencies:
#   sudo apt update
#   sudo apt install build-essential libsdl2-dev libzmq3-dev libconfig++-dev \
#     protobuf-c-compiler libprotobuf-c-dev pkg-config
#
# On macOS: brew install sdl2 protobuf-c libconfig zmq

.PHONY: all clean help shared universe-server trash-ship-client dashboard

# Default target
all: shared universe-server trash-ship-client

# Build shared library components (Communication, display, universe_data, protobuf)
shared:
	@echo "Building shared components..."
	$(MAKE) -C shared

# Build universe server
universe-server: shared
	@echo "Building universe-server..."
	$(MAKE) -C Universe-server

# Build trash-ship-client
trash-ship-client: shared
	@echo "Building trash-ship-client..."
	$(MAKE) -C Trash-ship-client

# Build dashboard (when implemented)
dashboard: shared
	@echo "Building dashboard..."
	$(MAKE) -C Dashboard

# Clean all build artifacts
clean:
	@echo "Cleaning all components..."
	$(MAKE) -C shared clean
	$(MAKE) -C Universe-server clean
	$(MAKE) -C Trash-ship-client clean
	$(MAKE) -C Dashboard clean 2>/dev/null || true
	$(MAKE) -C libconfig clean

# Display help information
help:
	@echo "=========================================="
	@echo "Makefile targets:"
	@echo "=========================================="
	@echo "  make all              - Build all components (default)"
	@echo "  make shared           - Build only shared components"
	@echo "  make universe-server  - Build universe server"
	@echo "  make trash-ship-client - Build trash-ship client"
	@echo "  make dashboard        - Build dashboard"
	@echo "  make clean            - Clean all build artifacts"
	@echo "  make help             - Show this help message"
	@echo "=========================================="
