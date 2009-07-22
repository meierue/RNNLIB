/*Copyright 2009 Alex Graves

This file is part of rnn_lib.

rnn_lib is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

rnn_lib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with rnn_lib.  If not, see <http://www.gnu.org/licenses/>.*/

#ifndef _INCLUDED_RegressionLayer_h  
#define _INCLUDED_RegressionLayer_h  

#include "LinearOutputLayer.h"
#include "Helpers.h"

struct RegressionLayer: public LinearOutputLayer
{
	//data
	ostream& out;
	SeqBuffer<double> targets;
	
	//functions
	RegressionLayer(ostream& o, const string& name, size_t numSeqDims, size_t size):
		LinearOutputLayer(name, numSeqDims, size),
		out(o),
		targets(size)
	{
		this->criteria = list_of("sumSquaresError");
		DISPLAY(targets);
	}
	double calculate_errors(const DataSequence& seq)
	{
		check(equal(this->outputActivations.shape, seq.targetPatterns.shape), 
			  "output shape = " + str(this->outputActivations.shape) 
				+ ", target shape = " + str(seq.targetPatterns.shape));
		targets = seq.targetPatterns;
		double sumSquaresError = 0;
		loop(int i, range(this->outputActivations.seq_size()))
		{
// 			double errScale = (seq.importance.empty() ? 1 : *(seq.importance[i].first));
			loop(TDDCF t, zip(this->outputErrors[i], this->outputActivations[i], seq.targetPatterns[i]))
			{
				double error = t.get<1>() - t.get<2>();// * errScale;
				t.get<0>() = error;
				sumSquaresError += error * error;
			}
		}
		if (verbose && (this->outputActivations.seq_size() == 1)) 
		{
			out << "target pattern: " << seq.targetPatterns.data << endl;
			out << "output pattern: " << this->outputActivations.data << endl;
		}
		sumSquaresError *= 0.5;
		this->errorMap["sumSquaresError"] = sumSquaresError;
		return sumSquaresError;
	}	
};

#endif
