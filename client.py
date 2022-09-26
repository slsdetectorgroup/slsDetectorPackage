import json
import zmq
import numpy as np
from PIL import Image as im
import pyqtgraph as pg

from slsdet import Detector
import matplotlib.pyplot as plt

det = Detector()

zmqIp = det.rx_zmqip
zmqport = det.rx_zmqport
zmq_stream = det.rx_zmqstream


context = zmq.Context()
socket = context.socket(zmq.SUB)
socket.connect(f"tcp://{zmqIp}:{zmqport}")
socket.subscribe("")

while True:
    msg = socket.recv_multipart()
    if len(msg) == 2:
        header, data = msg
        jsonHeader = json.loads(header)
        print(jsonHeader)
        print(f'Data size: {len(data)}')
        data_array = np.array(np.frombuffer(data, dtype=np.uint16))
        image = data_array.reshape([400, 400])
        break
    # print(data_array1)
        # pg.image(image, title="test")

    # if __name__ == '__main__':
    #      pg.QtWidgets.QApplication.exec_()



fig, ax = plt.subplots()
im = ax.imshow(image)
fig.colorbar(im)
plt.show()