import socket
import sys

HOST = '127.0.0.1'
PORT = 4444

print("[Debian Bridge] Online. Connecting to QEMU Serial Port...")

# Create a client socket instead of a binding server socket
conn = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

try:
    # Directly connect to the port that QEMU has already opened
    conn.connect((HOST, PORT))
    print("[Debian Bridge] Connected to Custom OS environment successfully!")
except Exception as e:
    print(f"[Error] Could not connect to QEMU: {e}")
    print("Make sure QEMU is running first with the -serial flag!")
    sys.exit(1)

buffer = ""
while True:
    try:
        data = conn.recv(1024).decode('utf-8', errors='ignore')
        if not data:
            break
        buffer += data
        
        if "\n" in buffer:
            line, buffer = buffer.split("\n", 1)
            line = line.strip()
            
            if not line:
                continue
                
            print(f"[OS Request]: {line}")
            
            if line.startswith("FETCH_PKG:"):
                # Cleanly extract the package name string out of the transmission payload
                package_name = line.split(":", 1)[1]
                print(f"[Debian Processing]: Fetching binary layout for '{package_name}'...")
                
                # These hex values simulate real x86 machine instructions (binary code).
                # This small code segment writes a cyan 'X' to screen memory and loops forever.
                # Assembly equivalent: mov word [0xB80A0], 0x0B58 -> jmp $
                mock_binary_code = bytes([0xC7, 0x05, 0xA0, 0x80, 0x0B, 0x00, 0x58, 0x0B, 0x00, 0x00, 0xEB, 0xFE, 0x00])
                
                conn.sendall(mock_binary_code)
                print("[Debian Processing]: Binary bytes injected successfully into connection link.")
                
    except KeyboardInterrupt:
        break

conn.close()
print("\n[Debian Bridge] Offline.")

