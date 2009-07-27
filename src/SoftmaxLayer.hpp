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

#ifndef _INCLUDED_SoftmaxLayer_h  
#define _INCLUDED_SoftmaxLayer_h  

#include <boost/algorithm/minmax_element.hpp>
#include "OutputLayer.hpp"

struct SoftmaxLayer: public OutputLayer
{
	//data
	SeqBuffer<LogDouble> logActivations;
	SeqBuffer<LogDouble> unnormedlogActivations;
	SeqBuffer<double> unnormedActivations;
	
	//functions
	SoftmaxLayer(const string& name, size_t numSeqDims, size_t size):
		OutputLayer(name, numSeqDims, size),
		logActivations(size),
		unnormedlogActivations(size),
		unnormedActivations(size)
	{
	}
	void start_sequence()
	{
		OutputLayer::start_sequence();
		logActivations.reshape(this->inputActivations);
		unnormedlogActivations.reshape(logActivations);
		unnormedActivations.reshape(logActivations);
	}
	void feed_forward(const vector<int>& coords)
	{	
		//transform to log scale
		View<LogDouble> unnormedLogActs = unnormedlogActivations[coords];
		loop(TDL t, zip(this->inputActivations[coords], unnormedLogActs))
		{
			t.get<1>() = LogDouble(t.get<0>(), true);
		}
		
		//apply exponential
		View<double> unnormedActs = unnormedActivations[coords];
		transform(unnormedLogActs, unnormedActs, mem_fun_ref(&LogDouble::exp));
		
		//normalise
		double Z = sum(unnormedActs);
		range_divide_val(this->outputActivations[coords], unnormedActs, Z);
		range_divide_val(logActivations[coords], unnormedLogActs, LogDouble(Z));
	}
	void feed_back(const vector<int>& coords)
	{
		View<double> outActs = this->outputActivations[coords];
		View<double> outErrs = this->outputErrors[coords];
		double Z = inner_product(outActs, outErrs);
		loop(TDDD t, zip(this->inputErrors[coords], outActs, outErrs))
		{
			t.get<0>() = t.get<1>() * (t.get<2>() - Z);
		}
	}	
};

#endif
