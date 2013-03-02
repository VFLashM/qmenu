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
#include <QFile>

struct MenuItem {
    QString text;
    QString result;
    QString hint;

    MenuItem() {}
    MenuItem(const QString& text_) : text(text_), result(text_) {}
    MenuItem(const QString& text_, const QString& result_) : text(text_), result(result_) {}
    MenuItem(const QString& text_, const QString& result_, const QString& hint_) : text(text_), result(result_), hint(hint_) {}

    bool operator== (const MenuItem& other) const { return text == other.text && result == other.result && hint == other.hint; } 
};

class Menu : public QMainWindow {
    Q_OBJECT
public:
    Menu(int width, int height, const QList<MenuItem>& values, const char* cacheFile = NULL, unsigned int maxListItems = -1)
        : QMainWindow(0, 0)//Qt::SplashScreen | Qt::WindowStaysOnTopHint)
		, _chosen(false)
		, _cacheFile(cacheFile)
		, _maxListItems(maxListItems)
        , _maxWidth(_startingMaxWidth)
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
	void addItem(const MenuItem& item) {
		if (!_items.contains(item)) {
			_items.append(item);
			QList<MenuItem> singleItemList;
			singleItemList.append(item);
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
			foreach(const MenuItem& item, _nonCacheItems) {
				outs << item.text  << "\n";
                outs << item.result << "\n";
                outs << item.hint  << "\n";
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
            _maxWidth = _startingMaxWidth;
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
	
	void _addItems(const QList<MenuItem>& values, bool fromCache) {
        QList<MenuItem> toAdd;
		foreach(const MenuItem& val, values) {
			if (!_items.contains(val)) {
				toAdd.append(val);
			}
			if (!fromCache) {
                if (!_nonCacheItems.contains(val)) {
					_nonCacheItems.append(val);
				}
			}
        }
		_items.append(toAdd);
		_addItemsToWidget(toAdd);
	}
	
	void _loadCache() {
		if (_cacheFile) {
			QFile cache(_cacheFile);
			if (cache.open(QFile::ReadOnly)) {
				QTextStream ins(&cache);
				QList<MenuItem> lines;
				while(true) {
					QString text   = ins.readLine();
                    QString result = ins.readLine();
                    QString hint   = ins.readLine();

                    if (text.isNull() || result.isNull() || hint.isNull()) {
                        break;
                    }

                    if (hint.isEmpty()) {
                        lines.append(MenuItem(text, result));
                    } else {
                        lines.append(MenuItem(text, result, hint));
                    }
				}
				_addItems(lines, true);
			}
		}
	}
	
	void _filterAndReorder(const QList<MenuItem>& items, QList<MenuItem>& dst) {
        dst.clear();

        foreach (const MenuItem& pair, items) {
            if (!dst.contains(pair)) {
                if (_match1(_filter, pair.text)) {
                    dst.append(pair);
                }
            }
        }

        foreach (const MenuItem& pair, items) {
            if (!dst.contains(pair)) {
                if (_match2(_filter, pair.text)) {
                    dst.append(pair);
                }
            }
        }

        foreach (const MenuItem& pair, items) {
            if (!dst.contains(pair)) {
                if (_match1(_filter, pair.hint)) {
                    dst.append(pair);
                }
            }
        }

        foreach (const MenuItem& pair, items) {
            if (!dst.contains(pair)) {
                if (_match2(_filter, pair.hint)) {
                    dst.append(pair);
                }
            }
        }
    }
	
	void _addItemsToWidget(QList<MenuItem>& items) {
        if ((unsigned int)_listWidget.count() >= _maxListItems) return;
        
        QList<MenuItem> res;
        _filterAndReorder(items, res);

        foreach (const MenuItem& pair, res) {
            _maxWidth = std::max(_maxWidth, pair.text.length());
        }

        foreach (const MenuItem& pair, res) {
            if ((unsigned int)_listWidget.count() >= _maxListItems) return;

            int marginSize = _maxWidth - pair.text.length();
            QString margin(marginSize, ' ');

            QString text;
            if (pair.hint.isNull()) {
                text = pair.text;
            } else {
                text = pair.text + margin + " --> " + pair.hint;
            }
            
            QListWidgetItem* item = new QListWidgetItem(text);

            QFont font("Courier");
            font.setStyleHint(QFont::Monospace);
            font.setPointSize(8);
            font.setFixedPitch(true);
            item->setFont(font);
            
            _widgetToContent[item] = pair.result;
            _listWidget.addItem(item);
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
	const char* _cacheFile;
	unsigned int _maxListItems;
    QWidget _containerWidget;
    QString _filter;
    QListWidget _listWidget;
    QLineEdit _lineEdit;
	QList<MenuItem> _nonCacheItems;
    QList<MenuItem> _items;
    static const int _startingMaxWidth = 32 ;
    int _maxWidth;
    QMap<QListWidgetItem*, QString> _widgetToContent;
};
