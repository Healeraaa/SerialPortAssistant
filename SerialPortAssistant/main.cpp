#include "SerialPortAssistant.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);      // 启用高 DPI 缩放（核心）
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);         // 让图标更清晰（强烈推荐）
    QApplication app(argc, argv);
    SerialPortAssistant window;
    window.show();
    return app.exec();
}
