#pragma once

#include <stdio.h>

#include <QMainWindow>
#include <QListWidget>
#include <QListWidgetItem>
#include <QLineEdit>
#include <QMap>
#include <QVBoxLayout>
#include <QKeyEvent>
#include <QApplication>
#include <QTextStream>

class Menu : public QMainWindow {
    Q_OBJECT
    typedef QPair<QString, QString> QStringPair;
public:
    Menu(int width, int height, const QList<QString>& values, const char* separator = NULL, const char* cacheFile = NULL, unsigned int maxListItems = -1)
        : QMainWindow(0, Qt::SplashScreen | Qt::WindowStaysOnTopHint)
		, _chosen(false)
		, _separator(separator)
		, _cacheFile(cacheFile)
		, _maxListItems(maxListItems)
    {
		setAttribute(Qt::WA_QuitOnClose, true);
        setCentralWidget(&_containerWidget);

        QVBoxLayout* layout = new QVBoxLayout();
        layout->setContentsMargins(0, 0, 0, 0);
        layout->addWidget(&_listWidget);
        layout->addWidget(&_lineEdit);
        _containerWidget.setLayout(layout);

        resize(width, height);
        _listWidget.setFocusPolicy(Qt::NoFocus);
        _lineEdit.setFocus(Qt::OtherFocusReason);

		_loadCache();
		_addItems(values, false);

        connect(&_lineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(_rebuildList()));
        connect(&_lineEdit, SIGNAL(editingFinished()), this, SLOT(_done()));
        connect(&_listWidget, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(_done()));
    }	

signals:
	void onDone();

public slots:
	void addItem(const QString& item) {
		QStringPair p = _value2pair(item);
		if (!_items.contains(p)) {
			_items.append(p);
			QList<QStringPair> singleItemList;
			singleItemList.append(p);
			_addItemsToWidget(singleItemList);
		}

		if (!_nonCacheItems.contains(item)) {
			_nonCacheItems.append(item);
		}
	}

	void flushCache() {
		if (!_cacheFile) {
			return;
		}

		QFile cache(_cacheFile);
		if (cache.open(QFile::WriteOnly)) {
			QTextStream outs(&cache);
			foreach(const QString& item, _nonCacheItems) {
				outs << item << "\n";
			}
		}
	}

private slots:
    void _done() {
		if (!_chosen) {
			if (_listWidget.count() > 0) {
				QListWidgetItem* item = _listWidget.currentItem();
				if (!item) {
					item = _listWidget.item(0);
				}
				printf("%s", qPrintable(_widgetToContent[item]));
			}
			_close();
		}
    }

    void _rebuildList() {
        if (_filter.isNull() || (_filter != _lineEdit.text())) {
            _filter = _lineEdit.text();
            _listWidget.clear();
            _widgetToContent.clear();

            _addItemsToWidget(_items);
            
            if (_listWidget.count() > 0) {
                _listWidget.setCurrentRow(0);
            }
        }
    }

private:
	void _close() {
		if (!_chosen) {
			_chosen = true;
			emit onDone();
			close();
		}
	}
	
	void _addItems(const QList<QString>& values, bool fromCache) {
		QList<QStringPair> items;
		foreach(const QString& val, values) {
			QStringPair p = _value2pair(val);
			if (!_items.contains(p)) {
				items.append(p);
			}
			if (!fromCache) {
				if (!_nonCacheItems.contains(val)) {
					_nonCacheItems.append(val);
				}
			}
        }
		_items.append(items);
		_addItemsToWidget(items);
	}
	
	void _loadCache() {
		if (_cacheFile) {
			QFile cache(_cacheFile);
			if (cache.open(QFile::ReadOnly)) {
				QTextStream ins(&cache);
				QList<QString> lines;
				while(true) {
					QString line = ins.readLine();
					if (line.isNull()) {
						break;
					} else {
						lines.append(line);
					}
				}
				_addItems(lines, true);
			}
		}
	}
	
	QStringPair _value2pair(const QString& val) {
		if (_separator && val.contains(_separator)) {
			int idx = val.indexOf(_separator);
			return QStringPair(val.left(idx), val.right(val.size() - idx - strlen(_separator)));
		} else {
			return QStringPair(val, val);
		}
	}
	
	void _addItemsToWidget(QList<QStringPair>& items) {
		if (_listWidget.count() >= _maxListItems) return;
		QList<const QStringPair*> matched;
		foreach (const QStringPair& pair, items) {
			_process(matched, pair, pair.second, &_match1);
			if (_listWidget.count() >= _maxListItems) return;
		}
		foreach (const QStringPair& pair, items) {
			_process(matched, pair, pair.second, &_match2);
			if (_listWidget.count() >= _maxListItems) return;
		}
		foreach (const QStringPair& pair, items) {
			_process(matched, pair, pair.first, &_match1);
			if (_listWidget.count() >= _maxListItems) return;
		}
		foreach (const QStringPair& pair, items) {
			_process(matched, pair, pair.first, &_match2);
			if (_listWidget.count() >= _maxListItems) return;
		}
	}
	
    static bool _match1(const QString& filter, QString text) {
        if (filter.isEmpty()) {
            return true;
        }
        if (filter.toLower() == filter) {
            text = text.toLower();
        }

        return text.contains(filter);
    }

    static bool _match2(const QString& filter, QString text) {
        if (filter.isEmpty()) {
            return true;
        }
        if (filter.toLower() == filter) {
            text = text.toLower();
        }
        
        int fi = 0;
        for (int i = 0; i < text.size(); ++i) {
            if (text[i] == filter[fi]) {
                ++fi;
                if (fi == filter.size()) {
                    return true;
                }
            }
        }

        return false;
    }

    void _process(QList<const QStringPair*>& matched, const QStringPair& pair, const QString& value, bool (*matchFn)(const QString&, QString)) {
        if (matched.contains(&pair)) {
            return;
        }
        if ((*matchFn)(_filter, value)) {
            matched.append(&pair);
            QListWidgetItem* item = new QListWidgetItem(pair.second);
            _widgetToContent[item] = pair.first;
            _listWidget.addItem(item);
        }
    }

    void keyPressEvent(QKeyEvent* event) {
        if (event->key() == Qt::Key_Up) {
            _listWidget.setCurrentRow(std::max(0, _listWidget.currentRow() - 1));
        } else if (event->key() == Qt::Key_Down) {
            _listWidget.setCurrentRow(std::min(_listWidget.count(), _listWidget.currentRow() + 1));
        } else if (event->key() == Qt::Key_Escape) {
			_close();
        } else if (event->key() == Qt::Key_G && (event->modifiers() & Qt::ControlModifier)) {
			_close();
        }
        QMainWindow::keyPressEvent(event);
    }

	bool _chosen;
	const char* _separator;
	const char* _cacheFile;
	unsigned int _maxListItems;
    QWidget _containerWidget;
    QString _filter;
    QListWidget _listWidget;
    QLineEdit _lineEdit;
	QList<QString> _nonCacheItems;
    QList<QStringPair> _items;
    QMap<QListWidgetItem*, QString> _widgetToContent;
};
