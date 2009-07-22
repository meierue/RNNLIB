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

#ifndef _INCLUDED_PmNet_h  
#define _INCLUDED_PmNet_h  

#include "Mdrnn.h"

struct PmNet: public Mdrnn
{	
	//data
	FullConnection* outputConn;
	bool minimizationMode;
	
	//functions
	PmNet(ostream& out, ConfigFile& conf, const DataHeader& data):
		Mdrnn(out, conf, data),
		minimizationMode(conf.get<bool>("minimize"))
	{
		int hiddenSize = conf.get<int>("hiddenSize");
		string hiddenType = conf.get<string>("hiddenType", "lstm");
		Layer* hidden = this->add_hidden_layer(hiddenType, hiddenSize, list_of<int>().repeat(this->num_seq_dims(), 1));
		this->outputLayer = new AutoregressionLayer("output", hidden);
		this->add_bias(this->outputLayer);
		this->connect_layers(this->get_input(), hidden);
		outputConn = this->connect_layers(hidden, this->outputLayer);
	}
	virtual ~PmNet()
	{
	}
	void feed_back()
	{
		feed_back_layer(outputLayer);
		if (minimizationMode)
		{
			fill(outputConn->derivs(), 0.0);
			loop_back(Layer* layer, hiddenLayers)
			{
				feed_back_layer(layer);
			}
		}
	}
	double calculate_errors(const DataSequence& seq)
	{
		feed_forward(seq);
		double err = outputLayer->calculate_errors(seq);
		if (minimizationMode)
		{
			err = -err;
			outputLayer->errorMap["sumSquaresError"] *= -1;
			outputLayer->errorMap["seqRmsError"] *= -1;
			range_negate(outputLayer->outputErrors.data);
		}
		return err;
	}
	double train(const DataSequence& seq)
	{
		double err = calculate_errors(seq);
		feed_back();
		return err;
	}
 	void build()
 	{
		Mdrnn::build();
		assert(squared(outputLayer->output_size()) == outputConn->num_weights());
		double* wts = outputConn->weights().begin();
		double* plasts = outputConn->plasts().begin();
		for (int i = 0; i < outputConn->num_weights(); i += outputLayer->output_size() + 1)
		{
			wts[i] = 0;
			plasts[i] = 0;
		}
	}
};

#endif
