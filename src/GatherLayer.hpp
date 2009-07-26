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

#ifndef _INCLUDED_GatherLayer_h  
#define _INCLUDED_GatherLayer_h  

#include "Layer.h"

struct GatherLayer: public Layer
{
	//data
	vector<Layer*> sources;
		
	//functions
	GatherLayer(const string& name, vector<Layer*>& srcs):
		Layer(name, srcs.front()->num_seq_dims(), 0, get_size(srcs), srcs.front()),
		sources(srcs)
	{
		this->source = sources.front();
		WeightContainer::instance().new_parameters(0, this->source->name, this->name, this->source->name + "_to_" + this->name);
		display(this->outputActivations, "activations");
		display(this->outputErrors, "errors");
	}
	int get_size(vector<Layer*>& srcs)
	{
		int size = 0;
		for (int i = 0; i < srcs.size(); ++i)
		{
			size += srcs[i]->output_size();
		}
		return size;
	}
	void feed_forward(const vector<int>& outCoords)
	{
		double* actBegin = this->outputActivations[outCoords].begin();
		loop(Layer* l, sources)
		{
			View<double> inActs = l->outputActivations[outCoords];
			copy(inActs.begin(), inActs.end(), actBegin);
			actBegin += inActs.size();
		}
	}
	void feed_back(const vector<int>& outCoords)
	{
		double* errBegin = this->outputErrors[outCoords].begin();
		loop(Layer* l, sources)
		{
			View<double> inErrs = l->outputErrors[outCoords];
			int dist = inErrs.size();
			copy(errBegin, errBegin + dist, inErrs.begin());
			errBegin += dist;
		}
	}
};

#endif
