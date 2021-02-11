import socket 
# Specify the IP addr and port number 
# (use "127.0.0.1" for localhost on local machine)
# Create a socket 
HOST, PORT = '127.0.0.1', 65432
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
#connect to a remote socket
s.connect((HOST, PORT))
while(True):
    response1 = input(s.recv(1000).decode('utf-8'))
    s.send(response1.encode('utf-8'))
    response2 = input(s.recv(1000).decode('utf-8'))
    s.send(response2.encode('utf-8'))
    if response2 =='n':        
        break
 