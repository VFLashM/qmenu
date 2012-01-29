#include <QApplication>

#include "qmenu.h"
#include "stdinReader.h"

int main(int argc, char** argv) {
    QApplication app(argc, argv);

    const char* separator = NULL;
	const char* cacheFile = NULL;
	unsigned int maxListItems = -1;

    for (int i = 1; i < argc-1; ++i) {
        if (strcmp("-s", argv[i]) == 0) {
            separator = argv[i+1];
        }
		if (strcmp("-c", argv[i]) == 0) {
            cacheFile = argv[i+1];
        }
		if (strcmp("-m", argv[i]) == 0) {
			maxListItems = QString(argv[i+1]).toUInt();
        }
    }
    
    Menu menu(500, 500, QList<QString>(), separator, cacheFile, maxListItems);
    menu.setVisible(true);
	menu.activateWindow();

	StdinReader reader;
	QObject::connect(&reader, SIGNAL(onNewItem(const QString&)), &menu, SLOT(addItem(const QString&)));
	QObject::connect(&reader, SIGNAL(onDone()), &menu, SLOT(flushCache()));
	QObject::connect(&menu, SIGNAL(onDone()), &reader, SLOT(die()));
	reader.start();

    int retcode = app.exec();
	reader.wait();

	return retcode;
}
