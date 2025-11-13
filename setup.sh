#!/bin/bash

# A script to set up the directory structure and initial files for the
# "Multicore Architecture Web Scraping Performance" project.

echo "--- Starting Project Setup ---"

# 1. Create the main project directory
PROJECT_ROOT="web_scraping_performance"
if [ -d "$PROJECT_ROOT" ]; then
    echo "Directory '$PROJECT_ROOT' already exists. Skipping creation."
else
    mkdir $PROJECT_ROOT
    echo "Created root directory: $PROJECT_ROOT"
fi
cd $PROJECT_ROOT

# 2. Create directories for each language and their respective models
echo "Creating language and model directories..."

# --- C ---
mkdir -p c/single_threaded
mkdir -p c/pthreads
mkdir -p c/openmp
touch c/single_threaded/scraper.c
touch c/pthreads/scraper.c
touch c/openmp/scraper.c

# --- C++ ---
mkdir -p cpp/single_threaded
mkdir -p cpp/pthreads
mkdir -p cpp/openmp
touch cpp/single_threaded/scraper.cpp
touch cpp/pthreads/scraper.cpp
touch cpp/openmp/scraper.cpp

# --- Java ---
mkdir -p java/single_threaded
mkdir -p java/multi_threaded
touch java/single_threaded/Scraper.java
touch java/multi_threaded/Scraper.java

# --- Python ---
mkdir -p python/single_threaded
mkdir -p python/multi_threaded
touch python/single_threaded/scraper.py
touch python/multi_threaded/scraper.py
touch python/multi_threaded/requirements.txt # For libraries like 'requests'

echo "Directory structure and empty files created successfully."

# 3. Create helper scripts and documentation
echo "Creating helper files..."
touch run_experiments.sh
chmod +x run_experiments.sh # Make it executable

# Create a README file with a basic template
cat > README.md <<- EOM
# Performance Analysis of Single vs. Multi-threaded Web Scraping

This project compares the performance of web scraping using different languages and concurrency models.

## Languages and Models
- **C:** Single-Threaded, Pthreads, OpenMP
- **C++:** Single-Threaded, Pthreads, OpenMP
- **Java:** Single-Threaded, Multi-Threaded (ExecutorService)
- **Python:** Single-Threaded, Multi-Threaded (concurrent.futures)

## How to Run
1. Install all dependencies.
2. Compile the source code for C, C++, and Java.
3. Use the \`run_experiments.sh\` script to execute all tests.
EOM

echo "--- Project Structure ---"
# Display the created structure
ls -R

echo ""
echo "--- Installation and Dependencies ---"
echo "To compile and run this project, you need to install the necessary tools."
echo "Please run the following commands for a Debian/Ubuntu-based system:"
echo ""
echo 'sudo apt-get update && sudo apt-get install -y build-essential openjdk-17-jdk python3 python3-pip libcurl4-openssl-dev'
echo ""
echo "For Python, you will need the 'requests' and 'beautifulsoup4' libraries:"
echo "pip3 install requests beautifulsoup4"
echo ""
echo "--- Setup Complete ---"
echo "You can now start adding your code to the created files inside the '$PROJECT_ROOT' directory."