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

#ifndef _INCLUDED_LogDouble_h  
#define _INCLUDED_LogDouble_h

#include <math.h>
#include <iostream>

using namespace std;

static const double expLimit = log(numeric_limits<double>::max());
static const double negExpLimit = log(numeric_limits<double>::min());
static const double logZero = -infinity;

static double safe_exp (double x)
{
	if (x == logZero)
	{
		return 0;
	}
	else if (x > expLimit)
	{
		return doubleMax;
	}
	return exp(x);
}

static double safe_log(double x)
{
	if (x < doubleMin)
	{
		return logZero;
	}
	return log(x);
}
static double log_add(double x, double y)
{
	if (x == logZero)
	{
		return y;
	}
	else if (y == logZero)
	{
		return x;
	}
	if (x < y) 
	{
		swap(x, y);
	}
	double diff = y - x;
	if (diff < negExpLimit)
	{
		return x;
	}
	return x + log(1.0 + safe_exp(diff));
}
static double log_subtract(double x, double y)
{
	if (y == logZero)
	{
		return x;
	}	
	assert (x >= y);
	double diff = y - x;
	if (diff < negExpLimit)
	{
		return x;
	}
	return x + log(1.0 - safe_exp(diff));
}
static double log_multiply(double x, double y)
{
	if (x == logZero || y == logZero)
	{
		return logZero;
	}
	return x + y;
}
static double log_divide(double x, double y)
{
	if (x == logZero)
	{
		return logZero;
	}
	assert(y != logZero);
	return x - y;
}
class LogDouble
{
	//data
	double expVal;

public:
	
	//data
	double log;
	
	//functions
	LogDouble(double v = 0, bool logScale = false):
		expVal(logScale ? -1 : v),
		log(logScale ? v : safe_log(v))
	{
	}
	LogDouble& operator = (const LogDouble& l)
	{
		log = l.log;
		expVal = l.expVal;
		return *this;
	}
	LogDouble& operator += (const LogDouble& l)
	{
		log = log_add(log, l.log);
		expVal = -1;
		return *this;
	}
	LogDouble& operator -= (const LogDouble& l)
	{
		log = log_subtract(log, l.log);
		expVal = -1;
		return *this;
	}
	LogDouble& operator *= (const LogDouble& l)
	{
		log = log_multiply(log, l.log);
		expVal = -1;
		return *this;
	}
	LogDouble& operator /= (const LogDouble& l)
	{
		log = log_divide(log, l.log);
		expVal = -1;
		return *this;
	}
	double exp()
	{
		if (expVal < 0)
		{
			expVal = safe_exp(log);
		}
		return expVal; 
	}
};
LogDouble operator+ (LogDouble log1, LogDouble log2)
{
	return LogDouble(log_add(log1.log, log2.log), true);
}
LogDouble operator- (LogDouble log1, LogDouble log2)
{
	return LogDouble(log_subtract(log1.log, log2.log), true);
}
LogDouble operator* (LogDouble log1, LogDouble log2)
{
	return LogDouble(log_multiply(log1.log, log2.log), true);
}
LogDouble operator/ (LogDouble log1, LogDouble log2)
{
	return LogDouble(log_divide(log1.log, log2.log), true);
}
bool operator> (LogDouble log1, LogDouble log2)
{
	return (log1.log > log2.log);
}
bool operator< (LogDouble log1, LogDouble log2)
{
	return (log1.log < log2.log);
}
bool operator== (LogDouble log1, LogDouble log2)
{
	return (log1.log == log2.log);
}
bool operator<= (LogDouble log1, LogDouble log2)
{
	return (log1.log <= log2.log);
}
bool operator>= (LogDouble log1, LogDouble log2)
{
	return (log1.log >= log2.log);
}
ostream& operator << (ostream& out, const LogDouble& l)
{
	out << (l.log == logZero ? 0 : l.log);
	return out;
}
istream& operator >> (istream& in, LogDouble& l)
{
	double d;
	in >> d;
	l = LogDouble(d, true);
	return in;
}

typedef const tuple<double&, LogDouble&, LogDouble&>& TDLL;
typedef const tuple<double&, LogDouble&>& TDL;

#endif
