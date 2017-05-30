#ifndef MOVINGSTAT_H
#define MOVINGSTAT_H 

#include <math.h>


class MovingStat
    {

      /** @short approximated moving average structure */
    public:

      
      /** constructor
	  \param nn number of samples  parameter to be used
      */
      MovingStat(int nn=1000) : n(nn), m_n(0) {}

	/**
	   clears the moving average number of samples parameter, mean and standard deviation
	*/
        void Clear()
        {
	  m_n = 0; 
	  m_newM=0; 
	  m_newM2=0;
        }

	/**
	   clears the moving average number of samples parameter, mean and standard deviation
	*/
        void Set(double val, double rms=0)
        {
	  m_n = n; 
	  m_newM=val*n; 
	  if (rms<=0)
	    m_newM2=val*val*n;
	  else
	    m_newM2=(n*rms*rms+m_newM*m_newM/n);
        }
	
	
	/** sets number of samples parameter
	    \param i number of samples  parameter to be set 
	*/
	
	void SetN(int i) {if (i>=1) n=i;};
	
	/** 
	    gets number of samples  parameter
	    \returns actual number of samples   parameter
	*/
	int GetN() {return n;};
		
	/** calculates the moving average i.e. adds if number of elements is lower than number of samples parameter, pushes otherwise
	   \param x value to calculate the moving average
	*/
	inline void Calc(double x) {
	  if (m_n<n) Add(x);
	  else Push(x);
	}
	/** adds the element to the accumulated average and standard deviation 
	   \param x value to add
	*/
	inline void Add(double x) {
	  m_n++;

	  if (m_n == 1)
            {
	      m_newM = x;
	      m_newM2 = x*x;
            }             else             {
                m_newM = m_newM + x;
		m_newM2 = m_newM2 + x*x;
	
            }

	}


        inline void Push(double x)
        {
	  /** adds the element to the accumulated average and squared mean, while subtracting the current value of the average and squared average
	      \param x value to push
	  */
	  if (m_n == 0)
            {
	      m_newM = x;
	      m_newM2 = x*x;
	      m_n++;
            }             else             {
                m_newM = m_newM + x - m_newM/m_n;
		m_newM2 = m_newM2 + x*x - m_newM2/m_n;
            }

        }

	/** returns the current number of elements of the moving average
	    \returns  returns the current number of elements of the moving average
	 */
        int NumDataValues() const
        {
            return m_n;
        }
	/** returns the mean, 0 if no elements are inside
	    \returns  returns the mean
	 */
        inline double Mean() const
        {
            return (m_n > 0) ? m_newM/m_n : 0.0;
        }

	/** returns the squared mean, 0 if no elements are inside
	    \returns  returns the squared average
	 */
	double M2() const
        {
            return ( (m_n > 1) ? m_newM2/m_n : 0.0 );
        }

	/** returns the variance, 0 if no elements are inside
	    \returns  returns the variance
	 */
        inline double Variance() const
        {
	  return ( (m_n > 1) ? (M2()-Mean()*Mean()) : 0.0 );
        }

	/** returns the standard deviation, 0 if no elements are inside
	    \returns  returns the standard deviation
	 */
        inline double StandardDeviation() const
        {
	  return ( (Variance() > 0) ? sqrt( Variance() ) : -1 );
        }

    private:
	int n; /**< number of samples parameter */
        int m_n; /**< current number of elements */
	double m_newM; /**< accumulated average */
	double m_newM2; /**< accumulated squared average */
    };
#endif
