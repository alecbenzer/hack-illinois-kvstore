import socket
import sys

HOST, POST = 'localhost', 9999
data = 'hello'

sock = socket.socket(socket.AD_INT, socket.SOCK_STREAM)

try:
    #connect to server
    sock.connect((HOST, PORT))
    
    #send message
    sock.sendall(data)

    recieved = sock.recv(1024)

finally:
    sock.close()

print "Sent:    {}".format(data)
print "Received:{}".format(recieved)
