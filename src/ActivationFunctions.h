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

#ifndef _INCLUDED_ActivationFunctions_h  
#define _INCLUDED_ActivationFunctions_h  

#include <numeric>
#include <limits>
#include <boost/array.hpp>
#include "LogDouble.h"

using namespace std;
using namespace boost;

//#define FAST_LOGISTIC

#ifdef FAST_LOGISTIC
static const unsigned int entries = 1 << 16;
static array<double, entries> lookup;
#endif

struct Logistic
{
#ifdef FAST_LOGISTIC
	static const int maximum = 16;
	static const int minimum = -maximum;
	static const int range = maximum - minimum;
	static const int entriesDivRange = entries / range;
	static void fill_lookup()
	{
		for (int i = 0; i < entries; ++i)
		{
			double x = (double)minimum + ((double)((i + 0.5) * range) / (double) entries);
			lookup[i] = (1.0 / (1.0 + exp(-x)));
		}
	}
#endif
	static double fn(double x)
	{
#ifdef FAST_LOGISTIC
		if (runningGradTest)
		{
#endif
			if (x < -expLimit)
			{
				return 1;
			}
			else if (x > expLimit)
			{
				return 0;
			}
			return 1.0 / (1.0 + exp(-x));
#ifdef FAST_LOGISTIC
		}
		else
		{
			if (x <= minimum)
			{
				return 0;
			}
			else if (x >= maximum)
			{
				return 1;
			}
			int box = (x + maximum) * entriesDivRange;
			return lookup[box];
		}
#endif
	}
	static double deriv(double y)
	{
		return y*(1.0-y); 
	}
};
struct Identity
{
	static double fn(double x)
	{
		return x;
	}
	static double deriv(double y)
	{
		return 1;
	}
};
struct Maxmin2
{
	static double fn(double x)
	{
		return (4 * Logistic::fn(x)) - 2;
	}
	static double deriv(double y)
	{
		if (y==-2 || y==2)
		{
			return 0;
		}
		return (4 - (y * y)) / 4.0;
	}
};
struct Maxmin1
{
	static double fn(double x)
	{
		return (2 * Logistic::fn(x)) - 1;
	}
	static double deriv(double y)
	{
		if (y==-1 || y==1)
		{
			return 0;
		}
		return (1.0 - (y * y)) / 2.0;
	}
};
struct Max2min0
{
	static double fn(double x)
	{
		if (x < -expLimit)
		{
			x = -expLimit;
		}
		else if (x > expLimit)
		{ 
			x = expLimit;
		}
		return (2 * Logistic::fn(x));
	}
	static double deriv(double y)
	{
		if (y==-1 || y==1)
		{
			return 0;
		}
		return y * (1 - (y / 2.0));
	}
};
struct Tanh
{
	static double fn(double x)
	{
		return Maxmin1::fn(2*x);
	}
	static double deriv(double y)
	{
		return 1.0 - (y *  y); 
	}
};

#endif
