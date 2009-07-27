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

#ifndef _INCLUDED_MemoryNet_h  
#define _INCLUDED_MemoryNet_h  

#include "Mdrnn.hpp"

typedef tuple<Layer*, Layer*> TLL;

struct MemoryNet: public Mdrnn
{
	//data
	vector<FullConnection*> memoryConns;
	
	//functions
	MemoryNet(ostream& out, ConfigFile& conf, const DataHeader& data):
		Mdrnn(out, conf, data)
	{
		assert(this->num_seq_dims());
		int hiddenSize = conf.get<int>("hiddenSize");
		string hiddenType = conf.get<string>("hiddenType", "lstm");
		Layer* in = this->get_input();
		this->add_hidden_level(hiddenType, hiddenSize, true, "encoder");
		this->add_hidden_level(hiddenType, hiddenSize, false, "decoder", false);
		loop(TLL p, zip(this->hiddenLevels[0], this->hiddenLevels[1]))
		{
			Layer* enc = p.get<0>();
			Layer* dec = p.get<1>();
			dec->source = enc;
			assert(this->copy_connections(enc, dec) == this->num_seq_dims() + 1);
			WeightContainer::instance().link_layers(enc->name, dec->name);
		}		
		this->outputLayer = new AutoregressionLayer("output", in);
		this->add_bias(this->outputLayer);
		this->connect_to_hidden_level(in, 0);
		this->connect_from_hidden_level(1, this->outputLayer);
	}
	virtual ~MemoryNet()
	{
	}
	void feed_forward(const DataSequence& seq)
	{
		inputLayer->copy_inputs(seq.inputs);
		if (inputBlockLayer)
		{
			this->feed_forward_layer(inputBlockLayer);
		}
		loop(Layer* enc, this->hiddenLevels[0])
		{
			this->feed_forward_layer(enc);
		}
		loop(TLL p, zip(this->hiddenLevels[0], this->hiddenLevels[1]))
		{
			Layer* enc = p.get<0>();
			Layer* dec = p.get<1>();
			dec->start_sequence();
			copy(enc->outputActivations.back(enc->directions), dec->outputActivations.front(dec->directions));
			LstmLayer<Tanh, Tanh, Logistic>* lenc = dynamic_cast<LstmLayer<Tanh, Tanh, Logistic>*>(enc);
			LstmLayer<Tanh, Tanh, Logistic>* ldec = dynamic_cast<LstmLayer<Tanh, Tanh, Logistic>*>(dec);
			if (lenc && ldec)
			{
				copy(lenc->states.back(lenc->directions), ldec->states.front(ldec->directions));
			}
			pair<CONN_IT, CONN_IT> connRange = connections.equal_range(dec);
			for (SeqIterator it = ++dec->input_seq_begin(); !it.end; ++it)
			{
				loop (PLC c, connRange)
				{
					c.second->feed_forward(*it);
				}
				dec->feed_forward(*it);
			}
		}		
		feed_forward_layer(outputLayer);
	}
	void feed_back()
	{
		feed_back_layer(this->outputLayer);
		loop(Layer* dec, this->hiddenLevels[1])
		{
			pair<CONN_IT, CONN_IT> connRange = connections.equal_range(dec);
			vector<int> first = *dec->input_seq_begin();
			for (SeqIterator it = dec->input_seq_rbegin(); !it.end; ++it)
			{
				dec->feed_back(*it);
				if (*it != first)
				{
					loop (PLC c, connRange)
					{
						c.second->feed_back(*it);
					}
				}
			}
			for (SeqIterator it = dec->input_seq_rbegin(); !it.end; ++it)
			{
				dec->update_derivs(*it);
				if (*it != first)
				{
					loop (PLC c, connRange)
					{
						c.second->update_derivs(*it);
					}
				}
			}
		}
		loop(TLL p, zip(this->hiddenLevels[0], this->hiddenLevels[1]))
		{
			Layer* enc = p.get<0>();
			Layer* dec = p.get<1>();
			copy(dec->inputErrors.front(dec->directions), enc->inputErrors.back(enc->directions));
			LstmLayer<Tanh, Tanh, Logistic>* lenc = dynamic_cast<LstmLayer<Tanh, Tanh, Logistic>*>(enc);
			LstmLayer<Tanh, Tanh, Logistic>* ldec = dynamic_cast<LstmLayer<Tanh, Tanh, Logistic>*>(dec);
			if (lenc && ldec)
			{
				copy(ldec->cellErrors.front(ldec->directions), lenc->cellErrors.back(lenc->directions));
				copy(ldec->forgetGateActs.front(ldec->directions), lenc->forgetGateActs.back(lenc->directions));
			}
			pair<CONN_IT, CONN_IT> connRange = connections.equal_range(enc);
			bool first = true;
			for (SeqIterator it = ++enc->input_seq_rbegin(); !it.end; ++it)
			{
				if (!first)
				{
					enc->feed_back(*it);
				}
				first = false;
				loop (PLC c, connRange)
				{
					c.second->feed_back(*it);
				}
			}
			first = true;
			for (SeqIterator it = enc->input_seq_rbegin(); !it.end; ++it)
			{
				if (!first)
				{
					enc->update_derivs(*it);
				}
				first = false;
				loop (PLC c, connRange)
				{
					c.second->update_derivs(*it);
				}
			}
		}		
	}
	double calculate_errors(const DataSequence& seq)
	{
		feed_forward(seq);
		return outputLayer->calculate_errors(seq);
	}
	double train(const DataSequence& seq)
	{
		double err = calculate_errors(seq);
		feed_back();
		return err;
	}
	void print(ostream& out = cout) const
	{
		Mdrnn::print(out);
	}
};

#endif
