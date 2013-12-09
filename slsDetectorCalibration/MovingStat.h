#ifndef MOVINGSTAT_H
#define MOVINGSTAT_H 

#include <math.h>


class MovingStat
    {
    public:
      MovingStat() : n(1000), m_n(0) {}

        void Clear()
        {
            m_n = 0;
        }
	
	void SetN(int i) {n=i;};
	int GetN() {return n;};
	inline void Calc(double x) {
	  if (m_n<n) Add(x);
	  else Push(x);
	}

	inline void Add(double x) {
	  m_n++;

            // See Knuth TAOCP vol 2, 3rd edition, page 232
	  if (m_n == 1)
            {
                m_oldM = m_newM = x;
                m_oldM2 = x*x;
            }
            else
            {
                m_newM = m_oldM + x;
		m_newM2 = m_oldM2 + x*x;
		//m_newM2 = m_oldM2 + (x*x - m_oldM*m_oldM)/m_n;

                // set up for next iteration
                m_oldM = m_newM; 
                m_oldM2 = m_newM2;
            }

	}


        inline void Push(double x)
        {
	  if (m_n == 1)
            {
	      m_oldM = m_newM = x;
	      m_oldM2 = x*x;
            }
            else
            {
                m_newM = m_oldM + (x - m_oldM/m_n);
		m_newM2 = m_oldM2 + (x*x - m_oldM2/m_n);
		//m_newM2 = m_oldM2 + (x*x/m_n - m_oldM*m_oldM/(m_n*m_n));
                // set up for next iteration
                m_oldM = m_newM; 
                m_oldM2 = m_newM2;
            }

        }

        int NumDataValues() const
        {
            return m_n;
        }

        inline double Mean() const
        {
            return (m_n > 0) ? m_newM/m_n : 0.0;
        }
	double M2() const
        {
            return ( (m_n > 1) ? m_newM2/m_n : 0.0 );
        }

        inline double Variance() const
        {
	  return ( (m_n > 1) ? (M2()-Mean()*Mean()) : 0.0 );
        }

        inline double StandardDeviation() const
        {
	  return ( (Variance() > 0) ? sqrt( Variance() ) : -1 );
        }

    private:
	int n;
        int m_n;
        double m_oldM, m_newM, m_oldM2, m_newM2;
    };
#endif
