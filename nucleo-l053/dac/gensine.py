#!/usr/bin/env python
#
# use as: ./gensine.py >src/sine.h

import math

N = 2500
R = 1920
O = 2048

L = []
for i in range(N):
    v = math.sin(i*2*math.pi/N)
    w = O + int(R*v)
    L.append(str(w))
    #print(v, w)

print(", ".join(L))
