#include "colourpicker.h"

#include <QApplication>
#include <QClipboard>
#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusError>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusMetaType>
#include <QDBusObjectPath>
#include <QDir>
#include <QFileInfo>
#include <QGridLayout>
#include <QGuiApplication>
#include <QIcon>
#include <QKeyEvent>
#include <QLabel>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>
#include <QProcess>
#include <QPushButton>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QScreen>
#include <QSizePolicy>
#include <QStandardPaths>
#include <QTemporaryFile>
#include <QVBoxLayout>
#include <QHBoxLayout>

namespace {
constexpr int PROCESS_START_TIMEOUT_MS = 1000;
constexpr int PROCESS_FINISH_TIMEOUT_MS = 3000;
constexpr int INTERACTIVE_PICK_TIMEOUT_MS = 30000;
constexpr int PORTAL_CALL_TIMEOUT_MS = 5000;

bool copyWithProcess(const QString& program, const QStringList& arguments, const QString& text) {
    if (!QFileInfo::exists(program))
        return false;

    QProcess process;
    process.setProgram(program);
    process.setArguments(arguments);
    process.start(QIODevice::WriteOnly);
    if (!process.waitForStarted(PROCESS_START_TIMEOUT_MS))
        return false;

    process.write(text.toUtf8());
    process.closeWriteChannel();
    if (!process.waitForFinished(PROCESS_FINISH_TIMEOUT_MS)) {
        process.kill();
        process.waitForFinished();
        return false;
    }

    return process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0;
}

void setQtClipboard(const QString& text) {
    auto* mimeData = new QMimeData;
    mimeData->setText(text);
    QApplication::clipboard()->setMimeData(mimeData, QClipboard::Clipboard);

    if (QApplication::clipboard()->supportsSelection()) {
        auto* selectionData = new QMimeData;
        selectionData->setText(text);
        QApplication::clipboard()->setMimeData(selectionData, QClipboard::Selection);
    }
}

void copyToClipboard(const QString& text) {
    setQtClipboard(text);

    if (qEnvironmentVariableIsSet("WAYLAND_DISPLAY")) {
        if (copyWithProcess(QStringLiteral("/usr/bin/wl-copy"),
                            {QStringLiteral("--type"), QStringLiteral("text/plain;charset=utf-8")},
                            text))
            return;

        copyWithProcess(QStringLiteral("/usr/bin/wl-copy"), {}, text);
        return;
    }

    if (qEnvironmentVariableIsSet("DISPLAY")) {
        if (copyWithProcess(QStringLiteral("/usr/bin/xclip"),
                            {QStringLiteral("-selection"), QStringLiteral("clipboard")},
                            text))
            return;

        copyWithProcess(QStringLiteral("/usr/bin/xsel"),
                        {QStringLiteral("--clipboard"), QStringLiteral("--input")},
                        text);
    }
}

QRect virtualScreenGeometry() {
    QRect geometry;
    const auto screens = QGuiApplication::screens();
    for (QScreen* screen : screens)
        geometry = geometry.united(screen->geometry());
    return geometry;
}

QColor sampleScreenColour(const QPoint& globalPos) {
    QScreen* screen = QGuiApplication::screenAt(globalPos);
    if (!screen)
        screen = QGuiApplication::primaryScreen();
    if (!screen)
        return {};

    const QPoint localPos = globalPos - screen->geometry().topLeft();
    const QPixmap sample = screen->grabWindow(0, localPos.x(), localPos.y(), 1, 1);
    if (sample.isNull())
        return {};

    const QImage image = sample.toImage();
    if (image.isNull())
        return {};

    return image.pixelColor(0, 0);
}

QString findProgram(const QString& program) {
    return QStandardPaths::findExecutable(program);
}

QColor pickColourWithGrimSlurp(bool* cancelled) {
    if (cancelled)
        *cancelled = false;

    const QString slurpPath = findProgram(QStringLiteral("slurp"));
    const QString grimPath = findProgram(QStringLiteral("grim"));
    if (slurpPath.isEmpty() || grimPath.isEmpty())
        return {};

    QProcess slurp;
    slurp.setProgram(slurpPath);
    slurp.setArguments({QStringLiteral("-p")});
    slurp.start();
    if (!slurp.waitForStarted(PROCESS_START_TIMEOUT_MS))
        return {};

    if (!slurp.waitForFinished(INTERACTIVE_PICK_TIMEOUT_MS)) {
        slurp.kill();
        slurp.waitForFinished();
        if (cancelled)
            *cancelled = true;
        return {};
    }

    if (slurp.exitStatus() != QProcess::NormalExit || slurp.exitCode() != 0) {
        if (cancelled)
            *cancelled = true;
        return {};
    }

    const QString point = QString::fromUtf8(slurp.readAllStandardOutput()).trimmed();
    const QStringList parts = point.split(QLatin1Char(','));
    if (parts.size() != 2)
        return {};

    bool xOk = false;
    bool yOk = false;
    const int x = parts.at(0).toInt(&xOk);
    const int y = parts.at(1).toInt(&yOk);
    if (!xOk || !yOk)
        return {};

    QTemporaryFile output(QDir::tempPath() + QStringLiteral("/lgl-colour-picker-XXXXXX.png"));
    if (!output.open())
        return {};
    const QString outputPath = output.fileName();
    output.close();

    QProcess grim;
    grim.setProgram(grimPath);
    grim.setArguments({
        QStringLiteral("-g"),
        QStringLiteral("%1,%2 1x1").arg(x).arg(y),
        outputPath
    });
    grim.start();
    if (!grim.waitForStarted(PROCESS_START_TIMEOUT_MS))
        return {};

    if (!grim.waitForFinished(PROCESS_FINISH_TIMEOUT_MS)) {
        grim.kill();
        grim.waitForFinished();
        return {};
    }

    if (grim.exitStatus() != QProcess::NormalExit || grim.exitCode() != 0)
        return {};

    const QImage image(outputPath);
    if (image.isNull())
        return {};

    return image.pixelColor(0, 0);
}

QString portalSenderPathPart(const QString& uniqueName) {
    QString pathPart = uniqueName;
    if (pathPart.startsWith(QLatin1Char(':')))
        pathPart.remove(0, 1);
    pathPart.replace(QLatin1Char('.'), QLatin1Char('_'));
    return pathPart;
}

QString portalParentWindow() {
    // Empty is accepted by the portal and keeps this independent of X11/Wayland
    // native window-handle details.
    return {};
}
}

ScreenPickerOverlay::ScreenPickerOverlay(QWidget* parent)
    : QWidget(parent)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_DeleteOnClose);
    setCursor(Qt::CrossCursor);
    setFocusPolicy(Qt::StrongFocus);
    setGeometry(virtualScreenGeometry());
}

void ScreenPickerOverlay::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Escape) {
        emit cancelled();
        close();
        return;
    }

    QWidget::keyPressEvent(event);
}

void ScreenPickerOverlay::mousePressEvent(QMouseEvent* event) {
    const QPoint globalPos = event->globalPosition().toPoint();
    hide();
    qApp->processEvents();

    const QColor colour = sampleScreenColour(globalPos);
    if (colour.isValid())
        emit colourPicked(colour);
    else
        emit cancelled();

    close();
}

void ScreenPickerOverlay::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.fillRect(rect(), QColor(0, 0, 0, 24));
}

ColourPicker::ColourPicker(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("LGL Simple Colour Picker");
    setWindowIcon(QIcon(QStringLiteral(":/icons/packaging/icons/256x256/lgl-colour-picker.png")));
    setMinimumSize(520, 360);

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(12, 12, 12, 12);
    root->setSpacing(12);

    auto* topRow = new QHBoxLayout;
    topRow->setContentsMargins(0, 0, 0, 0);
    topRow->setSpacing(12);

    m_preview = new QFrame(this);
    m_preview->setFixedSize(112, 112);
    m_preview->setFrameShape(QFrame::StyledPanel);
    m_preview->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    topRow->addWidget(m_preview);

    auto* actionColumn = new QVBoxLayout;
    actionColumn->setContentsMargins(0, 0, 0, 0);
    actionColumn->setSpacing(8);

    m_pickButton = new QPushButton("Pick Colour", this);
    m_pickButton->setMinimumHeight(40);
    connect(m_pickButton, &QPushButton::clicked, this, &ColourPicker::startScreenPick);
    actionColumn->addWidget(m_pickButton);

    m_status = new QLabel("Click Pick Colour, then click anywhere on screen.", this);
    m_status->setWordWrap(true);
    actionColumn->addWidget(m_status);
    actionColumn->addStretch();
    topRow->addLayout(actionColumn, 1);
    root->addLayout(topRow);

    auto* valuesGrid = new QGridLayout;
    valuesGrid->setContentsMargins(0, 0, 0, 0);
    valuesGrid->setHorizontalSpacing(8);
    valuesGrid->setVerticalSpacing(8);
    m_hex = makeFormatRow("HEX", valuesGrid, 0);
    m_rgb = makeFormatRow("RGB", valuesGrid, 1);
    m_rgba = makeFormatRow("RGBA", valuesGrid, 2);
    root->addLayout(valuesGrid);
    root->addStretch();

    QScreen* screen = QGuiApplication::primaryScreen();
    if (screen) {
        const QRect geom = screen->availableGeometry();
        const int w = qMin(int(geom.width() * 0.36), 560);
        const int h = qMin(int(geom.height() * 0.36), 380);
        resize(w, h);
        move(geom.center() - QPoint(w / 2, h / 2));
    }

    setCurrentColour(m_currentColour);
}

ColourPicker::~ColourPicker() {
    cleanupActivePick();
}

ColourPicker::FormatRow ColourPicker::makeFormatRow(const QString& labelText, QGridLayout* layout, int row) {
    auto* label = new QLabel(labelText, this);
    label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    layout->addWidget(label, row, 0);

    auto* field = new QLineEdit(this);
    field->setReadOnly(true);
    field->setMinimumHeight(36);
    layout->addWidget(field, row, 1);

    auto* copy = new QPushButton("Copy", this);
    copy->setMinimumHeight(36);
    copy->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(copy, &QPushButton::clicked, this, [this, field] { copyFormat(field); });
    layout->addWidget(copy, row, 2);

    return {field, copy};
}

void ColourPicker::startScreenPick() {
    if (m_overlay || m_pickProcess)
        return;

    if (qEnvironmentVariableIsSet("WAYLAND_DISPLAY")) {
        m_status->setText("Click a screen pixel to pick its colour.");
        if (startPortalPick())
            return;

        if (!startWaylandFallbackPick()) {
            raise();
            activateWindow();
            m_status->setText("Could not pick a colour. Your Wayland portal backend must support color picking.");
        }
        return;
    }

    m_status->setText("Click a screen pixel to pick its colour. Press Escape to cancel.");
    m_overlay = new ScreenPickerOverlay;
    connect(m_overlay, &ScreenPickerOverlay::colourPicked, this, [this](const QColor& colour) {
        setCurrentColour(colour);
        show();
        raise();
        activateWindow();
        m_status->setText("Colour picked.");
    });
    connect(m_overlay, &ScreenPickerOverlay::cancelled, this, [this] {
        show();
        raise();
        activateWindow();
        m_status->setText("Colour picking cancelled.");
    });
    connect(m_overlay, &QObject::destroyed, this, [this] {
        m_overlay = nullptr;
    });

    m_overlay->show();
    m_overlay->raise();
    m_overlay->activateWindow();
    m_overlay->setFocus();
}

void ColourPicker::cleanupActivePick() {
    if (!m_activeRequestPath.isEmpty()) {
        QDBusConnection::sessionBus().disconnect(QStringLiteral("org.freedesktop.portal.Desktop"),
                                                 m_activeRequestPath,
                                                 QStringLiteral("org.freedesktop.portal.Request"),
                                                 QStringLiteral("Response"),
                                                 this,
                                                 SLOT(onPortalResponse(uint,QVariantMap)));
        m_activeRequestPath.clear();
    }

    if (m_pickProcess) {
        disconnect(m_pickProcess, nullptr, this, nullptr);
        if (m_pickProcess->state() != QProcess::NotRunning) {
            m_pickProcess->terminate();
            if (!m_pickProcess->waitForFinished(PROCESS_FINISH_TIMEOUT_MS)) {
                m_pickProcess->kill();
                m_pickProcess->waitForFinished();
            }
        }
        m_pickProcess->deleteLater();
        m_pickProcess = nullptr;
    }

    if (m_overlay) {
        disconnect(m_overlay, nullptr, this, nullptr);
        m_overlay->close();
        m_overlay = nullptr;
    }
}

bool ColourPicker::startPortalPick() {
    QDBusConnection bus = QDBusConnection::sessionBus();
    if (!bus.isConnected())
        return false;

    const QString token = QStringLiteral("lgl_colour_picker_%1")
        .arg(QRandomGenerator::global()->generate());
    const QString sender = portalSenderPathPart(bus.baseService());
    const QString requestPath = QStringLiteral("/org/freedesktop/portal/desktop/request/%1/%2")
        .arg(sender, token);

    QVariantMap options;
    options.insert(QStringLiteral("handle_token"), token);

    QDBusMessage message = QDBusMessage::createMethodCall(
        QStringLiteral("org.freedesktop.portal.Desktop"),
        QStringLiteral("/org/freedesktop/portal/desktop"),
        QStringLiteral("org.freedesktop.portal.Screenshot"),
        QStringLiteral("PickColor"));
    message << portalParentWindow() << options;

    if (!bus.connect(QStringLiteral("org.freedesktop.portal.Desktop"),
                     requestPath,
                     QStringLiteral("org.freedesktop.portal.Request"),
                     QStringLiteral("Response"),
                     this,
                     SLOT(onPortalResponse(uint,QVariantMap)))) {
        return false;
    }

    QDBusMessage reply = bus.call(message, QDBus::Block, PORTAL_CALL_TIMEOUT_MS);
    if (reply.type() == QDBusMessage::ErrorMessage) {
        bus.disconnect(QStringLiteral("org.freedesktop.portal.Desktop"),
                       requestPath,
                       QStringLiteral("org.freedesktop.portal.Request"),
                       QStringLiteral("Response"),
                       this,
                       SLOT(onPortalResponse(uint,QVariantMap)));
        return false;
    }

    m_activeRequestPath = requestPath;
    return true;
}

bool ColourPicker::startWaylandFallbackPick() {
    const QString niriPath = findProgram(QStringLiteral("niri"));
    const QString desktop = qEnvironmentVariable("XDG_CURRENT_DESKTOP").toLower();
    if (!niriPath.isEmpty() && desktop.contains(QStringLiteral("niri"))
        && startProcessPick(niriPath, {QStringLiteral("msg"), QStringLiteral("pick-color")}))
        return true;

    bool cancelled = false;
    const QColor colour = pickColourWithGrimSlurp(&cancelled);
    show();
    raise();
    activateWindow();

    if (colour.isValid()) {
        setCurrentColour(colour);
        m_status->setText("Colour picked.");
        return true;
    }

    if (cancelled) {
        m_status->setText("Colour picking cancelled.");
        return true;
    }

    return false;
}

bool ColourPicker::startProcessPick(const QString& program, const QStringList& arguments) {
    auto* process = new QProcess(this);
    process->setProgram(program);
    process->setArguments(arguments);
    connect(process, &QProcess::finished, this, &ColourPicker::onProcessPickFinished);
    connect(process, &QObject::destroyed, this, [this] {
        m_pickProcess = nullptr;
    });
    process->start();
    if (!process->waitForStarted(PROCESS_START_TIMEOUT_MS)) {
        process->deleteLater();
        return false;
    }

    m_pickProcess = process;
    return true;
}

QColor ColourPicker::parsePortalColour(const QVariant& value) const {
    double red = -1.0;
    double green = -1.0;
    double blue = -1.0;

    if (value.canConvert<QDBusArgument>()) {
        const QDBusArgument argument = value.value<QDBusArgument>();
        argument.beginStructure();
        argument >> red >> green >> blue;
        argument.endStructure();
    } else if (value.typeId() == QMetaType::QVariantList) {
        const QVariantList values = value.toList();
        if (values.size() >= 3) {
            red = values.at(0).toDouble();
            green = values.at(1).toDouble();
            blue = values.at(2).toDouble();
        }
    }

    if (red < 0.0 || green < 0.0 || blue < 0.0)
        return {};

    return QColor::fromRgbF(qBound(0.0, red, 1.0),
                            qBound(0.0, green, 1.0),
                            qBound(0.0, blue, 1.0));
}

void ColourPicker::onPortalResponse(uint response, const QVariantMap& results) {
    if (!m_activeRequestPath.isEmpty()) {
        QDBusConnection::sessionBus().disconnect(QStringLiteral("org.freedesktop.portal.Desktop"),
                                                 m_activeRequestPath,
                                                 QStringLiteral("org.freedesktop.portal.Request"),
                                                 QStringLiteral("Response"),
                                                 this,
                                                 SLOT(onPortalResponse(uint,QVariantMap)));
        m_activeRequestPath.clear();
    }

    if (response != 0) {
        if (response == 2 && startWaylandFallbackPick())
            return;

        show();
        raise();
        activateWindow();
        m_status->setText("Colour picking cancelled.");
        return;
    }

    const QColor colour = parsePortalColour(results.value(QStringLiteral("color")));
    if (!colour.isValid()) {
        if (startWaylandFallbackPick())
            return;

        show();
        raise();
        activateWindow();
        m_status->setText("Could not read the colour returned by the Wayland portal.");
        return;
    }

    show();
    raise();
    activateWindow();
    setCurrentColour(colour);
    m_status->setText("Colour picked.");
}

void ColourPicker::onProcessPickFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    auto* process = qobject_cast<QProcess*>(sender());
    if (!process)
        return;

    const QString output = QString::fromUtf8(process->readAllStandardOutput()).trimmed();
    if (m_pickProcess == process)
        m_pickProcess = nullptr;
    process->deleteLater();

    show();
    raise();
    activateWindow();

    if (exitStatus != QProcess::NormalExit || exitCode != 0) {
        m_status->setText("Colour picking cancelled.");
        return;
    }

    QColor colour;
    static const QRegularExpression whitespace(QStringLiteral("\\s+"));
    const QStringList words = output.split(whitespace, Qt::SkipEmptyParts);
    for (const QString& word : words) {
        QColor candidate(word.trimmed());
        if (candidate.isValid()) {
            colour = candidate;
            break;
        }
    }

    if (!colour.isValid()) {
        m_status->setText("Could not read the colour returned by the compositor.");
        return;
    }

    setCurrentColour(colour);
    m_status->setText("Colour picked.");
}

void ColourPicker::setCurrentColour(const QColor& colour) {
    if (!colour.isValid())
        return;

    m_currentColour = colour;
    m_hex.value->setText(hexText());
    m_rgb.value->setText(rgbText());
    m_rgba.value->setText(rgbaText());
    m_preview->setStyleSheet(QStringLiteral(
        "QFrame {"
        "  background: %1;"
        "  border: 1px solid palette(mid);"
        "  border-radius: 6px;"
        "}"
    ).arg(hexText()));
    updateCopyButtons();
}

void ColourPicker::copyFormat(const QLineEdit* field) {
    if (!field)
        return;

    copyToClipboard(field->text());
    m_status->setText(QStringLiteral("Copied %1").arg(field->text()));
}

QString ColourPicker::hexText() const {
    return m_currentColour.name(QColor::HexRgb).toUpper();
}

QString ColourPicker::rgbText() const {
    return QStringLiteral("rgb(%1, %2, %3)")
        .arg(m_currentColour.red())
        .arg(m_currentColour.green())
        .arg(m_currentColour.blue());
}

QString ColourPicker::rgbaText() const {
    return QStringLiteral("rgba(%1, %2, %3, %4)")
        .arg(m_currentColour.red())
        .arg(m_currentColour.green())
        .arg(m_currentColour.blue())
        .arg(m_currentColour.alpha());
}

void ColourPicker::updateCopyButtons() {
    m_hex.copy->setToolTip(QStringLiteral("Copy %1").arg(m_hex.value->text()));
    m_rgb.copy->setToolTip(QStringLiteral("Copy %1").arg(m_rgb.value->text()));
    m_rgba.copy->setToolTip(QStringLiteral("Copy %1").arg(m_rgba.value->text()));
}

void ColourPicker::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Escape)
        close();
    else
        QDialog::keyPressEvent(event);
}
