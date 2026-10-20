#include "colourpicker.h"

#include <QApplication>
#include <QFont>
#include <QIcon>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("lgl-colour-picker");
    app.setApplicationDisplayName("LGL Simple Colour Picker");
    app.setDesktopFileName("lgl-colour-picker");
    app.setApplicationVersion("1.0.1");
    app.setWindowIcon(QIcon(QStringLiteral(":/icons/packaging/icons/256x256/lgl-colour-picker.png")));

    QFont appFont = app.font();
    appFont.setPointSize(appFont.pointSize() + 2);
    app.setFont(appFont);

    ColourPicker picker;
    picker.show();

    return app.exec();
}
