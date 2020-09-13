
def speedtest():
    somma = 0

    for i in range(1,1001):
        print ("Ciao " + str(i))

import time
start_time = time.time()

speedtest()

print("--- %s seconds ---" % (time.time() - start_time))


