import matplotlib.pyplot as plt
from pathlib import Path

CADMIUMV2_HOME = Path.home().joinpath('SED/demo-cadmiumv2')
CADMIUMV2_DIR = CADMIUMV2_HOME.joinpath('cadmium_v2')
CADMIUMV2_BLINKY_EXAMPLE = CADMIUMV2_HOME.joinpath('Blinky-improved')
CADMIUMV2_BLINKY_EXAMPLE_TOP = CADMIUMV2_BLINKY_EXAMPLE.joinpath('top_model')
LOGFILE = str(CADMIUMV2_BLINKY_EXAMPLE_TOP)+'/blinkyLog.csv'

tblk_list = []
outblk_list = []
tgen_list = []
outgen_list = []

# TODO: replace the following with pandas
with open(LOGFILE,'r') as f:
    for line in f.readlines():
        # debug
        # print(line)
        t,atomic_id,atomic_name,port_name,val = line.split(";")
        if (atomic_name=="blinky" and port_name=="out"):
            tblk_list.append(float(t))
            outblk_list.append(int(val.split(" ")[-1]))
        elif (atomic_name=="generator" and port_name=="out"):
            tgen_list.append(float(t))
            outgen_list.append(int(val.split(" ")[-1]))

f.close()

T1 = 500
T2 = 600
tblk = [x for x in tblk_list if x<T2 and x>T1]
outblk = outblk_list[:len(tblk)]
tgen = [x for x in tgen_list if x<T2 and x>T1]
outgen = outgen_list[:len(tgen)]


plt.figure(figsize=(15,5))
plt.step(tblk,outblk,label='output')
plt.plot(tgen,outgen,'r*',label='input')
plt.title('Blinky Output')
plt.xlabel('time (s)')
plt.ylabel('value')
plt.grid(True)
plt.legend()
plt.show()
