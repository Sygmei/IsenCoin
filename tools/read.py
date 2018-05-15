import socket
import time

hote = "localhost"
port = 15317

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect((hote, port))
print("Connection on {}".format(port))

print("...")
time.sleep(1)
bjr = sock.recv(1024)
print(bjr)

sock.close()