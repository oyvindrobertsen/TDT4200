import os
import subprocess
import math
import matplotlib.pyplot as plt

RANGES = ((2, 1000000000),)
NPS = (1, 2, 4, 8)
INCREMENTS = 20

CMD = "mpiexec -np {np} ./parallel {lo} {hi}"

def system_call(command):
    return subprocess.check_output(command.split(" "))

results = []
for lo, hi in RANGES:
    n = int(math.ceil((hi - lo) / 20.0))
    for i in xrange(1, INCREMENTS + 1):
        local_hi = lo + (n * i) if not i == INCREMENTS else hi
        for np in NPS:
            print np, lo, local_hi
            time = system_call(CMD.format(np=np, lo=lo, hi=local_hi))
            results.append({'np': np, 'lo': lo, 'hi': local_hi, 'time': time})
    print '==='


plt.title("The influence of number of processes")
plt.plot([result['hi'] for result in results if result['np'] == 1], [result['time'] for result in results if result['np'] == 1]) # Plotline for np = 1
plt.plot([result['hi'] for result in results if result['np'] == 2], [result['time'] for result in results if result['np'] == 2]) # Plotline for np = 2
plt.plot([result['hi'] for result in results if result['np'] == 4], [result['time'] for result in results if result['np'] == 4]) # Plotline for np = 4
plt.plot([result['hi'] for result in results if result['np'] == 8], [result['time'] for result in results if result['np'] == 8]) # Plotline for np = 8
plt.legend(['np = 1', 'np = 2', 'np = 4', 'np = 8'], loc='best')
plt.xlabel('Upper limit')
plt.ylabel('Time s')
plt.show()
