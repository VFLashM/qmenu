#ifndef _STDINREADER_H_
#define _STDINREADER_H_

#include <QThread>
#include <iostream>
#include <assert.h>
#include "qmenu.h"

class StdinReader : public QThread {
	Q_OBJECT
public:
	StdinReader(int mode) : _alive(true), _mode(mode) {}
	
	void run() {
		while (std::cin && _alive) {
			std::string str;
			std::cin >> str;
            QString qstr = str.c_str();
            _got.append(qstr);

            if (_got.size() >= _mode) {
                switch (_mode) {
                case 1:
                    {
                        MenuItem item(_got[0]);
                        _got.clear();
                        emit onNewItem(item);
                    }
                    break;
                case 2:
                    {
                        MenuItem item(_got[0], _got[1]);
                        _got.clear();
                        emit onNewItem(item);
                    }
                    break;
                case 3:
                    {
                        MenuItem item(_got[0], _got[1], _got[2]);
                        _got.clear();
                        emit onNewItem(item);
                    }
                    break;
                default:
                    assert(false);
                }
            }
		}
		if (_alive) {
			emit onDone();
		}
	}
	
signals:
	void onNewItem(const MenuItem& value);
	void onDone();

public slots:
	void die() { _alive = false; }
	
private:
	volatile bool _alive;
    const int _mode;
    QList<QString> _got;
};

#endif /* _STDINREADER_H_ */
