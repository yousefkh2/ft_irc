#!/usr/bin/env python3
import socket
import threading
import time
import sys

def client_session(client_id, host, port, password):
    """Simulate a client connecting and performing IRC operations"""
    try:
        # Create socket and connect
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(10)  # 10 second timeout
        
        print(f"Client {client_id}: Connecting to {host}:{port}")
        sock.connect((host, port))
        print(f"Client {client_id}: Connected successfully")
        
        # Send PASS command
        sock.send(f"PASS {password}\r\n".encode())
        time.sleep(0.1)
        
        # Send NICK command
        sock.send(f"NICK user{client_id}\r\n".encode())
        time.sleep(0.1)
        
        # Send USER command
        sock.send(f"USER user{client_id} 0 * :Test User {client_id}\r\n".encode())
        time.sleep(0.5)
        
        # Try to join a channel
        sock.send(f"JOIN #test{client_id % 3}\r\n".encode())  # 3 different channels
        time.sleep(0.2)
        
        # Send a message to the channel
        sock.send(f"PRIVMSG #test{client_id % 3} :Hello from client {client_id}\r\n".encode())
        time.sleep(0.2)
        
        # Try to list channels
        sock.send("LIST\r\n".encode())
        time.sleep(0.2)
        
        # Send WHO command
        sock.send(f"WHO #test{client_id % 3}\r\n".encode())
        time.sleep(0.2)
        
        # Keep connection alive for a bit
        time.sleep(2)
        
        # Gracefully disconnect
        sock.send("QUIT :Test completed\r\n".encode())
        time.sleep(0.1)
        
        print(f"Client {client_id}: Operations completed successfully")
        
    except Exception as e:
        print(f"Client {client_id}: Error - {e}")
    finally:
        try:
            sock.close()
        except:
            pass
        print(f"Client {client_id}: Disconnected")

def stress_test(num_clients, host, port, password):
    """Run stress test with multiple clients"""
    print(f"Starting stress test with {num_clients} clients...")
    
    threads = []
    
    # Start all clients
    for i in range(num_clients):
        thread = threading.Thread(target=client_session, args=(i+1, host, port, password))
        threads.append(thread)
        thread.start()
        time.sleep(0.1)  # Small delay between connections
    
    # Wait for all clients to complete
    for thread in threads:
        thread.join()
    
    print(f"Stress test with {num_clients} clients completed!")

if __name__ == "__main__":
    host = "localhost"
    port = 6667
    password = "password123"
    
    # Test with different numbers of clients
    test_cases = [5, 10, 15, 20]
    
    for num_clients in test_cases:
        print(f"\n{'='*50}")
        print(f"TESTING WITH {num_clients} CLIENTS")
        print(f"{'='*50}")
        
        stress_test(num_clients, host, port, password)
        
        print(f"\nWaiting 3 seconds before next test...")
        time.sleep(3)
    
    print("\nAll stress tests completed!")
