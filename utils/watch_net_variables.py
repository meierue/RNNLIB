#!/usr/bin/python

import time
from optparse import OptionParser
import commands

#command line options
usage = "usage: %prog [options] save_file"
parser = OptionParser(usage)
parser.add_option("-s", "--seq", dest="seq", default="test,0", action="store", type="string", help="which sequence to watch?")
parser.add_option("-v", "--vars", dest="vars", default="input_activations,output_outputActivations,output_inputErrors", action="store", type="string", help="which variables to watch?")
parser.add_option("-d", "--delay", dest="delay", default=60, action="store", type=int, help="delay (in secs) between redraws")

#parse command line options
(opt, args) = parser.parse_args()
if len(args) != 1:
        parser.error("incorrect number of arguments")
savefile = args[0]
dirstr = 'display_' + savefile + '_' + '_'.join(opt.seq.split(',')) + '/'
cmdstr = 'disp_seq_boost.sh ' + savefile + ' ' + ' '.join(opt.seq.split(','))
variables = opt.vars.split(',')
print cmdstr
print dirstr
print variables
while True:
	print commands.getoutput(cmdstr)
	for v in variables:
		print commands.getoutput('nohup plt_boost.py ' + dirstr + v)
	time.sleep(opt.delay)
