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

#ifndef _INCLUDED_ConfigFile_h  
#define _INCLUDED_ConfigFile_h  

#include <iostream>
#include <map>
#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>
#include "Helpers.hpp"

using namespace std;

struct ConfigFile
{
	//data
	map<string, string> params;
	string filename;
	
	//functions
	ConfigFile(const string& fname, char readLineChar = '_'):
		filename(fname)
	{
		ifstream instream(filename.c_str());
		check(instream.is_open(), "could not open config file \"" + filename + "\"");
		string name;
		string val;
		while(instream >> name && instream >> val)
		{
			string line;
			getline(instream, line);
			if(name[0] != '#')
			{
				if (in(name, readLineChar) && line.size() > 1)
				{
					val += line;
				}
				params[name] = val;
			}
		}
	}
	bool has(const string& name)
	{
		return in(params, name);
	}
	template<class T> const T& set(const string& name, const T& val)
	{
		stringstream ss;
		ss << boolalpha << val;
		params[name] = ss.str();
		return val;
	}
	template<class T> T get(const string& name, const T& defaultVal)
	{
		MSSI it = params.find(name);
		if (it == params.end())
		{	
			set<T>(name, defaultVal);
			return defaultVal;
		}
		return get<T>(name);
	}
	template<class T> T get(const string& name) const
	{
		MSSCI it = params.find(name);
		if (it == params.end())
		{	
			cout << "param '" << name << "' not found in config file '" << filename << "', exiting" << endl;
			exit(0);
		}
		return read<T>(it->second);
	}
	template<class T> vector<T> get_list(const string& name, const char delim = ',') const
	{
		vector<T> vect;
		MSSCI it = params.find(name);
		if (it != params.end())
		{
			vect = split<T>(it->second, delim);
		}
		return vect;
	}
	template<class T> vector<T> get_list(const string& name, const T& defaultVal, size_t length, const char delim = ',') const
	{
		vector<T> vect = get_list<T>(name, delim);
		vect.resize(length, vect.size() == 1 ? vect.front() : defaultVal);
		return vect;
	}
	template<class T> vector<vector<T> > get_array(const string& name, const char delim1 = ';', const char delim2 = ',') const
	{
		vector<vector<T> > vect;
		MSSCI it = params.find(name);
		if (it != params.end())
		{
			vector<string> lists = split<string>(it->second, delim1);
			for (VSCI it = lists.begin(); it != lists.end(); ++it)
			{
				vect.push_back(split<T>(*it, delim2));
			}
		}
		return vect;
	}
};

static ostream& operator << (ostream& out, const ConfigFile& conf)
{
	out << conf.params;
	return out;
}

#endif
