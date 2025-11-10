#!/bin/bash

# ==============================================================================
#  C, C++, and Python Crawler Test Script
# ==============================================================================
# This script runs all working versions of the C, C++, and Python crawlers
# to verify that they are compiled and running correctly.
# ==============================================================================

# --- Configuration ---
export TARGET_URL="http://192.168.0.141:5000/page_0.html"
export MAX_PAGES=20
export NUM_THREADS=4
# --------------------

# Define colors
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

# Function to run a command and check its output
run_test() {
    local test_name="$1"
    local command_to_run="$2"
    
    echo -e "\n${YELLOW}--- Running Test: $test_name ---${NC}"
    echo "Command: $command_to_run"
    
    output=$(eval "$command_to_run")
    
    if [[ $? -eq 0 && $(echo "$output" | jq . > /dev/null 2>&1; echo $?) -eq 0 ]]; then
        echo -e "${GREEN}SUCCESS! Output:${NC}"
        echo "$output" | jq .
    else
        echo -e "${RED}FAILURE!${NC}"
        echo "Error running the $test_name crawler."
        echo "Output was: $output"
    fi
}

# --- Check for jq ---
if ! command -v jq &> /dev/null; then
    echo -e "${YELLOW}Warning: 'jq' is not installed. sudo apt install jq${NC}"
    jq() { cat; }
fi

# --- Setup ---
echo "Temporarily updating STARTING_URL in source files for this test..."
sed -i "s|http://127.0.0.1:8000/page_0.html|$TARGET_URL|g" python/*.py cpp/*.hpp c/*.h
sed -i "s|MAX_PAGES_TO_CRAWL = 500|MAX_PAGES_TO_CRAWL = $MAX_PAGES|g" python/*.py
sed -i "s|MAX_PAGES_TO_CRAWL 500|MAX_PAGES_TO_CRAWL $MAX_PAGES|g" cpp/*.hpp c/*.h

# --- Re-compile C & C++ ---
echo -e "\n${YELLOW}--- Re-compiling C & C++ Crawlers ---${NC}"
g++ cpp/single_threaded.cpp -o cpp/single_threaded -std=c++17 -lcurl -lgumbo && echo "C++ single_threaded OK"
g++ cpp/multi_threaded_std.cpp -o cpp/multi_threaded_std -std=c++17 -lcurl -lgumbo -pthread && echo "C++ std::thread OK"
g++ cpp/multi_threaded_omp.cpp -o cpp/multi_threaded_omp -std=c++17 -lcurl -lgumbo -fopenmp && echo "C++ OpenMP OK"

gcc c/single_threaded.c -o c/single_threaded $(pkg-config --cflags --libs libxml-2.0) -lcurl -ljansson && echo "C single_threaded OK"
gcc c/multi_threaded_pthreads.c -o c/multi_threaded_pthreads $(pkg-config --cflags --libs libxml-2.0) -lcurl -ljansson -pthread && echo "C Pthreads OK"
gcc c/multi_threaded_omp.c -o c/multi_threaded_omp $(pkg-config --cflags --libs libxml-2.0) -lcurl -ljansson -fopenmp && echo "C OpenMP OK"


# --- Run Python Tests ---
run_test "Python (Single-Threaded)" "python3 python/single_threaded.py"
run_test "Python (Multi-Threaded, $NUM_THREADS threads)" "python3 python/multi_threaded.py $NUM_THREADS"

# --- Run C++ Tests ---
run_test "C++ (Single-Threaded)" "./cpp/single_threaded"
run_test "C++ (std::thread, $NUM_THREADS threads)" "./cpp/multi_threaded_std $NUM_THREADS"
run_test "C++ (OpenMP, $NUM_THREADS threads)" "./cpp/multi_threaded_omp $NUM_THREADS"

# --- Run C Tests ---
run_test "C (Single-Threaded)" "./c/single_threaded"
run_test "C (Pthreads, $NUM_THREADS threads)" "./c/multi_threaded_pthreads $NUM_THREADS"
run_test "C (OpenMP, $NUM_THREADS threads)" "./c/multi_threaded_omp $NUM_THREADS"


# --- Cleanup ---
echo -e "\n${YELLOW}--- Test complete. Reverting URL changes in source files... ---${NC}"
sed -i "s|$TARGET_URL|http://127.0.0.1:8000/page_0.html|g" python/*.py cpp/*.hpp c/*.h
sed -i "s|MAX_PAGES_TO_CRAWL = $MAX_PAGES|MAX_PAGES_TO_CRAWL = 500|g" python/*.py
sed -i "s|MAX_PAGES_TO_CRAWL $MAX_PAGES|MAX_PAGES_TO_CRAWL 500|g" cpp/*.hpp c/*.h

echo "Done."