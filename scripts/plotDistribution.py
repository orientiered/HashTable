import numpy as np
import matplotlib.pyplot as plt

bucketLengths = []
with open("distribution.txt") as input:
    bucketLengths = list(map(int, input.readlines()))
    bucketLengths = np.array(bucketLengths)


# xaxis = np.arange(len(bucketLengths))
# plt.plot(xaxis, bucketLengths)
# plt.show()

maxLen = np.max(bucketLengths)
totalElems = np.sum(bucketLengths)

lengthCounts = np.zeros(maxLen+1)
for len in bucketLengths:
    lengthCounts[len] += 1

xaxis = np.arange(0, maxLen+1)
def normal(x, sigma, median):
    return 1/np.sqrt(2*3.1415)/sigma * np.exp(-np.square(x-median)/2/np.square(sigma))

plt.bar(xaxis, lengthCounts, label='crc32')
# plt.bar(xaxis, lengthCounts/1501, label='checksum')
plt.plot(xaxis, 1501*normal(xaxis, 4.03, 16.42), label='Норм. распределение')
plt.legend()
plt.xlabel("Размер бакета")
plt.ylabel("Количество бакетов")
plt.title("Гистограмма распределения бакетов по их размеру")
# plt.show()
plt.savefig("crc32_dist.svg", format='svg')