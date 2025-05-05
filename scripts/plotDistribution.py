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

lengthCounts = np.zeros(maxLen)
for len in bucketLengths:
    lengthCounts[len-1] += 1

xaxis = np.arange(1, maxLen+1)
def normal(x, sigma, median):
    return 1/np.sqrt(2*3.1415)/sigma * np.exp(-np.square(x-median)/2/np.square(sigma))

plt.bar(xaxis, lengthCounts/1501, label='crc32')
plt.plot(xaxis, normal(xaxis, 4.03, 16.42), label='Нормальное распределение')
plt.legend(fontsize=20)
plt.xlabel("Размер бакета", fontsize=20)
plt.ylabel("Количество бакетов", fontsize=20)
plt.title("Гистограмма распределения бакетов по их размеру", fontsize=20)
plt.show()