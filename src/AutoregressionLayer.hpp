/*Copyright 2009 Alex Graves

This file is part of RNNLIB.

RNNLIB is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

RNNLIB is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with RNNLIB.  If not, see <http://www.gnu.org/licenses/>.*/

#ifndef _INCLUDED_AutoregressionLayer_h  
#define _INCLUDED_AutoregressionLayer_h  

#include "RegressionLayer.hpp"

struct AutoregressionLayer: public LinearOutputLayer
{
	//data
	SeqBuffer<double> targets;
	vector<double> seqMeanTargs;
			
	//functions
	AutoregressionLayer(const string& name, Layer* source):
		LinearOutputLayer(name, source->num_seq_dims(), source->output_size()),
		targets(this->output_size())
	{
		seqMeanTargs.resize(this->output_size());
		this->source = source;
		this->criteria = list_of("seqRmsError")("sumSquaresError");
		DISPLAY(targets);
		display(this->outputActivations, "activations");
	}
	double calculate_errors(const DataSequence& seq)
	{
		assert(equal(this->outputActivations.shape.begin(), this->outputActivations.shape.end(), this->source->outputActivations.shape.begin(), greater_equal<size_t>()));
		targets = this->source->outputActivations;
		fill (seqMeanTargs, 0);
		int seqSize = targets.seq_size();
		loop(int i, ::range(seqSize))
		{
			range_plus_equals(seqMeanTargs, targets[i]);
		}
		range_divide_val(seqMeanTargs, seqSize);
		double seqRmsNormFactor = 0;
		double rmsError = 0;
		loop(int i, ::range(seqSize))
		{
			seqRmsNormFactor += euclidean_squared(seqMeanTargs, targets[i]);
			loop(TDDD t, zip(this->outputErrors[i], this->outputActivations[i], targets[i]))
			{
				double error = t.get<1>() + t.get<2>();
				t.get<0>() = error;
				rmsError += error * error;
			}
		}
		double sumSquaresErr = 0.5 * rmsError;
		if (seqRmsNormFactor)
		{
			this->errorMap["seqRmsError"] = rmsError / seqRmsNormFactor;
		}
		this->errorMap["sumSquaresError"] = sumSquaresErr;
		return sumSquaresErr;
	}
};

#endif
