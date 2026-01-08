# =====================================================================
# Makefile for Dashboard (Python)
# =====================================================================
# Dashboard is a Python application that connects to the server via ZMQ
# and displays universe statistics

PYTHON := python3
VENV_DIR := venv
DASHBOARD_SCRIPT := dashboard.py

# Default target
all: install

# Install Python dependencies
install:
	@echo "Installing Python dependencies..."
	$(PYTHON) -m pip install -q -r requirements.txt
	@echo "Dashboard ready to run"

# Run the dashboard (connect to localhost:5555)
run: install
	@echo "Starting dashboard..."
	$(PYTHON) $(DASHBOARD_SCRIPT)

# Run with custom server address
run-remote:
	@echo "Starting dashboard (connecting to remote server)..."
	$(PYTHON) $(DASHBOARD_SCRIPT) 127.0.0.1 5555

# Create Python virtual environment (optional)
venv:
	$(PYTHON) -m venv $(VENV_DIR)
	@echo "Virtual environment created. Activate with: source $(VENV_DIR)/bin/activate"

# Clean up
clean:
	@echo "Cleaning up Python cache..."
	find . -type d -name __pycache__ -exec rm -rf {} + 2>/dev/null || true
	find . -type f -name "*.pyc" -delete
	@echo "Cleanup complete"

# Help
help:
	@echo "Dashboard Makefile targets:"
	@echo "  make install      - Install Python dependencies"
	@echo "  make run          - Run dashboard (connect to localhost:5555)"
	@echo "  make run-remote   - Run dashboard with custom server config"
	@echo "  make venv         - Create Python virtual environment"
	@echo "  make clean        - Remove Python cache files"
	@echo "  make help         - Show this help message"

.PHONY: all install run run-remote venv clean help
