#ifndef _STDINREADER_H_
#define _STDINREADER_H_

#include <QThread>
#include <iostream>

class StdinReader : public QThread {
	Q_OBJECT
public:
	StdinReader() : _alive(true) {}
	
	void run() {
		while (std::cin && _alive) {
			std::string str;
			std::cin >> str;
			QString qstr = str.c_str();
			emit onNewItem(qstr);
		}
		if (_alive) {
			emit onDone();
		}
	}
	
signals:
	void onNewItem(const QString& value);
	void onDone();

public slots:
	void die() { _alive = false; }
	
private:
	volatile bool _alive;
};

#endif /* _STDINREADER_H_ */
