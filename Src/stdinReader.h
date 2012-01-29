#ifndef _STDINREADER_H_
#define _STDINREADER_H_

#include <QThread>
#include <iostream>

class StdinReader : public QThread {
	Q_OBJECT
public:
	void run() {
		while (std::cin) {
			std::string str;
			std::cin >> str;
			QString qstr = str.c_str();
			emit onNewItem(qstr);
		}
	}
signals:
	void onNewItem(const QString& value);
};

#endif /* _STDINREADER_H_ */
