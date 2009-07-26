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

#ifndef _INCLUDED_SeqBuffer_h  
#define _INCLUDED_SeqBuffer_h  

#include "MultiArray.hpp"

template <class R> struct CoordIterator
{
	//data
	vector<size_t> shape;
	vector<int> directions;
	vector<int> pt;
	bool end;
		
	//functions
	CoordIterator (const R& shap, const vector<int>& dirs = list_of<int>(), bool reverse = false):
		directions(dirs),
		pt(boost::size(shap)),
		end(false)
	{
		vector_assign(shap, shape);
		directions.resize(shape.size(), 1);
		if (reverse)
		{
			range_multiply_val(directions, -1);
		}
		begin();
	}
	void step(size_t d)
	{
		if (directions[d] > 0)
		{
			if (pt[d] == shape[d] - 1)
			{
				pt[d] = 0;
				if (d)
				{
					step(d-1);
				}
				else
				{
					end = true;
				}
			}
			else
			{
				++pt[d];
			}
		}
		else
		{
			if (pt[d] == 0)
			{
				pt[d] = shape[d]-1;
				if (d)
				{
					step(d-1);
				}
				else
				{
					end = true;
				}
			}
			else
			{
				--pt[d];
			}
		}
	}
	CoordIterator& operator++ ()
	{
		if (shape.size())
		{
			step(shape.size()-1);
		}
		else
		{
			end = true;
		}
		return *this;
	}
	const vector<int>& operator* () const
	{
		return pt;
	}
	void begin()
	{
		for (int i = 0; i < shape.size(); ++i)
		{
			pt[i] = ((directions[i] > 0) ? 0 : shape[i] - 1);
		}
		end = false;
	}
};

#define CVI CoordIterator<const vector<size_t> > 
#define SeqIterator CoordIterator<View<const size_t> >
template <class T> struct SeqBuffer: public MultiArray<T>
{
	//data
	size_t depth;
	
	//functions
	SeqBuffer(size_t dep = 0):
		depth(dep)
	{
		reshape(list_of<size_t>());
	}
	SeqBuffer(const SeqBuffer& sb)
	{
		*this = sb;
	}
	virtual ~SeqBuffer()
	{
	}
	using MultiArray<T>::operator[];
	const View<T> operator[](int coord)
	{
		check(this->shape.size(), " operator [" + str(coord) + "] called for empty SeqBuffer");
		T* start = &this->data[coord * depth];
		return View<T>(start, start + depth);
	}
	const View<const T> operator[](int coord) const
	{
		check(this->shape.size(), " operator [" + str(coord) + "] called for empty SeqBuffer");
		const T* start = &this->data[coord * depth];
		return View<const T>(start, start + depth);
	}
	using MultiArray<T>::at;
	const View<T> at(int coord)
	{
		if (coord >= 0 && (coord * (this->shape.back() < product(this->shape))))
		{
			return (*this)[coord];
		}
		return View<T>();
	}
	const View<const T> at(int coord) const
	{
		if (coord >= 0 && (coord * (this->shape.back() < product(this->shape))))
		{
			return (*this)[coord];
		}
		return View<const T>();
	}
	const View<T> front(const vector<int>& dirs = list_of<int>())
	{
		return (*this)[*begin(dirs)];
	}
	const View<T> back(const vector<int>& dirs = list_of<int>())
	{
		return (*this)[*rbegin(dirs)];
	}	
	SeqIterator begin(const vector<int>& dirs = list_of<int>()) const
	{
		return SeqIterator(seq_shape(), dirs);
	}
	SeqIterator rbegin(const vector<int>& dirs = list_of<int>()) const
	{
		return SeqIterator(seq_shape(), dirs, true);
	}
	const View<const size_t> seq_shape() const
	{
		return View<const size_t>(&this->shape.front(), &this->shape.back());
	}
	size_t seq_size() const
	{
		return product(seq_shape());
	}
 	size_t num_seq_dims() const
	{
		return this->shape.size() - 1;
	}
	template<class T2> void reshape(const SeqBuffer<T2>& buff)
	{
		reshape(buff.seq_shape());
	}
	template<class T2> void reshape(const SeqBuffer<T2>& buff, const T& fillVal)
	{
		reshape(buff.seq_shape(), fillVal);
	}
	template<class R> void reshape(const R& newSeqShap)
	{
		if (depth)
		{
			vector_assign(newSeqShap, this->shape);
			this->shape += depth;
			this->resize_data();
		}
	}
	template<class R> void reshape(const R& newSeqShap, const T& fillval)
	{
		reshape(newSeqShap);
		fill(this->data, fillval);
	}
	template<class R> void reshape_with_depth(const R& newSeqShap, size_t dep)
	{
		depth = dep;
		reshape(newSeqShap);
	}
	template<class R> void reshape_with_depth(const R& newSeqShap, size_t dep, const T& fillval)
	{
		reshape_with_depth(newSeqShap, dep);
		fill(this->data, fillval);
	}
	void print(ostream& out) const
	{
		out << "DIMENSIONS: " << seq_shape() << endl;
		loop(int j, range(this->shape.back()))
		{
			loop(int i, range(seq_size()))
			{
				out << (*this)[i][j] << " ";
			}
			out << endl;
		}
	}
	template<class T2> SeqBuffer<T>& operator =(const SeqBuffer<T2>& a)
	{
		depth = a.depth;
		MultiArray<T>::assign(a);
		return *this;
	}
};

template <class T> static ostream& operator <<(ostream& out, const SeqBuffer<T>& a)
{
	a.print(out);
	return out;
}

#endif
