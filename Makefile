# Makefile for Scream - GNU Screen session management tool

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99
LIBS = -lncurses

# Paths
C_SRC = scream.c
C_BIN = scream
PY_DIR = scream_py
INSTALL_DIR = /usr/bin

# Phony targets
.PHONY: all c python install install-c install-python clean help

# Default target
all: c python

# Build C implementation
c:
	$(CC) $(CFLAGS) -o $(C_BIN) $(C_SRC) $(LIBS)

# Build and install Python package
python:
	cd $(PY_DIR) && pip install -e . --user

# Install both implementations
install: install-c install-python

# Install C implementation
install-c: c
	@echo "Installing C binary to $(INSTALL_DIR)"
	@if [ -L $(INSTALL_DIR)/$(C_BIN) ]; then \
		sudo rm $(INSTALL_DIR)/$(C_BIN); \
	fi
	sudo ln -sf $(CURDIR)/$(C_BIN) $(INSTALL_DIR)/$(C_BIN)

# Install Python package
install-python: python
	@echo "Python package installed"

# Uninstall both implementations
uninstall: uninstall-c uninstall-python

# Uninstall C implementation
uninstall-c:
	@if [ -L $(INSTALL_DIR)/$(C_BIN) ]; then \
		sudo rm $(INSTALL_DIR)/$(C_BIN); \
		echo "Removed C binary from $(INSTALL_DIR)"; \
	else \
		echo "C binary not installed at $(INSTALL_DIR)"; \
	fi

# Uninstall Python package
uninstall-python:
	pip uninstall -y scream-cli
	@echo "Python package uninstalled"

# Clean build artifacts
clean:
	rm -f $(C_BIN)
	rm -rf $(PY_DIR)/build/
	rm -rf $(PY_DIR)/dist/
	rm -rf $(PY_DIR)/*.egg-info/
	find . -name "__pycache__" -type d -exec rm -rf {} +
	find . -name "*.pyc" -delete

# Development reinstall for Python package
dev-reinstall:
	cd $(PY_DIR) && pip install -e . --user --force-reinstall

# Help target
help:
	@echo "Scream - GNU Screen session management tool"
	@echo ""
	@echo "Available targets:"
	@echo "  all            : Build both C and Python implementations (default)"
	@echo "  c              : Build only the C implementation"
	@echo "  python         : Build only the Python implementation"
	@echo "  install        : Install both implementations"
	@echo "  install-c      : Install only the C implementation"
	@echo "  install-python : Install only the Python implementation"
	@echo "  uninstall      : Uninstall both implementations"
	@echo "  uninstall-c    : Uninstall only the C implementation"
	@echo "  uninstall-python : Uninstall only the Python implementation"
	@echo "  clean          : Remove build artifacts"
	@echo "  dev-reinstall  : Reinstall Python package in development mode"
	@echo "  help           : Display this help message"