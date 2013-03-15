/*
 * qDebugStream.h
 *
 *  Created on: Jun 28, 2012
 *      Author: Anna Bergamaschi
 */

#ifndef QDEBUGSTREAM_H_
#define QDEBUGSTREAM_H_


#include "qDefs.h"


#include <QApplication>
#include <QWidget>
#include <QString>
#include <QCustomEvent>

#include <iostream>
#include <streambuf>
#include <string>
using namespace std;

#define STREAMEVENT 60001


//-------------------------------------------------------------------------------------------------------------------------------------------------

class qStreamEvent:public QEvent{
public:
	qStreamEvent(QString s):QEvent(static_cast<QEvent::Type>(STREAMEVENT)),str(s){
#ifdef PRINT_LOG
		printf("%s\n",str.toAscii().constData());
#endif
	}
	/** \returns the progress index */
	QString getString() {return str;}
private:
	QString str;

};

//-------------------------------------------------------------------------------------------------------------------------------------------------

class qDebugStream : public basic_streambuf<char> {

public:
	qDebugStream(ostream &stream, QWidget* w) : m_stream(stream), log_window(w) {
		m_old_buf = stream.rdbuf();
		stream.rdbuf(this);
	}

//-------------------------------------------------------------------------------------------------------------------------------------------------

	~qDebugStream(){
		// output anything that is left
		if (!m_string.empty())
			QApplication::postEvent(log_window, new qStreamEvent(m_string.c_str()));
		m_stream.rdbuf(m_old_buf);
	}

//-------------------------------------------------------------------------------------------------------------------------------------------------

protected:
	virtual int_type overflow(int_type v){
		if (v == '\n'){
			QApplication::postEvent(log_window, new qStreamEvent(m_string.c_str()));
			m_string.erase(m_string.begin(), m_string.end());
		}
		else
			m_string += v;
		return v;
	}

//-------------------------------------------------------------------------------------------------------------------------------------------------

	virtual streamsize xsputn(const char *p, streamsize n)	{
		m_string.append(p, p + n);
		//changed from uint because of 64 bit
		int pos = 0;

		while (pos != string::npos){
			pos = m_string.find('\n');
			if (pos != string::npos){
				string tmp(m_string.begin(), m_string.begin() + pos);
				QApplication::postEvent(log_window, new qStreamEvent(tmp.c_str()));
				m_string.erase(m_string.begin(), m_string.begin() + pos + 1);
			}
		}
		return n;
	}

//-------------------------------------------------------------------------------------------------------------------------------------------------

private:
	ostream &m_stream;
	streambuf *m_old_buf;
	string m_string;
	QWidget* log_window;
};

//-------------------------------------------------------------------------------------------------------------------------------------------------

#endif /* QDEBUGSTREAM_H_ */
