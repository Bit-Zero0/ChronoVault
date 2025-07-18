#include <QApplication>
#include <QFont>
#include "gui/MainWindow.h"
#include "services/TodoService.h"
#include <QSystemTrayIcon>
#include <QIcon>
#include <QMessageBox>
#include <QStyle> // <-- 【重要】引入 QStyle 头文件

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(true);

    QFont font("Microsoft YaHei UI");
    QApplication::setFont(font);

    // --- 【终极测试】使用一个绝对不会出错的系统内置图标 ---
    // QIcon icon(":/icons/logo1.png"); // 暂时注释掉您自己的图标
    QIcon icon = app.style()->standardIcon(QStyle::SP_ComputerIcon); // 直接获取系统“电脑”图标
    // ----------------------------------------------------

    // 检查图标是否加载成功
    if (icon.isNull()) {
        qWarning("Failed to load application icon.");
    }

    QSystemTrayIcon trayIcon(&app);
    trayIcon.setIcon(icon);
    trayIcon.setToolTip("ChronoVault");
    trayIcon.show();

    if (!trayIcon.isVisible()) {
        QMessageBox::warning(nullptr, "警告",
            "无法创建系统托盘图标。\n"
                                                   "这可能是由于您的操作系统环境限制。\n"
                                                   "提醒功能将无法正常工作。");
    }

    TodoService::instance()->setTrayIcon(&trayIcon);

    MainWindow mainWindow;
    mainWindow.show();

    return app.exec();
}
