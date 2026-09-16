#include <QApplication>
#include <QFile>
#include <QIcon>
#include <QStyleFactory>
#include "src/ui/MainWindow.h"

/**
 * @brief 程序入口
 */
int main(int argc, char *argv[]) {
    // Fusion 基础风格：跨平台一致，且对 QSS 样式表支持最好
    QApplication::setStyle(QStyleFactory::create("Fusion"));

    QApplication app(argc, argv);

    app.setApplicationName("TreeGraph Studio");
    app.setOrganizationName("C++Course");
    app.setApplicationVersion("2.0");

    const QIcon appIcon(":/icons/app-icon.png");
    app.setWindowIcon(appIcon);

    // 加载全局浅色主题（读取失败也不影响程序运行）
    QFile qss(":/themes/light.qss");
    if (qss.open(QFile::ReadOnly | QFile::Text)) {
        app.setStyleSheet(QString::fromUtf8(qss.readAll()));
        qss.close();
    }

    MainWindow window;
    window.resize(1280, 720);
    window.setWindowTitle("TreeGraph Studio - 树图教学系统");
    window.setWindowIcon(appIcon);
    window.show();

    return app.exec();
}
