import socket
#The calculating function
def calculator( problem ):
    x = problem.split() 
    if x[1] == '+':
        answer=float(x[0])+float(x[2])
    elif x[1] == '-':
        answer=float(x[0])-float(x[2])
    elif x[1] == '*':
        answer=float(x[0])*float(x[2])
    elif x[1] == '/':
        answer=float(x[0])/float(x[2])        
    return answer   
# Specify the IP addr and port number 
# (use "127.0.0.1" for localhost on local machine)
# Create a socket and bind the socket to the addr
# TODO start
HOST, PORT = '127.0.0.1',65432
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.bind((HOST, PORT))
# TODO end

while(True):
    # Listen for any request
    # TODO start
    s.listen(0)
    # TODO end
    print("The Grading server for HW2 is running..")

    while(True):
        # Accept a new request and admit the connection
        # TODO start
        client, address = s.accept()
        # TODO end
        print(str(address)+" connected")
        try: 
            while(True):               
                client.send(b"Welcom to the calculator server. Input your problem ?\n")
                # Recieve the data from the client and send the answer back to the client
                # Ask if the client want to terminate the process
                # Terminate the process or continue
                # TODO start
                problem = client.recv(1000).upper().decode('utf-8') 
                answer = calculator( problem )
                sentence = "The answer is " +str(answer)+".\nDo you have any question?(Y/N)\n"                         
                client.send(sentence.encode('utf-8'))
                if client.recv(1000).decode() == 'n' :
                    client.close()
                    break
             # TODO end
        except ValueError:
                print("except")
        break    
    break            
