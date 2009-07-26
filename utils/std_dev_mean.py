#!/usr/bin/python
import numpy
from math import sqrt
import sys

vals = []
for l in sys.stdin.readlines():
	vals.append(float(l))
numVals = len(vals)
stdDev = numpy.std(vals)
print numVals,"values"
print "mean",numpy.mean(vals)
print "std deviation",stdDev
print "std error",stdDev/sqrt(numVals)

