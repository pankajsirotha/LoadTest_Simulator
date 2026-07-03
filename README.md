# Distributed Game Call Simulator & Load Tester

📌 Project Overview

This project is a localized load-testing pipeline designed to simulate concurrent game client behavior, measure network latency, and identify concurrency bottlenecks in a C++ server environment.

It was built to demonstrate end-to-end systems design, stress testing, and root-cause analysis for multiplayer backend architecture.

⚙️ Architecture

The project consists of two main components:

The Target (C++ Matchmaking Server): A multithreaded TCP server utilizing POSIX/Winsock sockets. It simulates the computational cost of a matchmaking queue.

The Load Generator (Python): A multithreaded testing tool that spawns hundreds of concurrent clients. Each client executes a stateful game call pattern (Login -> Matchmake), records round-trip latency, and aggregates the data.

🚀 How to Run

1. Start the C++ Server

Compile the server (Windows MinGW example):

g++ server.cpp -o server -lws2_32 -pthread
.\server.exe


(For Linux/macOS, use: g++ server.cpp -o server -pthread and ./server)

2. Execute the Python Load Test

Run the test script, passing the number of concurrent users as an argument:

# Run a baseline test with 10 concurrent players
python load_tester.py 10

# Run a stress test with 500 concurrent players
python load_tester.py 500


📊 Root Cause Analysis & Bottleneck Injection

To simulate a real-world scaling failure, an artificial bottleneck (a global std::mutex) was initially injected into the C++ matchmaking logic. This forced all threads to compete for a single shared resource.

Test Results (With Mutex Contention)

Users: 500 concurrent

Average Latency: ~2,100 ms

95th Percentile (p95): > 4,500 ms

Conclusion: The CPU and network throughput were stable, but severe lock contention caused a thread queue, leading to massive latency spikes for players at the back of the line.

The Fix (Lock-Free / Parallel Processing)

By removing the global mutex and allowing threads to process their simulated workloads in parallel (mimicking a distributed or lock-free matchmaking architecture), the bottleneck was eliminated.

Test Results (Post-Fix)

Users: 500 concurrent

Average Latency: ~12 ms

95th Percentile (p95): ~15 ms

Conclusion: The server successfully handled 500 concurrent connections in parallel with zero lock contention, resulting in a flat, stable latency graph.
