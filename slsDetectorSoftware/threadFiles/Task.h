#pragma once

#include <pthread.h>
#include <unistd.h>
#include <deque>
#include <iostream>
#include <vector>
#include <errno.h>
#include <string.h>


#include "Global.h"
#include "sls_detector_defs.h"

#include <iostream>
#include <stdio.h>
using namespace std;

class slsDetector;

template<typename _Ret>
class func00_t{
public:
	func00_t(_Ret (slsDetector::*fn)(),slsDetector* ptr):
		m_fn(fn),m_ptr(ptr){}
	~func00_t() {}
	void operator()() const {((m_ptr->*m_fn)());}
private:
	_Ret (slsDetector::*m_fn)();
	slsDetector* m_ptr;
};

template<typename _Ret>
class func0_t{
public:
	func0_t(_Ret (slsDetector::*fn)(),slsDetector* ptr, _Ret* sto):
		m_fn(fn),m_ptr(ptr),m_store(sto){}
	~func0_t() {}
	void operator()() const {*m_store = ((m_ptr->*m_fn)());}
private:
	_Ret (slsDetector::*m_fn)();
	slsDetector* m_ptr;
	_Ret* m_store;
};

template<typename _Ret,typename _Arg1>
class func1_t{
public:
	func1_t(_Ret (slsDetector::*fn)(_Arg1),slsDetector* ptr,_Arg1 arg1, _Ret* sto):
		m_fn(fn),m_ptr(ptr),m_arg1(arg1),m_store(sto){}
	~func1_t() {}
	void operator()() const {*m_store = ((m_ptr->*m_fn)(m_arg1));}
private:
	_Ret (slsDetector::*m_fn)(_Arg1);
	slsDetector* m_ptr;
	_Arg1 m_arg1;
	_Ret* m_store;
};

template<typename _Ret,typename _Arg1, typename _Arg2>
class func2_t{
public:
	func2_t(_Ret (slsDetector::*fn)(_Arg1,_Arg2),slsDetector* ptr,_Arg1 arg1,_Arg2 arg2,_Ret* sto):
		m_fn(fn),m_ptr(ptr),m_arg1(arg1),m_arg2(arg2),m_store(sto){}
	~func2_t() {}
	void operator()() const {*m_store = ((m_ptr->*m_fn)(m_arg1,m_arg2));}
private:
	_Ret (slsDetector::*m_fn)(_Arg1,_Arg2);
	slsDetector* m_ptr;
	_Arg1 m_arg1;
	_Arg2 m_arg2;
	_Ret* m_store;
};

template<typename _Ret,typename _Arg1, typename _Arg2, typename _Arg3>
class func3_t{
public:
	func3_t(_Ret (slsDetector::*fn)(_Arg1,_Arg2,_Arg3),slsDetector* ptr,_Arg1 arg1,_Arg2 arg2,_Arg3 arg3,_Ret* sto):
		m_fn(fn),m_ptr(ptr),m_arg1(arg1),m_arg2(arg2),m_arg3(arg3),m_store(sto){}
	~func3_t() {}
	void operator()() const {*m_store = ((m_ptr->*m_fn)(m_arg1,m_arg2,m_arg3));}
private:
	_Ret (slsDetector::*m_fn)(_Arg1,_Arg2,_Arg3);
	slsDetector* m_ptr;
	_Arg1 m_arg1;
	_Arg2 m_arg2;
	_Arg3 m_arg3;
	_Ret* m_store;
};

template<typename _Ret,typename _Arg1, typename _Arg2, typename _Arg3, typename _Arg4>
class func4_t{
public:
	func4_t(_Ret (slsDetector::*fn)(_Arg1,_Arg2,_Arg3,_Arg4),slsDetector* ptr,_Arg1 arg1,_Arg2 arg2,_Arg3 arg3,_Arg4 arg4,_Ret* sto):
		m_fn(fn),m_ptr(ptr),m_arg1(arg1),m_arg2(arg2),m_arg3(arg3),m_arg4(arg4),m_store(sto){}
	~func4_t() {}
	void operator()() const {*m_store = ((m_ptr->*m_fn)(m_arg1,m_arg2,m_arg3,m_arg4));}
private:
	_Ret (slsDetector::*m_fn)(_Arg1,_Arg2,_Arg3,_Arg4);
	slsDetector* m_ptr;
	_Arg1 m_arg1;
	_Arg2 m_arg2;
	_Arg3 m_arg3;
	_Arg4 m_arg4;
	_Ret* m_store;
};


class SuperTask:  public virtual slsDetectorDefs {
public:
	SuperTask():
		m1(0),m2(0),m3(0),m4(0),m5(0),m6(0),m7(0),m8(0),m9(0),m10(0),m11(0),m12(0),m13(0),m14(0),m15(0),m16(0),m17(0),m18(0){};
	virtual ~SuperTask(){};
protected:
	/** Function signature defined
	 * First argument is Return type, the remaining are arguments
	 */
	func00_t <void				>* 								m1;
	func0_t <int				>* 								m2;
	func0_t <double				>* 								m3;
	func0_t <runStatus			>* 								m4;
	func1_t <int,				int>* 							m5;
	func1_t <int,				double>*						m6;
	func1_t <string,			string>* 						m7;
	func1_t <detectorSettings,	int>* 							m8;
	func2_t <int,				int,int>* 						m9;
	func2_t <int,				string,int>* 					m10;
	func2_t <dacs_t,			dacIndex,int>* 					m11;
	func2_t <detectorSettings,	detectorSettings,int>* 			m12;
	func2_t <int64_t,			timerIndex,int64_t>* 			m13;
	func2_t <string,			networkParameter,string>* 		m14;
	func3_t <int,				int,int,int>* 					m15;
	func4_t <int,				trimMode,int,int,int>* 			m16;
	func4_t <int,				int,int,detectorSettings,int>*	m17;
	func4_t <dacs_t,			dacs_t,dacIndex,int,int>* 		m18;
};

class Task:  public virtual SuperTask {
public:
	/** Task Constructors using appropriate function signature
	 * First argument is Return type, the remaining are arguments
	 */
	Task(func00_t <void				>* t):									SuperTask(),fnum(1){m1 = t;};
	Task(func0_t <int				>* t):									SuperTask(),fnum(2){m2 = t;};
	Task(func0_t <double			>* t):									SuperTask(),fnum(3){m3 = t;};
	Task(func0_t <runStatus			>* t):									SuperTask(),fnum(4){m4 = t;};
	Task(func1_t <int,				int>* t):								SuperTask(),fnum(5){m5 = t;};
	Task(func1_t <int,				double>* t):							SuperTask(),fnum(6){m6 = t;};
	Task(func1_t <string,			string>* t):							SuperTask(),fnum(7){m7 = t;};
	Task(func1_t <detectorSettings,	int>* t):								SuperTask(),fnum(8){m8 = t;};
	Task(func2_t <int,				int,int>* t):	 						SuperTask(),fnum(9){m9 = t;};
	Task(func2_t <int,				string,int>* t):						SuperTask(),fnum(10){m10 = t;};
	Task(func2_t <dacs_t,			dacIndex,int>* t):                  	SuperTask(),fnum(11){m11 = t;};
	Task(func2_t <detectorSettings,	detectorSettings,int>* t):				SuperTask(),fnum(12){m12 = t;};
	Task(func2_t <int64_t,			timerIndex,int64_t>* t):				SuperTask(),fnum(13){m13 = t;};
	Task(func2_t <string,			networkParameter,string>* t):			SuperTask(),fnum(14){m14 = t;};
	Task(func3_t <int,				int,int,int>* t): 						SuperTask(),fnum(15){m15 = t;};
	Task(func4_t <int,				trimMode,int,int,int>* t):				SuperTask(),fnum(16){m16 = t;};
	Task(func4_t <int,				int,int,detectorSettings,int>* t): 		SuperTask(),fnum(17){m17 = t;};
	Task(func4_t <dacs_t,			dacs_t,dacIndex,int,int>* t):       	SuperTask(),fnum(18){m18 = t;};

	virtual ~Task(){
        switch(fnum) {
        case 1: delete m1;  break;
        case 2: delete m2;  break;
        case 3: delete m3;  break;
        case 4: delete m4;  break;
        case 5: delete m5;  break;
        case 6: delete m6;  break;
        case 7: delete m7;  break;
        case 8: delete m8;  break;
        case 9: delete m9;  break;
        case 10: delete m10;  break;
        case 11: delete m11;  break;
        case 12: delete m12;  break;
        case 13: delete m13;  break;
        case 14: delete m14;  break;
        case 15: delete m15;  break;
        case 16: delete m16;  break;
        case 17: delete m17;  break;
        case 18: delete m18;  break;
        default:
            cprintf(RED, "Error: Task not defined. Abort!\n");
            break;
        }

	};

	void operator()(){
		switch(fnum) {
		case 1:		(*m1)();	break;
		case 2:		(*m2)();	break;
		case 3:		(*m3)();	break;
		case 4:		(*m4)();	break;
		case 5:		(*m5)();	break;
		case 6:		(*m6)();	break;
		case 7:		(*m7)();	break;
		case 8:		(*m8)();	break;
		case 9:		(*m9)();	break;
		case 10:	(*m10)();	break;
		case 11:	(*m11)();	break;
		case 12:	(*m12)();	break;
		case 13:	(*m13)();	break;
		case 14:    (*m14)();	break;
		case 15:    (*m15)();	break;
		case 16:    (*m16)();	break;
		case 17:    (*m17)();	break;
		case 18:    (*m18)();	break;
		default:
			cprintf(RED, "Error: Task not defined. Abort!\n");
			break;
		}
	}

private:
	/** function number */
	int fnum;

};


