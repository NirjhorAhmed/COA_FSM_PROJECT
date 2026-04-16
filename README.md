# FSM-based Simple Cache Controller Simulator

A cycle-accurate simulator of a simple direct-mapped cache controller implemented as a 4-state Finite State Machine (FSM), modeled after **Figure 5.38** from computer architecture textbooks (typically Patterson & Hennessy).

This simulator demonstrates cache behavior including:
- Read Hit / Read Miss
- Write Hit / Write Miss
- Write-back on dirty block replacement
- Memory access latency modeling

## Features

- Exact 4-state FSM: **IDLE → COMPARE_TAG → WRITE_BACK → ALLOCATE**
- Cycle-by-cycle detailed output
- Supports multiple test cases (Read/Write Hit & Miss scenarios)
- Clean separation between Memory and Cache Controller
- Easy to extend with new test cases

## Project Structure
Cache-Simulator/
├── main.cpp          # Complete source code (single file)
├── README.md         # This file
└── (optional) CMakeLists.txt
text## Dependencies

- **None** — This is a self-contained project.
- Only uses the C++ Standard Library (`<bits/stdc++.h>` for simplicity).
- No external libraries or downloads required.

## How to Build and Run

### Option 1: Using g++ (Recommended for most users)
# Clone or download the project
git clone <your-repo-url>
cd Cache-Simulator

# Compile
g++ -std=c++17 -O2 -Wall main.cpp -o cache_simulator

# Run the simulation
./cache_simulator
Option 2: Using CMake (Optional)
If you prefer CMake:
Bashmkdir build && cd build
cmake ..
make
./cache_simulator
Option 3: Online Compilers
You can also paste the entire main.cpp code into:

Godbolt.org
OnlineGDB
Replit

Expected Output
The program will display:

A header showing the FSM states: IDLE -> COMPARE_TAG -> WRITE_BACK -> ALLOCATE
Detailed cycle-by-cycle execution table for 6 test cases
Hit/Miss information
CPU ready status
Memory read/write activity
Final data output

Each test case runs until all requests are completed and the controller returns to IDLE state.
Test Cases Included

Read Miss → Allocate
Read Hit
Write Hit
Write Miss (Clean)
Write Miss with Write-back
Mixed Sequence (Read + Write operations)

Customization
You can easily add new test cases by modifying the tests vector in main():
C++vector<pair<string, vector<Request>>> tests = {
    {"Your Test Name", {{is_read, address, data}, ...}},
    // Add more here
};
