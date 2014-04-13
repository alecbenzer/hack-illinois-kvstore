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

OP_DEL = 0x04
OP_DEL_ACK = 0x14

def recieve_message(msg=None):
    import struct
    if msg:
        msgLength = struct.unpack(">i", msg[:4])[0]
        OP_CODE = int(struct.unpack("b", msg[4])[0])
        if OP_CODE == OP_GET_RET:
            keyLength = int(struct.unpack_from(">i", msg, 5)[0])
            key = struct.unpack("%ds"%keyLength, msg[9:9+keyLength])[0]
            valueLength = int(struct.unpack_from(">i", msg, 9+keyLength)[0])
            val = struct.unpack("%ds"%valueLength, msg[13+keyLength:])[0]
            print "'{}'".format(val)

        elif OP_CODE == OP_GET_FAIL:
            keyLength = int(struct.unpack_from(">i", msg, 5)[0])
            key = struct.unpack("%ds"%keyLength, msg[9:9+keyLength])[0]
            print "nil"

        elif OP_CODE == OP_SET_ACK:
            keyLength = int(struct.unpack_from(">i", msg, 5)[0])
            key = struct.unpack("%ds"%keyLength, msg[9:9+keyLength])[0]
            valueLength = int(struct.unpack_from(">i", msg, 9+keyLength)[0])
            val = struct.unpack("%ds"%valueLength, msg[13+keyLength:13+keyLength+valueLength])[0]
            print "'{}' -> '{}'".format(key, val)

        elif OP_CODE == OP_DEL_ACK:
            keyLength = int(struct.unpack_from(">i", msg, 5)[0])
            key = struct.unpack("%ds"%keyLength, msg[9:9+keyLength])[0]
            print "'{}' deleted".format(key)
        
        else:
            print "Unknown OP_CODE: {}".format(OP_CODE)

    else:
        print "fail"

def do_get(key):
    import struct
    keyLength = len(key)
    msgLength = 1+4+keyLength #1 byte for OPCode + 4bytes for keylength num + keylength bytes
    msg = bytearray(4+msgLength) #4byes for msgLength int
    struct.pack_into("!i", msg, 0, msgLength)
    struct.pack_into("b", msg, 4, OP_GET)
    struct.pack_into("!i", msg, 5, keyLength)
    struct.pack_into("%ds"%len(key), msg, 9, key)
    send_message(msg)

def do_set(key, val):
    import struct
    keyLength = len(key)
    valueLength = len(val)
    msgLength = 1+8+keyLength+valueLength

    msg = bytearray(4+msgLength)
    struct.pack_into("!i", msg, 0, msgLength)
    struct.pack_into("b", msg, 4, OP_SET)
    struct.pack_into("!i", msg, 5, keyLength)
    struct.pack_into("%ds"%len(key), msg, 9, bytes(key))
    struct.pack_into("!i", msg, 9+keyLength, valueLength)
    struct.pack_into("%ds"%len(val), msg, 13+keyLength, bytes(val))
    send_message(msg)

def do_del(key):
    import struct
    keyLength = len(key)
    msgLength = 1+4+keyLength #1 byte for OPCode + 4bytes for keylength num + keylength bytes
    msg = bytearray(4+msgLength) #4byes for msgLength int
    struct.pack_into("!i", msg, 0, msgLength)
    struct.pack_into("b", msg, 4, OP_DEL)
    struct.pack_into("!i", msg, 5, keyLength)
    struct.pack_into("%ds"%len(key), msg, 9, key)
    send_message(msg)

def do_getset(key, val):
    do_get(key)
    do_set(key, val)
