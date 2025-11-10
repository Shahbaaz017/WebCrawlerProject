#!/bin/bash

# ==============================================================================
#  Web Crawler Project Environment Setup Script for WSL (Debian/Ubuntu) - V3
# ==============================================================================
# This script will:
# 1. Create the project directory structure.
# 2. Update apt and install all necessary system and Python dependencies.
# 3. Download the nlohmann/json header for the C++ project.
# ==============================================================================

# Exit immediately if a command exits with a non-zero status.
set -e

# Define some colors for output
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# --- 1. Create Project Directory Structure ---
echo -e "${YELLOW}Step 1: Creating project directory structure...${NC}"
mkdir -p WebCrawlerProject/python
mkdir -p WebCrawlerProject/java
mkdir -p WebCrawlerProject/cpp
mkdir -p WebCrawlerProject/c
mkdir -p WebCrawlerProject/javascript
echo -e "${GREEN}Directory structure created successfully.${NC}"

# --- 2. Install ALL System & Python Dependencies via APT ---
echo -e "\n${YELLOW}Step 2: Installing all dependencies (requires sudo password)...${NC}"
sudo apt-get update

# This is one single command, with each package on a new line for readability.
# The comment that caused the error has been removed.
sudo apt-get install -y \
    build-essential \
    pkg-config \
    default-jdk \
    nodejs \
    npm \
    python3-pip \
    python3-venv \
    libcurl4-openssl-dev \
    libxml2-dev \
    libgumbo-dev \
    libjansson-dev \
    python3-bs4 \
    python3-pandas \
    python3-psutil \
    python3-matplotlib \
    python3-seaborn \
    python3-tqdm

echo -e "${GREEN}All dependencies installed successfully.${NC}"

# --- 3. Download C++ nlohmann/json Header ---
echo -e "\n${YELLOW}Step 3: Downloading nlohmann/json header for C++...${NC}"
CPP_DIR="WebCrawlerProject/cpp"
JSON_HEADER_URL="https://github.com/nlohmann/json/releases/download/v3.11.2/json.hpp"
JSON_HEADER_PATH="$CPP_DIR/json.hpp"

if [ ! -f "$JSON_HEADER_PATH" ]; then
    wget -O "$JSON_HEADER_PATH" "$JSON_HEADER_URL"
    echo -e "${GREEN}nlohmann/json.hpp downloaded successfully.${NC}"
else
    echo -e "${GREEN}nlohmann/json.hpp already exists. Skipping download.${NC}"
fi

# --- Final Instructions ---
echo -e "\n======================================================================"
echo -e "${GREEN}✅ All Done! Your development environment is ready.${NC}"
echo -e "======================================================================"
echo -e "${YELLOW}Navigate into your project directory to get started:${NC}"
echo -e "  cd WebCrawlerProject"
echo -e "\n${YELLOW}Remember to run your local web server for testing.${NC}"
echo -e "======================================================================"