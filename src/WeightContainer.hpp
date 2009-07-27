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

#ifndef _INCLUDED_WeightContainer_h  
#define _INCLUDED_WeightContainer_h  

#include <vector>
#include <numeric>
#include <algorithm>
#include <functional>
#include <math.h>
#include <boost/tuple/tuple.hpp>
#include "Random.hpp"
#include "DataExporter.hpp"

using namespace std;

typedef multimap<string, tuple<string, string, int, int> >::iterator WC_CONN_IT; 

struct WeightContainer: public DataExporter
{
	//data
	vector<double> weights;
	vector<double> derivatives;
	vector<double> plasticities;
	multimap<string, tuple<string, string, int, int> > connections;
	
	//functions
	WeightContainer():
		DataExporter("weightContainer")
	{
	}
	static WeightContainer& instance()
	{
		static WeightContainer wc;
		return wc;
	}
	void link_layers(const string& fromName, const string& toName, const string& connName = "", int paramBegin = -1, int paramEnd = -1)
	{
		connections.insert(make_pair(toName, make_tuple(fromName, connName, paramBegin, paramEnd)));
	}
	pair<size_t, size_t> new_parameters(size_t numParams, const string& fromName, const string& toName, const string& connName)
	{
		size_t begin = weights.size();
		weights.resize(weights.size() + numParams);
		size_t end = weights.size();
		link_layers(fromName, toName, connName, begin, end);
		return make_pair(begin, end);
	}
	View<double> get_weights(pair<int, int> range)
	{
		return View<double>(&weights[range.first], &weights[range.second]);
	}
	View<double> get_derivs(pair<int, int> range)
	{
		return View<double>(&derivatives[range.first], &derivatives[range.second]);
	}
	View<double> get_plasts(pair<int, int> range)
	{
		return View<double>(&plasticities[range.first], &plasticities[range.second]);
	}
	void randomise(double range)
	{
		loop(double& w, weights)
		{
			if (w == infinity)
			{
				w = Random::normal() * range;
			}
		}
	}
	void reset_derivs()
	{
		fill(derivatives, 0);
	}
	void save_by_conns(vector<double>& container, const string& nam)
	{
		for (WC_CONN_IT it = connections.begin(); it != connections.end(); ++it)
		{
			VDI begin = container.begin() + it->second.get<2>();
			VDI end = container.begin() + it->second.get<3>();
			if (begin != end)
			{
				save_range(make_pair(begin, end), it->second.get<1>() + "_" + nam);
			}
		}
	}
	//MUST BE CALLED BEFORE WEIGHT CONTAINER IS USED
	void build()
	{
		fill(weights, infinity);
		plasticities.resize(weights.size());
		derivatives.resize(weights.size());
		fill(plasticities, 1);
		save_by_conns(weights, "weights");
		save_by_conns(plasticities, "plasticities");
	}
};

#endif
