#!/usr/bin/env python
#
# use as: ./gensine.py | fold -s -w 80 >src/sine.h

import math

N = 2500    # number of entries in wave table
R = 1920    # range of values, +/- around the offset
O = 2048    # offset, set to mid-range of the DAC

L = []
for i in range(N):
    v = math.sin(i*2*math.pi/N)
    w = O + int(R*v)
    L.append(str(w))
    #print(v, w)

print(", ".join(L))
