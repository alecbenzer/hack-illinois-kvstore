import socket
import sys

HOST, PORT = 'localhost', 9999
data = 'Sai'

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
rec="foo"
try:
    #connect to server
    sock.connect((HOST, PORT))
	    
    #send message
    sock.send(data)

    rec = sock.recv(4096)

finally:
    sock.close()

    print "Sent:    {}".format(data)
    print "Received:{}".format(rec)

