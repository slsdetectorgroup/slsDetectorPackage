import json
import zmq
import numpy as np

from slsdet import Detector
import matplotlib.pyplot as plt

det = Detector()

zmqIp = det.zmqip
zmqport = det.zmqport
zmq_stream = det.rx_zmqstream


def zmq_receiver():
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
            break
    return data_array


def analog(data_array):
    adc_numbers = [
        9, 8, 11, 10, 13, 12, 15, 14, 1, 0, 3, 2, 5, 4, 7, 6, 23, 22, 21, 20, 19, 18, 17, 16, 31, 30, 29, 28, 27, 26,
        25, 24
    ]

    n_pixels_per_sc = 5000

    sc_width = 25
    analog_frame = np.zeros((400, 400))
    order_sc = np.zeros((400, 400))

    for n_pixel in range(n_pixels_per_sc):
        #these_dbits = int(digital_data[n_pixel])

        for i_sc, adc_nr in enumerate(adc_numbers):
            # ANALOG
            col = ((adc_nr % 16) * sc_width) + (n_pixel % sc_width)
            if i_sc < 16:
                row = 199 - int(n_pixel / sc_width)
            else:
                row = 200 + int(n_pixel / sc_width)

            index_min = n_pixel * 32 + i_sc

            pixel_value = data_array[index_min]
            analog_frame[row, col] = pixel_value
            order_sc[row, col] = i_sc
    return analog_frame


fig, ax = plt.subplots()
data = analog(data_array=zmq_receiver())
im = ax.imshow(data)
ax.invert_yaxis()
fig.colorbar(im)
plt.show()
# pg.image(data, title="test")

# if __name__ == '__main__':
#          pg.QtWidgets.QApplication.exec_()
