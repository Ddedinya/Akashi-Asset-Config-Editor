#include "include/program.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Program Program;
    Program.show();
    return app.exec();
}
