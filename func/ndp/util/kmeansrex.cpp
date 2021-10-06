
extern "C" {
	void RunKMeans(double *X_IN,  int N,  int D, int K, int Niter, \
	           int seed, const char* initname, double *Mu_OUT, double *Z_OUT);
	void SampleRowsPlusPlus(double *X_IN,  int N,  int D, int K, int seed, double *Mu_OUT);
}

/*
   A C-program for MT19937, with initialization improved 2002/1/26.
   Coded by Takuji Nishimura and Makoto Matsumoto.
   Before using, initialize the state by using init_genrand(seed)
   or init_by_array(init_key, key_length).
   Copyright (C) 1997 - 2002, Makoto Matsumoto and Takuji Nishimura,
   All rights reserved.
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:
     1. Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.
     2. Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.
     3. The names of its contributors may not be used to endorse or promote
        products derived from this software without specific prior written
        permission.
   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
   Any feedback is very welcome.
   http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/emt.html
   email: m-mat @ math.sci.hiroshima-u.ac.jp (remove space)
*/

#include <stdio.h>

/* Period parameters */
#define N_N 624
#define M_M 397
#define MATRIX_A 0x9908b0dfUL   /* constant vector a */
#define UPPER_MASK 0x80000000UL /* most significant w-r bits */
#define LOWER_MASK 0x7fffffffUL /* least significant r bits */

static unsigned long mt[N_N]; /* the array for the state vector  */
static int mti=N_N+1; /* mti==N_N+1 means mt[N_N] is not initialized */

/* initializes mt[N_N] with a seed */
void init_genrand(unsigned long s)
{
    mt[0]= s & 0xffffffffUL;
    for (mti=1; mti<N_N; mti++) {
        mt[mti] =
	    (1812433253UL * (mt[mti-1] ^ (mt[mti-1] >> 30)) + mti);
        /* See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. */
        /* In the previous versions, MSBs of the seed affect   */
        /* only MSBs of the array mt[].                        */
        /* 2002/01/09 modified by Makoto Matsumoto             */
        mt[mti] &= 0xffffffffUL;
        /* for >32 bit machines */
    }
}

/* initialize by an array with array-length */
/* init_key is the array for initializing keys */
/* key_length is its length */
/* slight change for C++, 2004/2/26 */
void init_by_array(unsigned long init_key[], int key_length)
{
    int i, j, k;
    init_genrand(19650218UL);
    i=1; j=0;
    k = (N_N>key_length ? N_N : key_length);
    for (; k; k--) {
        mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 30)) * 1664525UL))
          + init_key[j] + j; /* non linear */
        mt[i] &= 0xffffffffUL; /* for WORDSIZE > 32 machines */
        i++; j++;
        if (i>=N_N) { mt[0] = mt[N_N-1]; i=1; }
        if (j>=key_length) j=0;
    }
    for (k=N_N-1; k; k--) {
        mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 30)) * 1566083941UL))
          - i; /* non linear */
        mt[i] &= 0xffffffffUL; /* for WORDSIZE > 32 machines */
        i++;
        if (i>=N_N) { mt[0] = mt[N_N-1]; i=1; }
    }

    mt[0] = 0x80000000UL; /* MSB is 1; assuring non-zero initial array */
}

/* generates a random number on [0,0xffffffff]-interval */
unsigned long genrand_int32(void)
{
    unsigned long y;
    static unsigned long mag01[2]={0x0UL, MATRIX_A};
    /* mag01[x] = x * MATRIX_A  for x=0,1 */

    if (mti >= N_N) { /* generate N_N words at one time */
        int kk;

        if (mti == N_N+1)   /* if init_genrand() has not been called, */
            init_genrand(5489UL); /* a default initial seed is used */

        for (kk=0;kk<N_N-M_M;kk++) {
            y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
            mt[kk] = mt[kk+M_M] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        for (;kk<N_N-1;kk++) {
            y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
            mt[kk] = mt[kk+(M_M-N_N)] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        y = (mt[N_N-1]&UPPER_MASK)|(mt[0]&LOWER_MASK);
        mt[N_N-1] = mt[M_M-1] ^ (y >> 1) ^ mag01[y & 0x1UL];

        mti = 0;
    }

    y = mt[mti++];

    /* Tempering */
    y ^= (y >> 11);
    y ^= (y << 7) & 0x9d2c5680UL;
    y ^= (y << 15) & 0xefc60000UL;
    y ^= (y >> 18);

    return y;
}

/*  =====================================================================
Generates double in interval [0,1]
Based on this note found in the README:
Note: the last five functions call the first one.
if you need more speed for these five functions, you may
suppress the function call by copying genrand_int32() and
replacing the last return(), following to these five functions.
 *  =====================================================================
 */
double genrand_double(void)
{
    unsigned long y;
    static unsigned long mag01[2]={0x0UL, MATRIX_A};
    /* mag01[x] = x * MATRIX_A  for x=0,1 */

    if (mti >= N_N) { /* generate N_N words at one time */
        int kk;

        if (mti == N_N+1)   /* if init_genrand() has not been called, */
            init_genrand(5489UL); /* a default initial seed is used */

        for (kk=0;kk<N_N-M_M;kk++) {
            y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
            mt[kk] = mt[kk+M_M] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        for (;kk<N_N-1;kk++) {
            y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
            mt[kk] = mt[kk+(M_M-N_N)] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        y = (mt[N_N-1]&UPPER_MASK)|(mt[0]&LOWER_MASK);
        mt[N_N-1] = mt[M_M-1] ^ (y >> 1) ^ mag01[y & 0x1UL];

        mti = 0;
    }

    y = mt[mti++];

    /* Tempering */
    y ^= (y >> 11);
    y ^= (y << 7) & 0x9d2c5680UL;
    y ^= (y << 15) & 0xefc60000UL;
    y ^= (y >> 18);
    return y*(1.0/4294967295.0);
}


/* generates a random number on [0,0x7fffffff]-interval */
long genrand_int31(void)
{
    return (long)(genrand_int32()>>1);
}

/* generates a random number on [0,1]-real-interval */
double genrand_real1(void)
{
    return genrand_int32()*(1.0/4294967295.0);
    /* divided by 2^32-1 */
}

/* generates a random number on [0,1)-real-interval */
double genrand_real2(void)
{
    return genrand_int32()*(1.0/4294967296.0);
    /* divided by 2^32 */
}

/* generates a random number on (0,1)-real-interval */
double genrand_real3(void)
{
    return (((double)genrand_int32()) + 0.5)*(1.0/4294967296.0);
    /* divided by 2^32 */
}

/* generates a random number on [0,1) with 53-bit resolution*/
double genrand_res53(void)
{
    unsigned long a=genrand_int32()>>5, b=genrand_int32()>>6;
    return(a*67108864.0+b)*(1.0/9007199254740992.0);
}
/* These real versions are due to Isaku Wada, 2002/01/09 added */


/* KMeansRexCore.h
   Provides header file function declarations for Matlab MEX compilation.
*/

#include "Eigen/Dense"
#include <iostream>
using namespace Eigen;
using namespace std;

/*  DEFINE Custom Type Names to make code more readable
    ExtMat :  2-dim matrix/array externally defined (e.g. in Matlab or Python)
*/
typedef Map<ArrayXXd> ExtMat;
typedef ArrayXXd Mat;
typedef ArrayXd Vec;

void set_seed( int seed );
void select_without_replacement( int N, int K, Vec &chosenIDs);
int discrete_rand( Vec &p );

void init_Mu( ExtMat &X, ExtMat &Mu, const char* initname );
void run_lloyd( ExtMat &X, ExtMat &Mu, ExtMat &Z, int Niter);

/* KMeansRexCore.cpp
A fast, easy-to-read implementation of the K-Means clustering algorithm.
allowing customized initialization (random samples or plus plus)
and vectorized execution via the Eigen matrix template library.
Intended to be compiled as a shared library which can then be utilized
from high-level interactive environments, such as Matlab or Python.
Contains
--------
Utility Fcns
* discrete_rand : sampling discrete random variable
* select_without_replacement : sample without replacement
Cluster Location Mu Initialization:
* sampleRowsRandom : sample rows of X at random (w/out replacement)
* sampleRowsPlusPlus : sample rows of X via kmeans++ procedure of Arthur et al.
    see http://en.wikipedia.org/wiki/K-means%2B%2B
K-Means Algorithm (aka Lloyd's Algorithm)
* run_lloyd : executes lloyd for specfied number of iterations
External "C" function interfaces (for calling from Python)
* RunKMeans          : compute cluster centers and assignments via lloyd
* SampleRowsPlusPlus : get just a plusplus initialization
Dependencies:
  mersenneTwister2002.c : random number generator
Author: Mike Hughes (www.michaelchughes.com)
Date:   2 April 2013
*/

#include <iostream>
#include "Eigen/Dense"

using namespace Eigen;
using namespace std;

/*  DEFINE Custom Type Names to make code more readable
    ExtMat :  2-dim matrix/array externally defined (e.g. in Matlab or Python)
*/
typedef Map<ArrayXXd> ExtMat;
typedef ArrayXXd Mat;
typedef ArrayXd Vec;

// ====================================================== Utility Functions
void set_seed( int seed ) {
  init_genrand( seed );
}

/*
 * Return random integers from `low` (inclusive) to `high` (exclusive).
 */
int randint(int low, int high) {
    double r = ((high - low)) * genrand_double();
    int rint = (int) r; // [0,1) -> 0, [1,2) -> 1, etc
    return rint + low;
}

int discrete_rand( Vec &p ) {
    double total = p.sum();
    int K = (int) p.size();

    double r = total*genrand_double();
    double cursum = p(0);
    int newk = 0;
    while ( r >= cursum && newk < K-1) {
        newk++;
        cursum += p[newk];
    }
    if ( newk < 0 || newk >= K ) {
        cerr << "Badness. Chose illegal discrete value." << endl;
        return -1;
    }
    return newk;
}

void select_without_replacement( int N, int K, Vec &chosenIDs) {
    Vec p = Vec::Ones(N);
    for (int kk =0; kk<K; kk++) {
        int choice;
        int doKeep = false;
        while ( doKeep==false) {
            doKeep=true;
            choice = discrete_rand( p );

            for (int previd=0; previd<kk; previd++) {
                if (chosenIDs[previd] == choice ) {
                doKeep = false;
                break;
                }
            }
        }
        chosenIDs[kk] = choice;
    }
}

// ======================================================= Init Cluster Locs Mu

void sampleRowsRandom( ExtMat &X, ExtMat &Mu ) {
    int N = X.rows();
    int K = Mu.rows();
    Vec ChosenIDs = Vec::Zero(K);
    select_without_replacement(N, K, ChosenIDs);
    for (int kk=0; kk<K; kk++) {
        Mu.row( kk ) = X.row( ChosenIDs[kk] );
    }
}

void sampleRowsPlusPlus( ExtMat &X, ExtMat &Mu ) {
    int N = X.rows();
    int K = Mu.rows();
    if (K > N) {
        // User requested more clusters than we have available.
        // So, we'll fill only first N rows of Mu
        // and leave all remaining rows of Mu uninitialized.
        K = N;
    }
    int choice = randint(0, N);
    Mu.row(0) = X.row( choice );
    Vec minDist(N);
    Vec curDist(N);
    for (int kk=1; kk<K; kk++) {
        curDist = (X.rowwise() - Mu.row(kk-1)).square().rowwise().sum();
        if (kk==1) {
            minDist = curDist;
        } else {
            minDist = curDist.min( minDist );
        }
        choice = discrete_rand( minDist );
        Mu.row(kk) = X.row( choice );
    }
}

void init_Mu( ExtMat &X, ExtMat &Mu, const char* initname ) {
    if (string(initname) == "random") {
        sampleRowsRandom( X, Mu );
    } else if (string(initname) == "plusplus") {
        sampleRowsPlusPlus( X, Mu );
    }
}

// ======================================================= Update Assignments Z
void pairwise_distance( ExtMat &X, ExtMat &Mu, Mat &Dist ) {
    int N = X.rows();
    int D = X.cols();
    int K = Mu.rows();

    // For small dims D, for loop is noticeably faster than fully vectorized.
    // Odd but true.  So we do fastest thing
    if ( D <= 16 ) {
        for (int kk=0; kk<K; kk++) {
            Dist.col(kk) = (X.rowwise() - Mu.row(kk)).square().rowwise().sum();
        }
    } else {
        Dist = -2*(X.matrix() * Mu.transpose().matrix());
        Dist.rowwise() += Mu.square().rowwise().sum().transpose().row(0);
    }
}

double assignClosest( ExtMat &X, ExtMat &Mu, ExtMat &Z, Mat &Dist) {
    double totalDist = 0;
    int minRowID;

    pairwise_distance( X, Mu, Dist );

    for (int nn=0; nn<X.rows(); nn++) {
        totalDist += Dist.row(nn).minCoeff( &minRowID );
        Z(nn,0) = minRowID;
    }
    return totalDist;
}

// ======================================================= Update Locations Mu
void calc_Mu( ExtMat &X, ExtMat &Mu, ExtMat &Z) {
    //Mu = Mat::Zero(Mu.rows(), Mu.cols());
    Mu.fill(0);
    Vec NperCluster = Vec::Zero(Mu.rows());
    for (int nn=0; nn<X.rows(); nn++) {
        Mu.row((int) Z(nn,0)) += X.row(nn);
        NperCluster[(int) Z(nn,0)] += 1;
    }
    NperCluster += 1e-100; // avoid division-by-zero
    for (int k=0; k < Mu.rows(); k++) {
       Mu.row(k) /= NperCluster(k);
    }
}

// ======================================================= Overall Lloyd Alg.
void run_lloyd( ExtMat &X, ExtMat &Mu, ExtMat &Z, int Niter )  {
    double prevDist,totalDist = 0;
    Mat Dist = Mat::Zero( X.rows(), Mu.rows() );

    for (int iter=0; iter<Niter; iter++) {
        totalDist = assignClosest( X, Mu, Z, Dist );
        calc_Mu( X, Mu, Z );
        if (prevDist == totalDist) {
            break;
        }
        prevDist = totalDist;
    }
}

// ===========================================================================
// ===========================================================================
// ===========================  EXTERNALLY CALLABLE FUNCTIONS ================
// ===========================================================================
// ===========================================================================

void RunKMeans(double *X_IN,  int N,  int D, int K, int Niter, \
               int seed, const char* initname, double *Mu_OUT, double *Z_OUT) {
  set_seed(seed);

  ExtMat X (X_IN, N, D);
  ExtMat Mu (Mu_OUT, K, D);
  ExtMat Z (Z_OUT, N, 1);

  init_Mu(X, Mu, initname);
  run_lloyd(X, Mu, Z, Niter );
}


void SampleRowsPlusPlus(double *X_IN,  int N,  int D, int K, \
                        int seed, double *Mu_OUT) {
  set_seed(seed);

  ExtMat X (X_IN, N, D);
  ExtMat Mu (Mu_OUT, K, D);

  sampleRowsPlusPlus(X, Mu);
}
