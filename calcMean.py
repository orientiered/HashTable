import numpy as np

time, clocks = [], []
for i in range(5):
    t, c = map(float, input().split())
    time.append(t)
    clocks.append(c)

time = np.array(time)
clocks = np.array(clocks)

time_mean = np.mean(time)
clocks_mean = np.mean(clocks)
time2_mean = np.mean(np.square(time))
clocks2_mean = np.mean(np.square(clocks))

time_disp = np.sqrt((time2_mean - np.square(time_mean))) * np.sqrt(5/4)
clocks_disp = np.sqrt((clocks2_mean - np.square(clocks_mean))) * np.sqrt(5/4)

print(f'{time_mean:.2f} +- {time_disp:.2f}')
print(f'{clocks_mean:.2f} +- {clocks_disp:.2f}')