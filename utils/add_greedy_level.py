#!/usr/bin/env python
from optparse import OptionParser

usage = "usage: %prog filename code_size hidden_size greedy"
parser = OptionParser(usage)
(options, args) = parser.parse_args()

#print options
if len(args) != 4:
        parser.error("incorrect number of arguments")
infilename = args[0]
codeSize = args[1]
hiddenSize = args[2]
greedy = bool(args[3])
if greedy:
	greedyStr = "_greedy"
else:
	greedyStr = ""
outfile = file(infilename.replace("@","") + ".hidden_" + hiddenSize + "_code_" + codeSize + greedyStr + ".config", 'w')
for l in file(infilename).readlines():
	words = l.split()
	names = words[0].split('_')
	if (not(greedy) or names[-1] != 'plasticities' or names[-2] == 'output'):
		if (words[0] == 'hiddenSize'):
			outfile.write(l.strip() + ',' + hiddenSize + '\n')
		elif (words[0] == 'codeSize'):
			outfile.write(l.strip() + ',' + codeSize + '\n')
		else:
			outfile.write(l)
			if greedy and names[-1] == 'weights' and names[-2] != 'output':
				numWeights = int(words[1])
				plasts = ['0'] * numWeights
				outfile.write('_'.join(names[:-1]) + '_plasticities ' + str(numWeights) + ' ' + ' '.join(plasts) + '\n')
outfile.close()

