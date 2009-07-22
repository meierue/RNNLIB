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

#ifndef _INCLUDED_GradientCheck_h
#define _INCLUDED_GradientCheck_h

#include "WeightContainer.h"
#include "Mdrnn.h"

extern bool runningGradTest;

struct GradientCheck
{
	//data
	ostream& out;
	map<string, bool> checked;
	Mdrnn* net;
	const DataSequence& seq;
	double perturbation;
	unsigned sigFigs;
	vector<double>& weights;
	vector<double>& derivs;
	bool verbose;
	multimap<string, tuple<string, string, int, int> >& conns;
				
	//functions
	GradientCheck(ostream& o, Mdrnn* n, const DataSequence& s, unsigned sf = 6, double pert = 1e-5, bool verb = false):
			out(o),
			net(n),
			seq(s),
			perturbation(pert),
			sigFigs(sf),
			weights(WeightContainer::instance().weights),
			derivs(WeightContainer::instance().derivatives),
			verbose(verb),
			conns(WeightContainer::instance().connections)
	{		
		runningGradTest = true;
		PRINT (perturbation, out);
		PRINT (sigFigs, out);
		PRINT (verbose, out);
		prt_line(out);
		out << "calculating algorithmic pds" << endl;
		net->train(seq);
		out << "checking against numeric pds" << endl;
		if (check_layer(net->outputLayer->name))
		{
			out << "GRADIENT CHECK SUCCESSFUL!" << endl;
		}
		else
		{
			out << "GRADIENT CHECK FAILED!" << endl;
			exit(0);
		}
		runningGradTest = false;
	}
	bool check_layer(const string& name)
	{
		if (!checked[name])
		{
			checked[name] = true;
			pair<WC_CONN_IT, WC_CONN_IT> range = conns.equal_range(name);
			if (range.first != range.second)
			{
				prt_line(out);
				out << "checking layer " << name << endl;
			}
			for (WC_CONN_IT it = range.first; it != range.second; ++it)
			{
				if (!check_connection(it->second.get<1>(), it->second.get<2>(), it->second.get<3>()))
				{
					return false;
				}
			}
			for (WC_CONN_IT it = range.first; it != range.second; ++it)
			{
				if (!check_layer(it->second.get<0>()))
				{
					return false;
				}
			}
		}
		return true;
	}
	bool check_connection(const string& name, int begin, int end)
	{
		if (begin == end)
		{
			return true;
		}
		out << "checking connection " << name << endl;
		for (int i = begin; i < end; ++i)
		{
			//store original weight
			double oldWt = weights[i];
	
			//add positive perturbation and compute error
			weights[i] += perturbation;
			double plusErr = net->calculate_errors(seq);
	
			//add negative perturbation and compute error
			weights[i] = oldWt - perturbation;
			double minusErr = net->calculate_errors(seq);
	
			//store symmetric difference
			double numericDeriv = (plusErr-minusErr)/(2*perturbation);
	
			//restore original weight
			weights[i] = oldWt;
			
			double algoDeriv = derivs[i];
			int index = i - begin;
			if (verbose)
			{
				out << "weight " << index << " numeric deriv " << numericDeriv << " algorithmic deriv " << algoDeriv << endl;
			}
			double threshold = pow(10.0, max(0.0, ceil(log10(min(fabs(algoDeriv), fabs(numericDeriv)))))-(int)sigFigs);
			if (fabs(numericDeriv - algoDeriv) > threshold)
			{
				if(!verbose)
				{
					out << "weight " << index << " numeric deriv " << numericDeriv << " algorithmic deriv " << algoDeriv << endl;
				}
				return false;
			}
		}
		return true;
	}
};

#endif
