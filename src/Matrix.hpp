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

#ifndef _INCLUDED_Matrix_h  
#define _INCLUDED_Matrix_h  

#define OP_TRACKING

#ifdef OP_TRACKING
static unsigned long long matrixOps = 0;
#endif

// M += a * b 
static void outer(const double *aBegin, const double *aEnd, double *M, const double *b, const double *bEnd)
{
#ifdef OP_TRACKING
	const double* mStart = M;
#endif
	for (;b != bEnd; ++b)
	{
		double input = *b;
		for (const double *a = aBegin; a != aEnd; ++a, ++M)
		{
			*M += *a * input;
		}
	}
#ifdef OP_TRACKING
	matrixOps += M - mStart;
#endif
}
//out += M in
static void dot(const double *inBegin, const double *inEnd, const double *M, double *out, double *outEnd)
{
#ifdef OP_TRACKING
	const double* mStart = M;
#endif
	for (;out != outEnd; ++out)
	{
		double sum = 0;
		for (const double *in = inBegin; in != inEnd; ++in, ++M)
		{ 
			sum += *M * (*in);
		}
		*out += sum; 
	}
#ifdef OP_TRACKING
	matrixOps += M - mStart;
#endif
}
//out += M^T in
static void dot_transpose(const double *in, const double *inEnd, const double *M, double *outBegin, double *outEnd)
{
#ifdef OP_TRACKING
	const double* mStart = M;
#endif
	for (;in != inEnd; ++in)
	{
		double input = *in;
		for (double *out = outBegin; out != outEnd; ++out, ++M)
		{
			*out += *M * input;
		}
	}
#ifdef OP_TRACKING
	matrixOps += M - mStart;
#endif
}
template<class R> static void outer(const R& a, double *M, const R&b)
{
	outer(boost::begin(a), boost::end(a), M, boost::begin(b), boost::end(b));
}
template<class R> static void dot(const R& a, const double *M, const R& b)
{
	dot(boost::begin(a), boost::end(a), M, boost::begin(b), boost::end(b));
}
template<class R> static void dot_transpose(const R& a, const double *M, const R& b)
{
	dot_transpose(boost::begin(a), boost::end(a), M, boost::begin(b), boost::end(b));
}

#endif
