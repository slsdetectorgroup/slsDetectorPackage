  /********************************************//**
 * @file movingStat.h
 * @short handles pedestal data and moves accordingly
 ***********************************************/
#ifndef MOVINGSTAT_H
#define MOVINGSTAT_H

//#include "sls_detector_defs.h"
#include <stdint.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstring>
#include <string>
#include <sstream>
#include <queue>
#include <math.h>

using namespace std;


typedef  double double32_t;
typedef  float float32_t;
typedef  int int32_t;


/**
   @short class handling pedestal data and moves according to data
 */

class movingStat{

public:
	/**
	 * Constructor
	 */
	movingStat() :  n(0), m_n(0), m_oldM(0), m_newM(0), m_oldM2(0), m_newM2(0){}

	/**
	 * Clear number of data values
	 */
	 inline void Clear(){
		m_n = 0;
	}

	/**
	 * Set Pedestal
	 */
	 inline void SetN(int i) {n=i;};

	/**
	 * Get Pedestal
	 */
	 inline int GetN() {return n;};

	/**
	 * Calculate Pedestal
	 */
	 inline void Calc(double x) {
		if (m_n<n) Add(x);
		else Push(x);
	}

	/**
	 * Adding Pedestal
	 */
	 inline void Add(double x){
		m_n++;

		// See Knuth TAOCP vol 2, 3rd edition, page 232
		if (m_n == 1){
			m_oldM = m_newM = x;
			m_oldM2 = x*x;
		}else{
			m_newM = m_oldM + x;
			m_newM2 = m_oldM2 + x*x;
			//m_newM2 = m_oldM2 + (x*x - m_oldM*m_oldM)/m_n;
			// set up for next iteration
			m_oldM = m_newM;
			m_oldM2 = m_newM2;
		}
	}

	/**
	 * Push Pedestal
	 */
	 inline void Push(double x){
		if (m_n == 1){
			m_oldM = m_newM = x;
			m_oldM2 = x*x;
		}else{
			m_newM = m_oldM + (x - m_oldM/m_n);
			m_newM2 = m_oldM2 + (x*x - m_oldM2/m_n);
			//m_newM2 = m_oldM2 + (x*x/m_n - m_oldM*m_oldM/(m_n*m_n));
			// set up for next iteration
			m_oldM = m_newM;
			m_oldM2 = m_newM2;
		}
	}

	/**
	 * Get number of data values
	 */
	 inline int NumDataValues() const{
		return m_n;
	}

	/**
	 * Get mean
	 */
	 inline double Mean() const{
		return (m_n > 0) ? m_newM/m_n : 0.0;
	}

	/**
	 * Get mean 2
	 */
	 inline double M2() const{
		return ( (m_n > 1) ? m_newM2/m_n : 0.0 );
	}

	/**
	 * Get variance
	 */
	 inline double Variance() const{
		return ( (m_n > 1) ? (M2()-Mean()*Mean()) : 0.0 );
	}

	/**
	 * Get standard deviation
	 */
	 inline double StandardDeviation() const{
		return ( (Variance() > 0) ? sqrt( Variance() ) : -1 );
	}

private:
	/** pedestal */
	int n;
	/** number of data values */
	int m_n;

	/** old and new mean */
	double m_oldM, m_newM, m_oldM2, m_newM2;
};


#endif
