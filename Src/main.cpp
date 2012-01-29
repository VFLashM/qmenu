#include <QApplication>

#include "qmenu.h"
#include "stdinReader.h"

int main(int argc, char** argv) {
    QApplication app(argc, argv);

    const char* separator = NULL;

    for (int i = 1; i < argc-1; ++i) {
        if (strcmp("-s", argv[i]) == 0) {
            separator = argv[i+1];
        }
    }
    
    Menu menu(500, 500, QList<QString>(), separator);
    menu.setVisible(true);
	menu.activateWindow();

	StdinReader reader;
	QObject::connect(&reader, SIGNAL(onNewItem(const QString&)), &menu, SLOT(addItem(const QString&)));
	reader.start();

    return app.exec();
}
