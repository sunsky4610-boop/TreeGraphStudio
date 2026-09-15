#include <QApplication>
#include "src/ui/MainWindow.h"

/**
 * @brief 程序入口
 */
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    app.setApplicationName("TreeGraph Studio");
    app.setOrganizationName("C++Course");
    app.setApplicationVersion("2.0");

    MainWindow window;
    window.resize(1280, 720);
    window.setWindowTitle("TreeGraph Studio - 树图教学系统");
    window.show();

    return app.exec();
}
