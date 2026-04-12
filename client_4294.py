import socket
import sys

HOST = '127.0.0.1'
PORT = 50294
BUFFER_SIZE = 1024
HEADER = "LEN: %d %s"
KEY = 4

G, P = 5, 23

def mod_exp(base, exp, mod):
    result = 1
    base = base % mod
    while exp > 0:
        if (exp % 2) == 1:
            result = (result * base) % mod
        exp = exp >> 1
        base = (base * base) % mod
    return result

def xor_cipher(data, key):
    """Decrypt data in-place using XOR cipher with single byte key"""
    key_byte = key & 0xFF
    result = bytearray(data)
    for i in range(len(result)):
        result[i] ^= key_byte
    return bytes(result)

def exchange_dh(sock):
    """Perform Diffie-Hellman key exchange"""
    pub = mod_exp(G, KEY, P)
    
    # Send pub key to server
    sock.send(pub.to_bytes(8, byteorder='little', signed=False))
    
    # receive server's pub key
    server_pub_bytes = sock.recv(8)
    if not server_pub_bytes:
        raise Exception("Failed to receive public key from server")
    server_pub = int.from_bytes(server_pub_bytes, byteorder='little', signed=False)
    
    # Compute shared secret
    shared_secret = mod_exp(server_pub, KEY, P)
    return shared_secret

def secure_send(sock, data, shared_secret):
    """Encrypt data using XOR cipher and send it securely over the socket"""
    key_byte = shared_secret & 0xFF
    encrypted_data = bytearray(data.encode())
    for i in range(len(encrypted_data)):
        encrypted_data[i] ^= key_byte
    sock.send(bytes(encrypted_data))

def main():
    # Socket setup
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect((HOST, PORT))
    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(1)
        
    # User interaction
    print("Choose an option:")
    print("0: LOGIN")
    print("1: REGISTER")
    print("2: LOGOUT")
    print("3: SEND MESSAGE")
    
    sys.stdout.write("option> ")
    sys.stdout.flush()
    option = int(input())
    
    payload = ""
    
    if option == 0:
        print("====LOGIN====")
        sys.stdout.write("Enter username: ")
        sys.stdout.flush()
        username = input()
        sys.stdout.write("Enter password: ")
        sys.stdout.flush()
        password = input()
        payload = f'LOGIN: {username} {password}'
    elif option == 1:
        print("====REGISTER====")
        sys.stdout.write("Enter username: ")
        sys.stdout.flush()
        username = input()
        sys.stdout.write("Enter password: ")
        sys.stdout.flush()
        password = input()
        payload = f'REGISTER: {username} {password}'
    elif option == 2:
        print("====LOGOUT====")
        sys.stdout.write("Enter username: ")
        sys.stdout.flush()
        username = input()
        payload = f'LOGOUT: {username}'
    elif option == 3:
        print("====SEND MESSAGE====")
        sys.stdout.write("Enter token: ")
        sys.stdout.flush()
        token = input()
        sys.stdout.write("Enter message: ")
        sys.stdout.flush()
        message = input()
        payload = f'MSG: token:{token} {message}'
    else:
        print("Invalid option")
        sys.exit(1)
    
    # Perform Diffie-Hellman key exchange
    try:
        shared_secret = exchange_dh(sock)
    except Exception as e:
        print(f"Error during Diffie-Hellman key exchange: {e}", file=sys.stderr)
        sock.close()
        sys.exit(1)
    
    payload_len = len(payload)
    buffer = HEADER % (payload_len, payload)
    secure_send(sock, buffer, shared_secret)
    sock.shutdown(socket.SHUT_WR)
    
    # Receive response
    try:
        data = sock.recv(BUFFER_SIZE)
        if data:
            decrypted_data = xor_cipher(data, shared_secret).decode(errors='ignore')
            print(f"Server response: {decrypted_data}")
    except Exception as e:
        print(f"Error receiving data: {e}", file=sys.stderr)
        sock.close()
        sys.exit(1)
    
    sock.close()
    return 0

if __name__ == "__main__":
    sys.exit(main())