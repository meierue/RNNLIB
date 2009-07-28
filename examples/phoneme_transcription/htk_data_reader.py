from scipy import *

def extractHlistData(filename,preFrames=0,postFrames=0,printFilename=1):
	if (printFilename):
		print ("reading	input file "+filename)
	f=file(filename)
	s = f.readline()
	input = []
	while (s!=""):
		words = s.split()
		vect = []
		for w in words:
			vect.append(float(w))
		input.append(vect)
		s=f.readline()
#	print "input",shape(input)
	if (preFrames or postFrames):
		totalFrames=shape(input)[0]
		oldInputsPerFrame=shape(input)[1]
		contextInputsPerFrame=oldInputsPerFrame*(preFrames+postFrames+1)
		contextInput= zeros((totalFrames,contextInputsPerFrame),Float)
		for centreFrame in range(totalFrames):
			offset=0
			for inputFrame in range (centreFrame-preFrames,centreFrame+postFrames+1):
				if (inputFrame>=0 and inputFrame<totalFrames):
					for i in range(oldInputsPerFrame):
						offset=(inputFrame-centreFrame+preFrames)*oldInputsPerFrame
						contextInput[centreFrame][offset+i]=input[inputFrame][i]
		print "contextInput",shape(contextInput) 
		return contextInput
	else:
		return input


def getNumInputs(filename):
	f=file(filename)
	return len(f.readline().split())

def calculatePatternLengths(fileList, lengths=0):
	totalLength=0
	for filename in fileList:
		length=len(file(filename).readlines())
		if lengths <> 0:
			lengths.append(length)
		totalLength += length
#		f=file(filename)
#		fileLength = 0
#		for line in f:
#			fileLength += 1
#		totalLength+=fileLength
	return totalLength

def extractHlistDataNew(filename,inputs,index,printFilename=1):
	if (printFilename):
		print ("reading	input file "+filename)
	f=file(filename)
	s = f.readline()
	numLines=0
	offset=index
	while (s!=""):
		words = s.split()
		for w in words:
			inputs[offset]=float(w)
			offset=offset+1
#			inputs.append(float(w))
		s=f.readline()
		numLines=numLines+1
	return numLines
