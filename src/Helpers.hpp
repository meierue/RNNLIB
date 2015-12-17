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

#ifndef _INCLUDED_Helpers_h  
#define _INCLUDED_Helpers_h  

#include <boost/date_time.hpp>
#include <boost/date_time/local_time/local_time.hpp>
#include <boost/array.hpp>
#include <boost/timer.hpp>
#include <boost/assign/std/vector.hpp>
#include <boost/range.hpp>
#include <boost/iterator/counting_iterator.hpp>
#include <boost/iterator/zip_iterator.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/minmax_element.hpp>
#include <boost/bimap.hpp>
#include <boost/foreach.hpp>
#include <math.h>
#include <numeric>
#include <utility>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <sstream>
#include <list>
#include <set>
#include <algorithm>
#include <iterator>
#include <map>
#include <assert.h>
#include "list_of.hpp"

using namespace std;
using namespace boost;
using namespace boost::assign;
using namespace boost::posix_time;
using namespace boost::gregorian;

#define loop BOOST_FOREACH
#define loop_back BOOST_REVERSE_FOREACH

typedef vector<size_t>::const_iterator VSTCI;
typedef vector<double>::iterator VDI;
typedef vector<double>::const_iterator VDCI;
typedef vector<double>::reverse_iterator VDRI;
typedef string::iterator SI;
typedef string::const_iterator SCI;
typedef vector<int>::iterator VII;
typedef vector<string>::iterator VSI;
typedef vector<string>::const_iterator VSCI;
typedef vector<int>::reverse_iterator VIRI;
typedef vector<vector<int> >::reverse_iterator VVIRI;
typedef vector<int>::const_iterator VICI;
typedef vector<bool>::iterator VBI;
typedef vector<float>::iterator VFI;
typedef vector<float>::const_iterator VFCI;
typedef vector<vector<double> >::iterator VVDI;
typedef vector<vector<double> >::const_iterator VVDCI;
typedef vector<vector<int> >::iterator VVII;
typedef vector<vector<int> >::const_iterator VVICI;
typedef vector<unsigned int>::iterator VUII;
typedef vector<vector<float> >::iterator VVFI;
typedef map<string, string>::iterator MSSI;
typedef map<string, string>::const_iterator MSSCI;
typedef map<string, double>::iterator MSDI;
typedef map<string, double>::const_iterator MSDCI;
typedef map<string, pair<int,double> >::iterator MSPIDI;
typedef map<string, pair<int,double> >::const_iterator MSPIDCI;
typedef vector< map<string, pair<int,double> > >::const_iterator VMSDCI;
typedef vector<map<string, pair<int,double> > >::iterator VMSDI;
typedef vector<map<string, pair<int,double> > >::reverse_iterator VMSDRI;
typedef map<string, int>::iterator MSII;
typedef map<string, int>::const_iterator MSICI;
typedef map<int, int>::iterator MIII;
typedef map<int, int>::const_iterator MIICI;
typedef vector<vector<int> >::const_reverse_iterator VVIRCI;
typedef vector<int>::const_reverse_iterator VIRCI;
typedef vector<const float*>::const_iterator VPCFCI;
typedef vector<const float*>::iterator VPCFI;
typedef vector<const float*>::const_reverse_iterator VPCFCRI;
typedef vector<bool>::const_iterator VBCI;
typedef vector<bool>::iterator VBI;
typedef map <string, pair<double, int> >::iterator MCSPDII;
typedef map <string, pair<double, int> >::const_iterator MCSPDICI;
typedef bimap<int, string>::left_const_iterator BMISLCI;
typedef bimap<int, string>::right_const_iterator BMISRCI;
typedef bimap<int, string>::relation BMISR;
typedef pair<string, double> PSD;
typedef pair<int, int> PII;
typedef pair<const string, double> PCSD;
typedef pair<string, int> PSI;
typedef pair<string, string> PSS;
typedef const tuple<double&, double&, double&, double&>& TDDDD;
typedef const tuple<double&, double&, double&, double&, double&>& TDDDDD;
typedef const tuple<double&, double&, double&>& TDDD;
typedef const tuple<double&, double&, int&>& TDDI;
typedef const tuple<double&, double&, float&>& TDDF;
typedef const tuple<double&, double&, float>& TDDCF;
typedef const tuple<double&, double&>& TDD;
typedef const tuple<string, int>& TSI;
typedef const tuple<int, int>& TII;
typedef const tuple<int, set<int>&>& TISETI;

//global variables
static const double doubleMax = numeric_limits<double>::max();
static const double doubleMin = numeric_limits<double>::min();
static const double infinity = numeric_limits<double>::infinity();
static bool runningGradTest = false;
static bool verbose = false;
static ostream& COUT = cout;

#define PRINT(x, o) ((o) << boolalpha << #x " = " << (x) << endl)
#define PRINTN(x, o) (o) << boolalpha << #x ":" << endl; print_range((o), (x), string("\n")); (o) << endl
#define PRT(x) PRINT(x, cout)
#define PRTN(x) PRINTN(x, cout)
#define PRINTR(x, o) (o) << boolalpha << #x " = "; print_range((o), (x)); (o) << endl
#define PRTR(x) PRINTR(x, cout)
#define check(condition, str)  if(!(condition)) {cout << "ERRROR: " << (str) << endl; assert((condition));}

//MISC FUNCTIONS
static bool warn (bool condition, ostream& out, const string& str)
{
	if (!condition)
	{
		out << "WARNING: " << str << endl;
	}
	return condition;
}
static void print_time(double totalSeconds, ostream& out = cout, bool abbrv = false)
{
	int wholeSeconds = floor(totalSeconds);
	int seconds = wholeSeconds % 60;
	int totalMinutes = wholeSeconds / 60;
	int minutes = totalMinutes % 60;
	int totalHours = totalMinutes / 60;
	int hours = totalHours % 24;
	int totalDays = totalHours / 24;
	int days = totalDays % 365;
	if (days)
	{
		out << days << " day";
		if (days > 1)
		{
			out << "s";
		}
		out << " ";
	}
	if (hours)
	{
		out << hours << (abbrv ? " hr" : " hour");
		if (hours > 1)
		{
			out << "s";
		}
		out << " ";
	}
	if (minutes)
	{
		out << minutes << (abbrv ? " min" : " minute");
		if (minutes > 1)
		{
			out << "s";
		}
		out << " ";
	}
	out << totalSeconds - wholeSeconds + seconds << (abbrv ? " secs" : " seconds");
}
static string time_stamp(const string& format = "%Y.%m.%d-%H.%M.%S%F%Q")
{
	time_facet* timef = new time_facet(format.c_str());
	stringstream ss;
	ss.imbue(locale(ss.getloc(), timef));
	ss << microsec_clock::local_time();
	return ss.str();
}
static void mark()
{
	static int num = 0;
	cout << "MARK " << num << endl;
	++num;
}
template<class T> static T squared(const T& t)
{
	return t*t;
}
template<class T> static int sign(const T& t)
{
	if (t < 0)
	{
		return -1;
	}
	else if (t > 0)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}
template <class T> static T bound (const T& v, const T& minVal, const T& maxVal)
{
	return min(max(minVal, v), maxVal);
}
//CAST OPERATIONS
template<class T> static string str(const T& t)
{
	stringstream ss;
	ss << t;
	return ss.str();
//	return lexical_cast<string>(t);
}
template<class T> static double dbl(const T& t)
{
	return lexical_cast<double>(t);
}
template<class T> static double flt(const T& t)
{
	return lexical_cast<float>(t);
}
template<class T> static double integer(const T& t)
{
	return lexical_cast<int>(t);
}
//GENERIC RANGE OPERATIONS
template <class R> static size_t count_adjacent(const R& r)
{
	size_t count = 0;
	for (typename range_iterator<R>::type it = boost::begin(r); it != boost::end(r); it = adjacent_find(it, boost::end(r)))
	{
		++count;	
	}
	return count;
}
template<class R1, class R2> static size_t range_min_size (const R1& a, const R2& b)
{
	return min(boost::size(a), boost::size(b));
}
template<class R1, class R2, class R3> static size_t range_min_size (const R1& a, const R2& b, const R3& c)
{
	return min(min(boost::size(a), boost::size(b)), boost::size(c));
}
template<class R1, class R2, class R3, class R4> static size_t range_min_size (const R1& a, const R2& b, const R3& c, const R4& d)
{
	return min(min(min(boost::size(a), boost::size(b)), boost::size(c)), boost::size(d));
}
template<class R1, class R2, class R3, class R4, class R5> static size_t range_min_size (const R1& a, const R2& b, const R3& c, const R4& d, const R5& e)
{
	return min(min(min(min(boost::size(a), boost::size(b)), boost::size(c)), boost::size(d)), boost::size(e));
}
template <class R1, class R2> static pair<zip_iterator<tuple<typename range_iterator<R1>::type, typename range_iterator<R2>::type> >,
											zip_iterator<tuple<typename range_iterator<R1>::type, typename range_iterator<R2>::type> > > 
zip(R1& r1, R2& r2)
{
	size_t size = range_min_size(r1, r2);
	return make_pair(make_zip_iterator(make_tuple(boost::begin(r1), boost::begin(r2))), 
					 make_zip_iterator(make_tuple(boost::end(r1) - (boost::size(r1) - size), boost::end(r2) - (boost::size(r2) - size))));
}
template <class R1, class R2, class R3> static pair<zip_iterator<tuple<typename range_iterator<R1>::type, typename range_iterator<R2>::type, typename range_iterator<R3>::type> >,
													zip_iterator<tuple<typename range_iterator<R1>::type, typename range_iterator<R2>::type, typename range_iterator<R3>::type> > > 
zip(R1& r1, R2& r2, R3& r3)
{
	size_t size = range_min_size(r1, r2, r3);
	return make_pair(make_zip_iterator(make_tuple(boost::begin(r1), boost::begin(r2), boost::begin(r3))), 
					 make_zip_iterator(make_tuple(boost::end(r1) - (boost::size(r1) - size), boost::end(r2) - (boost::size(r2) - size), boost::end(r3) - (boost::size(r3) - size))));
}
template <class R1, class R2, class R3, class R4> static pair<zip_iterator<tuple<typename range_iterator<R1>::type, typename range_iterator<R2>::type, typename range_iterator<R3>::type, typename range_iterator<R4>::type> >,
zip_iterator<tuple<typename range_iterator<R1>::type, typename range_iterator<R2>::type, typename range_iterator<R3>::type, typename range_iterator<R4>::type> > > 
zip(R1& r1, R2& r2, R3& r3, R4& r4)
{
	size_t size = range_min_size(r1, r2, r3, r4);
	return make_pair(make_zip_iterator(make_tuple(boost::begin(r1), boost::begin(r2), boost::begin(r3), boost::begin(r4))), 
					 make_zip_iterator(make_tuple(boost::end(r1) - (boost::size(r1) - size), boost::end(r2) - (boost::size(r2) - size), boost::end(r3) - (boost::size(r3) - size), boost::end(r4) - (boost::size(r4) - size))));
}
template <class R1, class R2, class R3, class R4, class R5> static pair<zip_iterator<tuple<typename range_iterator<R1>::type, typename range_iterator<R2>::type, typename range_iterator<R3>::type, typename range_iterator<R4>::type, typename range_iterator<R5>::type> >,
zip_iterator<tuple<typename range_iterator<R1>::type, typename range_iterator<R2>::type, typename range_iterator<R3>::type, typename range_iterator<R4>::type, typename range_iterator<R5>::type> > > 
zip(R1& r1, R2& r2, R3& r3, R4& r4, R5& r5)
{
	size_t size = range_min_size(r1, r2, r3, r4, r5);
	return make_pair(make_zip_iterator(make_tuple(boost::begin(r1), boost::begin(r2), boost::begin(r3), boost::begin(r4), boost::begin(r5))), 
					 make_zip_iterator(make_tuple(boost::end(r1) - (boost::size(r1) - size), boost::end(r2) - (boost::size(r2) - size), boost::end(r3) - (boost::size(r3) - size), boost::end(r4) - (boost::size(r4) - size), boost::end(r5) - (boost::size(r5) - size))));
}
template <class R> static pair<zip_iterator<tuple<typename range_iterator<R>::type, counting_iterator<typename range_difference<R>::type> > >,
								zip_iterator<tuple<typename range_iterator<R>::type, counting_iterator<typename range_difference<R>::type> > > >
enumerate(R& r)
{
	return make_pair(make_zip_iterator(make_tuple(boost::begin(r), counting_iterator<typename range_difference<R>::type>(0))), 
					make_zip_iterator(make_tuple(boost::end(r), counting_iterator<typename range_difference<R>::type>(boost::size(r)))));
}
template <class T> static pair<counting_iterator<T>, counting_iterator<T> > range(const T& t)
{
	return make_pair(counting_iterator<T>(0), counting_iterator<T>(t));
}
template <class T> static pair<counting_iterator<T>, counting_iterator<T> > range(const T& t1, const T& t2)
{
	return make_pair(counting_iterator<T>(t1), counting_iterator<T>(t2));
}
template <class R> static pair<counting_iterator<typename range_size<R>::type>, counting_iterator<typename range_size<R>::type> > indices(const R& r)
{
    return ::range(boost::size(r));
}
template <class R1, class R2, class F> static typename range_iterator<R2>::type transform(const R1& r1, R2& r2, F f)
{
	return transform(boost::begin(r1), boost::end(r1), boost::begin(r2), f);
}
template <class R> static typename range_value<R>::type& nth_last(R& r, size_t n = 1)
{
	check(n > 0 && n <= boost::size(r), "nth_last called with n = " + str(n) + " for range of size " + (str(boost::size(r))));
	return *(boost::end(r) - n); 
}
template <class R> size_t last_index(R& r)
{
	return (boost::size(r) - 1); 
}
template <class R, class UnaryFunction> static UnaryFunction for_each(R& r, UnaryFunction f)
{
	return for_each(boost::begin(r), boost::end(r), f); 
}
template <class R, class T> static bool in(const R& r, const T& t)
{
	return find(boost::begin(r), boost::end(r), t) != boost::end(r);
}
template <class R, class T> static size_t index(const R& r, const T& t)
{
	return distance(boost::begin(r), find(boost::begin(r), boost::end(r), t));
}
template <class R> static void reverse(R& r)
{
	reverse(boost::begin(r), boost::end(r));
}
template <class R> static void sort(R& r)
{
	sort(boost::begin(r), boost::end(r));
}
template <class R> static void reverse_sort(R& r)
{
	sort(boost::rbegin(r), boost::rend(r));
}
template <class R> pair<typename range_value<R>::type, typename range_value<R>::type> minmax(const R& r)
{
	pair<typename range_const_iterator<R>::type, typename range_const_iterator<R>::type> p = minmax_element(boost::begin(r), boost::end(r));
	return make_pair(*p.first, *p.second); 
}
template <class R> static void bound_range (R& r, const typename boost::range_value<R>::type& minVal, const typename boost::range_value<R>::type& maxVal)
{
	for (typename range_iterator<R>::type it = boost::begin(r); it != boost::end(r); ++it) 
	{
		*it = bound(*it, minVal, maxVal);
	}
}
template <class R1, class R2> typename boost::range_value<R1>::type euclidean_squared(const R1& r1, const R2& r2)
{
	typename range_const_iterator<R2>::type b2 = boost::begin(r2); 
	typename range_const_iterator<R1>::type e = boost::end(r1);
	typename boost::range_value<R1>::type d = 0;
	for (typename range_const_iterator<R1>::type b1 = boost::begin(r1); b1 != e; ++b1, ++b2)
	{
		typename boost::range_value<R1>::type diff = *b1-*b2;
		d += diff * diff;
	}
	return d;
}
template<class R> static void range_negate
		(R& r)
{
	transform(boost::begin(r), boost::end(r), boost::begin(r), negate<typename boost::range_value<R>::type>());
}
template<class R> static void fill (R& r, const typename boost::range_value<R>::type& v)
{
	fill(boost::begin(r), boost::end(r), v);
}
template<class R> static size_t count(const R& r, const typename boost::range_value<R>::type& v)
{
	return count(boost::begin(r), boost::end(r), v);
}
template<class R1, class R2> static void copy(const R1& source, R2& dest)
{
	assert(boost::size(dest) >= boost::size(source));
	copy(boost::begin(source), boost::end(source), boost::begin(dest));
}
template<class R1, class R2> static void reverse_copy(const R1& source, R2& dest)
{
	reverse_copy(boost::begin(source), boost::end(source), boost::begin(dest));
}
template <class R> static vector<typename boost::range_value<R>::type>& flip(const R& r)
{
	static vector<typename boost::range_value<R>::type> v;
	v.resize(boost::size(r));
	reverse_copy(r, v);
	return v;
}
template<class R1, class R2> static bool equal(const R1& source, R2& dest)
{
	return ((boost::size(source) == boost::size(dest)) && equal(boost::begin(source), boost::end(source), boost::begin(dest)));
}
template<class R> static void shuffle (R& r)
{
	random_shuffle(boost::begin(r), boost::end(r));
}
template <class R> static typename range_value<R>::type max(const R& r)
{
	return *max_element(boost::begin(r), boost::end(r));
}
template <class C, class Tr, class R> static void print_range(basic_ostream<C, Tr>& out, const R& r, const basic_string<C, Tr>& delim = " ")
{
	typename range_const_iterator<R>::type b = boost::begin(r); 
	typename range_const_iterator<R>::type e = boost::end(r);
	if (b != e) 
	{ 
		out << *b;
		while (++b != e) 
		{
			out << delim << *b; 
		}
	}
}
template <class C, class Tr, class R> static basic_ostream<C, Tr>& operator <<(basic_ostream<C, Tr>& out, const R& r)
{
	print_range(out, r);
	return out;
}
template <class C, class Tr, class R> static basic_istream<C, Tr>& operator >>(basic_istream<C, Tr>& in, R& r)
{
	typename range_iterator<R>::type b = boost::begin(r); 
	typename range_iterator<R>::type e = boost::end(r);
	for (; b != e; ++b)
	{
		in >> *b; 
	}
	return in;
}
template<class R> void delete_range(R& r)
{
	for (typename range_iterator<R>::type it = boost::begin(r); it != boost::end(r); ++it)
	{
		delete *it;
	}
}
template <class R> static int max_index(const R& r)
{
	return distance(boost::begin(r), max_element(boost::begin(r), boost::end(r)));
}
//ARITHMETIC RANGE OPERATIONS
template<class R1, class R2> static typename range_value<R1>::type inner_product(const R1& a, const R2& b, typename range_value<R1>::type c = 0)
{
	return inner_product(boost::begin(a), boost::end(a), boost::begin(b), c);
}
template <class R> static typename range_value<R>::type magnitude(const R& r)
{
	return 0.5 * inner_product(r, r);
}
template <class R1, class R2> static typename range_value<R1>::type sum_of_squares(const R1& r1, const R2& r2)
{
	typename range_const_iterator<R1>::type it1 = boost::begin(r1); 
	typename range_const_iterator<R2>::type it2 = boost::begin(r2); 
	typename range_const_iterator<R1>::type e = boost::end(r1);
	typename range_value<R1>::type v = 0;
	for (; it1 != e; ++it1, ++it2)
	{
		typename range_value<R1>::type diff = *it1 - *it2;
		v += diff * diff;
	}
	return v / 2;
}
template <class R> static typename range_value<R>::type product(const R& r)
{
	return accumulate(boost::begin(r), boost::end(r), (typename range_value<R>::type)1, multiplies<typename range_value<R>::type>());
}
template <class R> static typename range_value<R>::type sum(const R& r)
{
	return accumulate(boost::begin(r), boost::end(r), (typename range_value<R>::type)0);
}
template <class R> static typename range_value<R>::type mean(const R& r)
{
	return sum(r) / (typename range_value<R>::type)boost::size(r);
}
//plus
template<class R1, class R2, class R3> static R1& range_plus(R1& a, const R2& b, const R3& c)
{
	transform(boost::begin(b), boost::end(b), boost::begin(c), boost::begin(a), plus<typename boost::range_value<R1>::type>());
	return a;
}
template<class R1, class R2> static void range_plus_equals(R1& a, const R2& b)
{
	range_plus(a, a, b);
}
//minus
template<class R1, class R2, class R3> static void range_minus(R1& a, const R2& b, const R3& c)
{
	transform(boost::begin(b), boost::end(b), boost::begin(c), boost::begin(a), minus<typename boost::range_value<R1>::type>());
}
template<class R1, class R2> static void range_minus_equals(R1& a, const R2& b)
{
	range_minus(a, a, b);
}
//multiply
template<class R1, class R2> static void range_multiply_val(R1& a, const R2& b, const typename boost::range_value<R2>::type& c)
{
	transform(boost::begin(b), boost::end(b), boost::begin(a), bind2nd(multiplies<typename boost::range_value<R2>::type>(), c));
}
template<class R> static void range_multiply_val(R& a, const typename boost::range_value<R>::type& b)
{
	range_multiply_val(a, a, b);
}
template<class R1, class R2, class R3> static void range_multiply(R1& a, const R2& b, const R3& c)
{
	transform(boost::begin(b), boost::begin(b) + range_min_size(a, b, c), boost::begin(c), boost::begin(a), multiplies<typename boost::range_value<R1>::type>());
}
template<class R1, class R2> static void range_multiply_equals(R1& a, const R2& b)
{
	range_multiply(a, a, b);
}
//divide
template<class R1, class R2> static void range_divide_val(R1& a, const R2& b, const typename boost::range_value<R1>::type& c)
{
	transform(boost::begin(b), boost::end(b), boost::begin(a), bind2nd(divides<typename boost::range_value<R1>::type>(), c));
}
template<class R> static void range_divide_val(R& a, const typename boost::range_value<R>::type& b)
{
	range_divide_val(a, a, b);
}
template<class R1, class R2, class R3> static void range_divide(R1& a, const R2& b, const R3& c)
{
	transform(boost::begin(b), boost::end(b), boost::begin(c), boost::begin(a), divides<typename boost::range_value<R1>::type>());
}
template<class R1, class R2> static void range_divide_equals(R1& a, const R2& b)
{
	range_divide(a, a, b);
}
//SET OPERATIONS
template<class R, class T> void operator +=(set<T>& s, const R& r)
{
	s.insert(boost::begin(r), boost::end(r));
}
//VECTOR OPERATIONS
template<class R, class T> void vector_assign(const R& r, vector<T>& v)
{
	v.resize(boost::size(r));
	copy(r, v);
}
//TUPLE OPERATIONS
template<class T1, class T2> static ostream& operator << (ostream& out, const tuple<T1, T2>& t)
{
	out << t.get<0>() << " " << t.get<1>();
	return out;
}
template<class T1, class T2, class T3> static ostream& operator << (ostream& out, const tuple<T1, T2, T3>& t)
{
	out << t.get<0>() << " " << t.get<1>() << " " << t.get<2>();
	return out;
}
template<class T1, class T2, class T3, class T4> static ostream& operator << (ostream& out, const tuple<T1, T2, T3, T4>& t)
{
	out << t.get<0>() << " " << t.get<1>() << " " << t.get<2>() << " " << t.get<3>();
	return out;
}
template<class T1, class T2, class T3, class T4, class T5> static ostream& operator << (ostream& out, const tuple<T1, T2, T3, T4, T5>& t)
{
	out << t.get<0>() << " " << t.get<1>() << " " << t.get<2>() << " " << t.get<3>() << " " << t.get<4>();
	return out;
}
//PAIR OPERATIONS
template<class T1, class T2> static void operator+= (pair<T1, T2>& a, const pair<T1, T2>& b)
{
	a.first += b.first;
	a.second += b.second;
}
template<class T1, class T2, class T3> static pair<T1, T2> operator+ (const pair<T1, T2>& a, const T3& b)
{
	return make_pair(a.first + b, a.second + b);
}
template<class T1, class T2> static ostream& operator << (ostream& out, const pair<T1, T2>& p)
{
	out << p.first << " " << p.second;
	return out;
}
template<class T1, class T2> static double pair_product(const pair<T1, T2>& p)
{
	return (double)(p.first * p.second);
}
template<class T1, class T2> static double pair_sum(const pair<T1, T2>& p)
{
	return (double)(p.first + p.second);
}
template<class T1, class T2> static double pair_mean(const pair<T1, T2>& p)
{
	return pair_sum(p)/2.0;
}
template <class T1, class T2> static size_t difference(const pair<T1,T2>& p)
{
	return p.second - p.first;
}
//MAP OPERATIONS
template<class T1, class T2> static bool in (const map<T1, T2>& a, const T1& b)
{
	return (a.find(b) != a.end());
}
template<class T1, class T2> static const T2& at (const map<T1, T2>& a, const T1& b)
{
	typename map<T1, T2>::const_iterator it = a.find(b);
	check(it != a.end(), str(b) + " not found in map:\n" + str(a));
	return it->second;
}
template<class T1, class T2> static ostream& operator << (ostream& out, const map<T1, T2>& m)
{
	for (typename map<T1, T2>::const_iterator it = m.begin(); it != m.end(); ++it)
	{
		out << *it << endl; 
	}
	return out;
}
template<class T1, class T2> static ostream& operator << (ostream& out, const map<T1, T2*>& m)
{
	for (typename map<T1, T2*>::const_iterator it = m.begin(); it != m.end(); ++it)
	{
		out << it->first << " " << *(it->second) << endl; 
	}
	return out;
}
template<class T1, class T2> static T2 sum_right (const map<T1, T2>& m)
{
	T2 ret = 0;
	for (typename map<T1, T2>::const_iterator it = m.begin(); it != m.end(); ++it)
	{
		ret += it->second;
	}
	return ret;
}
template<class T1, class T2, class T3, class T4> static void operator += (map<T1, T2>& a, const map<T3, T4>& b)
{
	for (typename map<T3, T4>::const_iterator it = b.begin(); it != b.end(); ++it)
	{
		a[it->first] += it->second;
	}
}
template<class T1, class T2, class T3, class T4> static void operator-= (map<T1, T2>& a, const map<T3, T4>& b)
{
	for (typename map<T3, T4>::const_iterator it = b.begin(); it != b.end(); ++it)
	{
		a[it->first] -= it->second;
	}
}
template<class T1, class T2, class T3, class T4> static void operator/= (map<T1, T2>& a, const map<T3, T4>& b)
{
	for (typename map<T3, T4>::const_iterator it = b.begin(); it != b.end(); ++it)
	{
		a[it->first] /= it->second;
	}
}
template<class T1, class T2, class T3, class T4> static void operator*= (map<T1, T2>& a, const map<T3, T4>& b)
{
	for (typename map<T3, T4>::const_iterator it = b.begin(); it != b.end(); ++it)
	{
		a[it->first] *= it->second;
	}
}
template<class T1, class T2, class T3> static void operator*= (map<T1, T2>& a, const T3& b)
{
	for (typename map<T1, T2>::iterator it = a.begin(); it != a.end(); ++it)
	{
		it->second *= b;
	}
}
template<class T1, class T2, class T3> static void operator/= (map<T1, T2>& a, const T3& b)
{
	for (typename map<T1, T2>::iterator it = a.begin(); it != a.end(); ++it)
	{
		it->second /= b;
	}
}
template<class R> void delete_map(R& r)
{
	for (typename range_iterator<R>::type it = boost::begin(r); it != boost::end(r); ++it)
	{
		delete it->second;
	}
}
//MULTIMAP OPERATIONS
template<class T1, class T2> static bool in (const multimap<T1, T2>& a, const T1& b)
{
	return (a.find(b) != a.end());
}
//BIMAP OPERATIONS
template<class T1, class T2, class T3, class T4> static ostream& operator << (ostream& out, const boost::bimaps::relation::structured_pair<T1, T2, T3, T4>& p)
{
	out << p.first << " " << p.second;
	return out;
}
template<class T1, class T2> void print_bimap (const bimap<T1, T2>& m, ostream& out)
{
	for (typename bimap<T1, T2>::left_const_iterator it = m.left.begin(); it != m.left.end(); ++it)
	{
		out << *it << endl;
	}
}
template<class T1, class T2> static ostream& operator << (ostream& out, const bimap<T1, T2>& m)
{
	print_bimap(m, out);
	return out;
}
template<class T1, class T2> static bool in_left(const bimap<T1, T2>& a, const T1& b)
{
	return (a.left.find(b) != a.left.end());
}
template<class T1, class T2> static bool in_right(const bimap<T1, T2>& a, const T2& b)
{
	return (a.right.find(b) != a.right.end());
}
//IO OPERATIONS
template<class T> static void print(const T& t, ostream& out = cout)
{
	out << t << endl;
}
template<class T1, class T2> static void print(const T1& t1, const T2& t2, ostream& out = cout)
{
	out << t1 << " " << t2 << endl;
}
template<class T1, class T2, class T3> static void print(const T1& t1, const T2& t2, const T3& t3, ostream& out = cout)
{
	out << t1 << " " << t2 << " " << t3 << endl;
}
template<class T1, class T2, class T3, class T4> static void print(const T1& t1, const T2& t2, const T3& t3, const T4& t4, ostream& out = cout)
{
	out << t1 << " " << t2 << " " << t3  << " " << t4 << endl;
}
template<class T1, class T2, class T3, class T4, class T5> static void print(const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, ostream& out = cout)
{
	out << t1 << " " << t2 << " " << t3  << " " << t4 << " " << t5 << endl;
}
static void prt_line(ostream& out = cout)
{
	out << "------------------------------" << endl;
}
template<class T> T read(const string& data)
{
	T val;
	stringstream ss;
	ss << boolalpha << data;
	check(ss >> val, "cannot read string '" + data + "' into variable with type '" + typeid(T).name() + "'");
	return val;
}
//STRING OPERATIONS
static string ordinal(size_t n)
{
	string s = str(n);
	if (n < 100)
	{
		char c = nth_last(s);
		if(c == '1')
		{
			return s + "st";
		}
		else if(c == '2')
		{
			return s + "nd";
		}
		else if(c == '3')
		{
			return s + "rd";
		}
	}
	return s + "th";
}
static void trim(string& str)
{
    size_t startpos = str.find_first_not_of(" \t\n");
    size_t endpos = str.find_last_not_of(" \t\n");
	if(string::npos == startpos || string::npos == endpos)  
    {  
        str = "";  
    }  
    else 
	{
        str = str.substr(startpos, endpos-startpos + 1);
	}
}
static bool in(const string& str, const string& search)
{
	return (str.find(search) != string::npos);
}
static bool in(const string& str, const char* search)
{
	return in(str, string(search));
}
template<class T> vector<T>& split(const string& original, const char delim = ' ')
{
	static vector<T> vect;
	vect.clear();
	stringstream ss;
	ss << original;
	string s;
	while (getline(ss, s, delim))
	{
		vect += read<T>(s);
	}
	return vect;
}
template<class T, class R >string join(const R& r, const string joinStr = "")
{
	typename range_iterator<R>::type b = boost::begin(r);
	string s = str(*b);
	++b;
	for (; b != end(r); ++b)
	{
		s += joinStr + str(*b);
	}
	return s;
}

//HELPER STRUCTS
template<class T> struct View: public sub_range<pair <T*, T*> >
{	
	View(pair<T*, T*>& p):
		sub_range<pair <T*, T*> >(p)
	{}
	View(T* first = 0, T* second = 0):
		sub_range<pair <T*, T*> >(make_pair(first, second))
	{}
	T& at(size_t i)
	{
		check(i < this->size(), "at(" + str(i) + ") called for view of size " + str(this->size()));
		return (*this)[i];
	}
	const T& at(size_t i) const
	{
		check(i < this->size(), "at(" + str(i) + ") called for view of size " + str(this->size()));
		return (*this)[i];	
	}
};

typedef const tuple<View<double>&, vector<double>&>& TVWDVD;
typedef const tuple<View<double>&, View<double>&>& TVWDVWD;

#endif
