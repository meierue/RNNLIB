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

#ifndef _INCLUDED_SamplingLayer_h  
#define _INCLUDED_SamplingLayer_h  

#include "Layer.hpp"

struct SamplingLayer: public Layer
{
	//functions
	SamplingLayer(Layer* src):
		Layer(src->name + "_sample", src->num_seq_dims(), 0, src->output_size(), src)
	{
		//TODO assert that source has activations bounded between 0 and 1 (logistic or lstm)
		display(this->outputActivations, "activations");
		display(this->outputErrors, "errors");
	}
	~SamplingLayer(){}
	void feed_forward(const vector<int>& coords)
	{
		loop(TDD t, zip(this->outputActivations[coords], this->source->outputActivations[coords]))
		{
			t.get<0>() = (Random::uniform() < t.get<1>()) ? 1 : 0;
		}
	}
	void feed_back(const vector<int>& coords)
	{
		loop(TDDDD t, zip(this->source->outputErrors[coords], this->outputErrors[coords], 
						  this->outputActivations[coords], this->source->outputActivations[coords]))
		{
			t.get<0>() = t.get<1>() + (t.get<2>() ? t.get<3>() : 1 - t.get<3>());
		}
	}
};

#endif
