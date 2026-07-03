import socket
import threading
import time
import statistics
import sys

# Global lists to store results
latencies = []
errors = 0
results_lock = threading.Lock()

def simulate_player(player_id, server_ip='127.0.0.1', server_port=8080):
    global errors
    try:
        # 1. Connect
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.settimeout(5.0) # 5 second timeout
        s.connect((server_ip, server_port))
        
        # 2. Login
        s.sendall(f"LOGIN:Player_{player_id}\n".encode())
        s.recv(1024) 
        
        # 3. Matchmake (Start tracking latency here)
        start_time = time.time()
        s.sendall(b"MATCHMAKE\n")
        s.recv(1024)
        end_time = time.time()
        
        s.close()
        
        # Record the latency in milliseconds
        latency_ms = (end_time - start_time) * 1000
        
        with results_lock:
            latencies.append(latency_ms)
            
    except Exception as e:
        with results_lock:
            errors += 1

def run_stress_test(concurrent_players):
    print(f"Spawning {concurrent_players} concurrent players...")
    global latencies, errors
    latencies = []
    errors = 0
    
    threads = []
    start_test = time.time()
    
    for i in range(concurrent_players):
        t = threading.Thread(target=simulate_player, args=(i,))
        threads.append(t)
        t.start()
        
    for t in threads:
        t.join()
        
    end_test = time.time()
    
    # Calculate and Print Statistics
    print("\n--- LOAD TEST RESULTS ---")
    print(f"Total Time Elapsed: {end_test - start_test:.2f} seconds")
    print(f"Successful Calls:   {len(latencies)}")
    print(f"Failed Calls:       {errors}")
    
    if latencies:
        print(f"Min Latency:        {min(latencies):.2f} ms")
        print(f"Max Latency:        {max(latencies):.2f} ms")
        print(f"Avg Latency:        {statistics.mean(latencies):.2f} ms")
        print(f"95th Percentile:    {statistics.quantiles(latencies, n=100)[94]:.2f} ms")

if __name__ == "__main__":
    users = int(sys.argv[1]) if len(sys.argv) > 1 else 10
    run_stress_test(users)