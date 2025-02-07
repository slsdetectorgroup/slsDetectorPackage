import json
import zmq

c = zmq.Context()
s = c.socket(zmq.PULL)
s.connect("tcp://127.0.0.1:5555")

while True:
     m = s.recv_multipart()
     for p in m:
         if p.startswith(b"{"):
             print(p.decode().strip())
         else:
             print("binary")
     print("--------")

