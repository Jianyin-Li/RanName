#include <QApplication>
#include <QTranslator>
#include <QFileInfo>
#include "main_window.h"
#include "../core/config_manager.h"
#include "../i18n/localizer.h"
#include "../utils/platform.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setApplicationName("RandomNamePicker");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("Jianyin Li");

    utils::Platform::setUTF8Encoding();
    utils::Platform::createDirectory("data");

    config::ConfigManager config;
    config.loadFromFile("data/config.conf");
    std::string langStr = config.getLanguage();

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--lang" && i + 1 < argc) {
            langStr = argv[++i];
        }
    }

    i18n::Localizer::setLanguage(langStr);

    // Load Qt UI translation (.qm embedded via resources.qrc) based on --lang or config.
    // Without this, QCoreApplication::translate()/tr() always falls back to the source text (English).
    QTranslator qtTranslator;
    const QString qmPath = QStringLiteral(":/i18n/RandomNamePicker_%1.qm")
                               .arg(QString::fromStdString(langStr));
    if (QFileInfo::exists(qmPath) && qtTranslator.load(qmPath)) {
        app.installTranslator(&qtTranslator);
    }

    MainWindow window;
    window.show();

    return app.exec();
}
