  /********************************************//**
 * @file runningStat.h
 * @short handles pedestal data and doesnt move
 ***********************************************/
#ifndef RUNNINGSTAT_H
#define RUNNINGSTAT_H

#include <math.h>

//#include "sls_detector_defs.h"
#include <stdint.h>
typedef  double double32_t;
typedef  float float32_t;
typedef  int int32_t;

/**
   @short class handling pedestal data that is static
 */

class runningStat{

public:
	/**
	 * Constructor
	 */
	runningStat() : m_n(0),m_oldM(0),m_newM(0),m_oldS(0),m_newS(0) {}

	/**
	 * Clear number of data values
	 */
	void Clear(){
		m_n = 0;
	}

	/**
	 * Push Pedestal
	 */
	void Push(double x){
		m_n++;

		// See Knuth TAOCP vol 2, 3rd edition, page 232
		if (m_n == 1){
			m_oldM = m_newM = x;
			m_oldS = 0.0;
		}else{
			m_newM = m_oldM + (x - m_oldM)/m_n;
			m_newS = m_oldS + (x - m_oldM)*(x - m_newM);

			// set up for next iteration
			m_oldM = m_newM;
			m_oldS = m_newS;
		}
	}

	/**
	 * Get number of data values
	 */
	int NumDataValues() const{
		return m_n;
	}

	/**
	 * Get mean
	 */
	double Mean() const{
		return (m_n > 0) ? m_newM : 0.0;
	}

	/**
	 * Get variance
	 */
	double Variance() const{
		return ( (m_n > 1) ? m_newS/(m_n - 1) : 0.0 );
	}

	/**
	 * Get standard deviation
	 */
	double StandardDeviation() const{
		return sqrt( Variance() );
	}

private:
	/** number of data values */
	int m_n;

	/** old and new mean */
	double m_oldM, m_newM, m_oldS, m_newS;

};

#endif
