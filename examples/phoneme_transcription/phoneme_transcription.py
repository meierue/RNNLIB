#!/usr/bin/python
import netcdf_helpers
import re
from scipy import *
import array
import wave
import operator
from optparse import OptionParser
import sys

labels = ['h#','b','d','g','p','t','k','dx','q','jh','ch','s','sh','z','zh','f','th','v','dh','m','n','ng','em','en','eng','nx','l','r','w','y','hh','hv','el','iy','ih','eh','ey','ae','aa','aw','ay','ah','ao','oy','ow','uh','uw','ux','er','ax','ix','axr','ax-h','pau','epi','bcl','dcl','gcl','pcl','kcl','tcl']

#command line options
usage = 'usage: %prog [options] input_file output_file'
parser = OptionParser(usage)
parser.add_option('-m', '--mfc', dest='mfc', default=False, action='store_true', help='use MFCC inputs?')
parser.add_option('-p', '--phones', dest='phones', default='thirtynine', action='store', help='which phoneme set to use (all|fortyeight|thirtynine)')

#parse command line options
(options, args) = parser.parse_args()
if len(args) != 2:
        parser.error('incorrect number of arguments')
inputFilename = args [0]
ncFilename = args[1]
print options
print 'input filename', inputFilename
print 'data filename', ncFilename

#sort out labels
reducedLabels = dict()
if options.phones == 'thirtynine':
	reducedLabels['h#'] = 'sil'
	reducedLabels['pau'] = 'sil'
	reducedLabels['epi'] = 'sil'
	reducedLabels['bcl'] = 'sil'
	reducedLabels['dcl'] = 'sil'
	reducedLabels['gcl'] = 'sil'
	reducedLabels['kcl'] = 'sil'
	reducedLabels['pcl'] = 'sil'
	reducedLabels['tcl'] = 'sil'
	reducedLabels['hv'] = 'hh'
	reducedLabels['eng'] = 'ng'
	reducedLabels['nx'] = 'n'
	reducedLabels['en'] = 'n'
	reducedLabels['em'] = 'm'
	reducedLabels['axr'] = 'er'
	reducedLabels['ux'] = 'uw'
	reducedLabels['el'] = 'l'
	reducedLabels['zh'] = 'sh'
	reducedLabels['aa'] = 'ao'
	reducedLabels['ix'] = 'ih'
	reducedLabels['ax'] = 'ah'
	reducedLabels['ax-h'] = 'ah'
	reducedLabels['q'] = 'NULL'
	labels.append("sil")
if options.phones == 'fortyeight':
	reducedLabels['h#'] = 'sil'
	reducedLabels['pau'] = 'sil'
	reducedLabels['bcl'] = 'vcl'
	reducedLabels['dcl'] = 'vcl'
	reducedLabels['gcl'] = 'vcl'
	reducedLabels['kcl'] = 'cl'
	reducedLabels['pcl'] = 'cl'
	reducedLabels['tcl'] = 'cl'
	reducedLabels['hv'] = 'hh'
	reducedLabels['eng'] = 'ng'
	reducedLabels['nx'] = 'n'
	reducedLabels['em'] = 'm'
	reducedLabels['axr'] = 'er'
	reducedLabels['ux'] = 'uw'
	reducedLabels['ax-h'] = 'ah'
	reducedLabels['q'] = 'NULL'
	labels.append("sil")
	labels.append("cl")
	labels.append("vcl")
if len(reducedLabels):
	reducedLabelSet = set()
	for l in labels:
		try:
			lab = reducedLabels[l]
			if lab <> 'NULL':			
				reducedLabelSet.add(lab)
		except KeyError:
			reducedLabelSet.add(l)
	labels = list(reducedLabelSet)
labels.sort()
print len(labels), 'labels:'
print labels

#read in data
seqDims = []
seqLengths = []
targetStrings = []
seqTags = []
inputs = []
filelines = file(inputFilename).read().strip().split('\n')
print 'reading', len(filelines), 'data files'
for tag in filelines:
	print 'reading file', tag
	seqTags.append(tag)
	start = len(inputs)
	if options.mfc:
		inputlines = file(tag.replace('.WAV','.mft')).read().strip().split('\n')
		for i in inputlines:
			inputs.append([float(j) for j in i.split()])
	else:
		w = wave.open(tag,'r')
		inputlines = array.array('h',w.readframes(w.getnframes()))
		inputs.extend([[float(j)] for j in inputlines])
	inputlength = len(inputlines)
	print 'input length', inputlength
	seqDims.append([inputlength])
	seqLengths.append(inputlength)
	phnfile = tag.replace('.WAV','.PHN')
	targetStr = ''
	for l in file(phnfile).read().strip().split('\n'):
		words = l.split()
		label = words[2]
		if label in reducedLabels:
			label = reducedLabels[label]
		if label <> "NULL":
			targetStr += label + ' '
	targetStrings.append(targetStr.strip())
	print 'phoneme transcription (' + str(len(targetStr)) +' labels)'
	print targetStr
	
#create a new .nc file
file = netcdf_helpers.NetCDFFile(ncFilename, 'w')

#create the dimensions
netcdf_helpers.createNcDim(file,'numSeqs',len(seqLengths))
netcdf_helpers.createNcDim(file,'numTimesteps',len(inputs))
netcdf_helpers.createNcDim(file,'inputPattSize',len(inputs[0]))
netcdf_helpers.createNcDim(file,'numDims',1)
netcdf_helpers.createNcDim(file,'numLabels',len(labels))

#create the variables
netcdf_helpers.createNcStrings(file,'seqTags',seqTags,('numSeqs','maxSeqTagLength'),'sequence tags')
netcdf_helpers.createNcStrings(file,'labels',labels,('numLabels','maxLabelLength'),'labels')
netcdf_helpers.createNcStrings(file,'targetStrings',targetStrings,('numSeqs','maxTargStringLength'),'target strings')
netcdf_helpers.createNcVar(file,'seqLengths',seqLengths,'i',('numSeqs',),'sequence lengths')
netcdf_helpers.createNcVar(file,'seqDims',seqDims,'i',('numSeqs','numDims'),'sequence dimensions')
netcdf_helpers.createNcVar(file,'inputs',inputs,'f',('numTimesteps','inputPattSize'),'input patterns')

#write the data to disk
print 'closing file', ncFilename
file.close()

