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

#ifndef _INCLUDED_ObjectStore_h  
#define _INCLUDED_ObjectStore_h  

template<class T> class ObjectStore
{
		
	vector<T*> store;
		
public:
		
	void delete_object(T* object)
	{
		store.push_back(object);
	}
	T* new_object()
	{
		T* object;
		if (store.empty())
		{
			object = new T;
		}
		else
		{
			object = store.back();
			store.pop_back();
		}
		return object;
	}
	T* copy(T* tOld)
	{
		if (tOld)
		{
			T* tNew = new_object();
			*tNew = *tOld;
			return tNew;
		}
		else
		{
			return 0;
		}
	}
	~ObjectStore()
	{
		delete_range(store);
	}
};

#endif
