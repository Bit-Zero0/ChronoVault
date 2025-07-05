#include <QApplication>
#include "gui/MainWindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // 未来可以在这里加载 QSS 样式表
    // QFile styleFile(":/styles/style.qss");
    // styleFile.open(QFile::ReadOnly);
    // app.setStyleSheet(QLatin1String(styleFile.readAll()));

    QFont font("Microsoft YaHei UI");
    QApplication::setFont(font);
    MainWindow mainWindow;
    mainWindow.show();

    return app.exec();
}
