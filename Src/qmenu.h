#pragma once

#include <QMainWindow>
#include <QListWidget>
#include <QListWidgetItem>
#include <QLineEdit>
#include <QMap>
#include <QVBoxLayout>
#include <QKeyEvent>
#include <QApplication>

class Menu : public QMainWindow {
    Q_OBJECT
    typedef QPair<QString, QString> QStringPair;
public:
    Menu(int width, int height, const QList<QString>& values, const char* separator = NULL)
        : QMainWindow(0, Qt::Popup)
    {
        if (values.size() == 0) {
            QApplication::exit(0);
        }

        setCentralWidget(&_containerWidget);

        QVBoxLayout* layout = new QVBoxLayout();
        layout->setContentsMargins(0, 0, 0, 0);
        layout->addWidget(&_listWidget);
        layout->addWidget(&_lineEdit);
        _containerWidget.setLayout(layout);

        foreach(const QString& val, values) {
            if (separator && val.contains(separator)) {
                int idx = val.indexOf(separator);
                _items.append(QStringPair(val.left(idx), val.right(val.size() - idx - strlen(separator))));
            } else {
                _items.append(QStringPair(val, val));
            }
        }

        resize(width, height);

        _listWidget.setFocusPolicy(Qt::NoFocus);
        _lineEdit.setFocus(Qt::OtherFocusReason);

        _rebuildList();

        connect(&_lineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(_rebuildList()));
        connect(&_lineEdit, SIGNAL(editingFinished()), this, SLOT(_done()));
        connect(&_listWidget, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(_done()));
    }

private slots:
    void _done() {
        if (_listWidget.count() > 0) {
            QListWidgetItem* item = _listWidget.currentItem();
            if (!item) {
                item = _listWidget.item(0);
            }
            printf(qPrintable(_widgetToContent[item]));
        }
        QApplication::exit(0);
    }

    void _rebuildList() {
        if (_filter.isNull() || (_filter != _lineEdit.text())) {
            _filter = _lineEdit.text();
            _listWidget.clear();
            _widgetToContent.clear();

            QList<const QStringPair*> matched;

            foreach (const QStringPair& pair, _items) {
                _process(matched, pair, pair.second, &_match1);
            }

            foreach (const QStringPair& pair, _items) {
                _process(matched, pair, pair.second, &_match2);
            }

            foreach (const QStringPair& pair, _items) {
                _process(matched, pair, pair.first, &_match1);
            }

            foreach (const QStringPair& pair, _items) {
                _process(matched, pair, pair.first, &_match2);
            }
            
            if (_listWidget.count() > 0) {
                _listWidget.setCurrentRow(0);
            }
        }
    }

private:
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

private:
    void keyPressEvent(QKeyEvent* event) {
        if (event->key() == Qt::Key_Up) {
            _listWidget.setCurrentRow(std::max(0, _listWidget.currentRow() - 1));
        } else if (event->key() == Qt::Key_Down) {
            _listWidget.setCurrentRow(std::min(_listWidget.count(), _listWidget.currentRow() + 1));
        } else if (event->key() == Qt::Key_Escape) {
            QApplication::exit(0);
        } else if (event->key() == Qt::Key_G && (event->modifiers() & Qt::ControlModifier)) {
            QApplication::exit(0);
        }
        QMainWindow::keyPressEvent(event);
    }

    QWidget _containerWidget;
    QString _filter;
    QListWidget _listWidget;
    QLineEdit _lineEdit;
    QList<QStringPair> _items;
    QMap<QListWidgetItem*, QString> _widgetToContent;
};
