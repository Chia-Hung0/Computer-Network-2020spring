from socket import *
import sys
import time
from _thread import*

if len(sys.argv) <= 1:
	print('Usage : "python ProxyServer.py server_ip"\n[server_ip : It is the IP Address Of Proxy Server & Web server.')
	sys.exit(2)
# Environment : Python 3.8
# Create a server socket, bind it to a port and start listening
tcpSerSock = socket(AF_INET, SOCK_STREAM)
# Fill in start.
tcpSerSock.bind(('', 65432))
tcpSerSock.listen(5)
# Fill in end.
def routine(tcpCliSock):
	# Strat receiving data from the client
	message = tcpCliSock.recv(1024).decode()
	print(message)
	# Extract the filename from the given message
	print(message.split()[1])
	filename = message.split()[1].partition("/")[2]
	print(filename)
	fileExist = "false"
	filetouse = "/" + filename
	print(filetouse)
	try:
		# Check wether the file exist in the cache
		f = open(filetouse[1:], "r")
		outputdata = f.read()
		fileExist = "true"
		# ProxyServer finds a cache hit and generates a response message
		
		# Fill in start.
		for data in outputdata:
				tcpCliSock.send(data.encode())
		# Fill in end.
		print('Read from cache')
	# Error handling for file not found in cache
	except IOError:
		if fileExist == "false":
			# Create a socket on the proxyserver
			c = socket(AF_INET, SOCK_STREAM)
			try:
				# Connect to the socket to port 80
				hostname = filename.replace("www.","",1)
				c.connect((hostname, 80))
				# ask port 127.0.0.1:80 for the file requested by the client
				request ="GET "+" http://" + filename + " HTTP/1.0\n\n" + "Host: " + hostname +"\n\n"
				c.send(request.encode())
				# receive the response 
				# Fill in start.
				html_file = c.recv(1024)
				# Fill in end.
				# Create a new file in the cache for the requested file.
				# Also send the response in the buffer to client socket and the corresponding file in the cache
				tmpFile = open("./" + filename,"wb")
				# Fill in start.
				tmpFile.write(html_file)
				tmpFile.close()
				tmpFile = open("./" + filename,"rb")
				outputdata = tmpFile.read()
				tcpCliSock.send(outputdata)
				# Fill in end.
				
				
			except:
				print("Illegal request")
			c.close()
		else:
			# HTTP response message for file not found
			# Fill in start.
			tcpCliSock.send("HTTP/1.1 404 Not Found\r\n".encode())
			# Fill in end.
	# Close the client and the server sockets. For testing multi-user, you should comment the tcpCliSock.close()
	#tcpCliSock.close()

# Fill in start. Change this part, such that multi-users can connect to this proxy server
while True:
	print('Ready to serve...')
	tcpCliSock, addr = tcpSerSock.accept()
	print('Received a connection from:', addr)
	start_new_thread(routine,(tcpCliSock,))
tcpSerSock.close()
# Fill in end
