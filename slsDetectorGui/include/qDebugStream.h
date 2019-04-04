#pragma once

#include <QApplication>
#include <QCustomEvent>
#include <QString>
#include <QWidget>

#include <iostream>
#include <streambuf>
#include <string>

#define STREAMEVENT 60001


class qStreamEvent : public QEvent {
public:
	qStreamEvent(QString s) : QEvent(static_cast<QEvent::Type>(STREAMEVENT)), str(s) {
#ifdef PRINT_LOG
		printf("%s\n", str.toAscii().constData());
#endif
	}
	/** \returns the progress index */
	QString getString() { return str; }

private:
	QString str;
};


class qDebugStream : public std::basic_streambuf<char> {

public:
	qDebugStream(std::ostream &stream, QWidget *w) : m_stream(stream), log_window(w) {
		pthread_mutex_init(&mutex, NULL);
		m_old_buf = stream.rdbuf();
		stream.rdbuf(this);
	}


	~qDebugStream() {
		// output anything that is left
		if (!m_string.empty()) {
			pthread_mutex_lock(&mutex);
			const char *c_string = m_string.c_str();
			QApplication::postEvent(log_window, new qStreamEvent(c_string));
			pthread_mutex_unlock(&mutex);
		}
		m_stream.rdbuf(m_old_buf);
	}


protected:
	virtual int_type overflow(int_type v) {
		if (v == '\n') {
			pthread_mutex_lock(&mutex);
			const char *c_string = m_string.c_str();
			QApplication::postEvent(log_window, new qStreamEvent(c_string));
			m_string.erase(m_string.begin(), m_string.end());
			pthread_mutex_unlock(&mutex);
		} else
			m_string += v;
		return v;
	}


	virtual std::streamsize xsputn(const char *p, std::streamsize n) {
		pthread_mutex_lock(&mutex);
		m_string.append(p, p + n);

		//changed from uint because of 64 bit
		size_t pos = 0;

		while (pos != std::string::npos) {
			pos = m_string.find('\n');
			if (pos != std::string::npos) {
				std::string tmp(m_string.begin(), m_string.begin() + pos);
				const char *c_tmp = tmp.c_str();
				QApplication::postEvent(log_window, new qStreamEvent(c_tmp));
				m_string.erase(m_string.begin(), m_string.begin() + pos + 1);
			}
		}
		pthread_mutex_unlock(&mutex);
		return n;
	}


private:
	pthread_mutex_t mutex;
	std::ostream &m_stream;
	std::streambuf *m_old_buf;
	std::string m_string;
	QWidget *log_window;
};


