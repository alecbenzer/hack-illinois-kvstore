import socket
import struct
import kvstore_pb2

class Client:
    def __init__(self, addr):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        print "Trying to connect to", addr
        self.sock.connect(addr)
        self.struct = struct.Struct('!L')
        assert self.struct.size == 4

    def send_message(self, msg):
        data = msg.SerializeToString()

        self.sock.send(self.struct.pack(len(data)))
        self.sock.send(data)

    def receive_message(self):
        bytes_to_read = self.struct.unpack(self.sock.recv(self.struct.size))[0]
        print bytes_to_read
        response = kvstore_pb2.Response()
        response.ParseFromString(self.sock.recv(bytes_to_read))
        
        print response

    def get(self, key):
        request = kvstore_pb2.Request()
        request.get_request.key = key
        self.send_message(request)
        self.receive_message()

    def set(self, key, value):
        request = kvstore_pb2.Request()
        request.set_request.key = key
        request.set_request.value = value
        self.send_message(request)
        self.receive_message()
