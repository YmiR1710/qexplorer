#include "mainwindow.h"
#include <QApplication>
#include <QDesktopWidget>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setWindowIcon(QIcon("icons/icon.png"));
    MainWindow w;
    w.setWindowTitle("QExplorer");
    QRect rec = QApplication::desktop()->screenGeometry();
    w.resize(rec.width()/2, rec.height()/2);
    w.setStyleSheet("QMainWindow {background: 'black';}");
    w.show();
    return a.exec();
}
