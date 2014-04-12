def send_message(msg):
    import socket
    import sys

    HOST, PORT = 'localhost', 9999
    data = msg

    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    try:
        #connect to server
        sock.connect((HOST, PORT))
        
        #send message
        bytes_sent = sock.send(data)

        recieved = sock.recv(1024)

    finally:
        sock.close()

    recieve_message(recieved)

OP_SET = 0x02
OP_SET_ACK = 0x12
OP_GET = 0x03
OP_GET_RET = 0x13
OP_GET_FAIL = 0x23

def recieve_message(msg=None):
    import struct
    if msg:
        msgLength = struct.unpack(">i", msg[:4])
        OP_CODE = struct.unpack("b", msg[4])

        if OP_CODE == OP_GET_RET:
            keyLength = int(struct.unpack("=i", msg[5:9]))
            key = struct.unpack("s", msg[9:9+keyLength])
            valLength = int(struct.unpack("=i", msg[9+keyLength:14+keyLength]))
            val = struct.unpack("s", msg[14+keyLength:14+keyLength+valLength)
            print "Recieved: GET key={} -> {}".format(key, val)

        elif OP_CODE == OP_GET_FAIL:
            keyLength = int(struct.unpack("=i", msg[5:9]))
            key = struct.unpack("s", msg[9:9+keyLength])
            print "Recieved: GET FAILED for key={}".format(key)
            
        elif OP_CODE == OP_SET_ACK:
            keyLength = int(struct.unpack("=i", msg[5:9]))
            key = struct.unpack("s", msg[9:9+keyLength])
            valLength = int(struct.unpack("=i", msg[9+keyLength:14+keyLength]))
            val = struct.unpack("s", msg[14+keyLength:14+keyLength+valLength)
            print "Recieved: SET k: {} -> v: {}".format(key, val)
            
    else:
        print "fail"


def do_get(key):
    import struct
    keyLength = len(key)
    msgLength = 1+4+keyLength #1 byte for OPCode + 4bytes for keylength num + keylength bytes
    msg = bytearray(4+msgLength) #4byes for msgLength int
    
    msg.append(struct.pack(">i", msgLength)) # add bytes for msg length to bytearray
    msg.append(OP_GET)
    msg.append(struct.pack(">i", keyLength)) # add bytes for keylength
    msg.append(struct.pack("s", key)) #add bytes for key
    send_message(msg)

def do_set(key, val):
    import struct
    keyLength = len(key)
    valueLength = len(val)
    msgLength = 1+4+keyLength+valueLength
    msg = bytearray(4+msgLength) #4byes for msgLength int
    
    msg.append(struct.pack(">i", msgLength)) # add bytes for msg length to bytearray
    msg.append(struct.pack("b",OP_SET)) # opcode
    msg.append(struct.pack(">i", keyLength)) # add bytes for keylength
    msg.append(struct.pack("s", key)) #add bytes for key
    msg.append(struct.pack(">i", valueLength)) # add bytes for keylength
    msg.append(struct.pack("s", val)) #add bytes for key

    send_message(msg)
