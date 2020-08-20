from slsdet import Eiger

d = Eiger()
threshold = range(0, 2000, 200)
for th in threshold:
    print(f'{th=}')
    d.vthreshold = th
    d.acquire()