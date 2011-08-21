#include "qmenu.h"

#include <iostream>
#include <QApplication>
#include <QTextStream>

int main(int argc, char** argv) {
    QApplication app(argc, argv);

    const char* separator = NULL;

    for (int i = 1; i < argc-1; ++i) {
        if (strcmp("-s", argv[i]) == 0) {
            separator = argv[i+1];
        }
    }

    QList<QString> values;

    while (std::cin) {
        std::string str;
        std::cin >> str; 
        values.append(str.c_str());
    }
    
    //QTextStream stream(stdin);
    /*
    QString line;
    do {
         line = stream.readLine();
         values.append(line);
    } while (!line.isNull());
    */
    
    Menu menu(500, 500, values, separator);
    menu.setVisible(true);

    return app.exec();
}