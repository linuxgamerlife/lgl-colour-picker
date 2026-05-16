#pragma once

#include <QColor>
#include <QDialog>
#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QPointer>
#include <QProcess>
#include <QVariant>
#include <QWidget>

class QGridLayout;
class QPushButton;

class ScreenPickerOverlay : public QWidget {
    Q_OBJECT
public:
    explicit ScreenPickerOverlay(QWidget* parent = nullptr);

signals:
    void colourPicked(const QColor& colour);
    void cancelled();

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
};

class ColourPicker : public QDialog {
    Q_OBJECT
public:
    explicit ColourPicker(QWidget* parent = nullptr);

protected:
    void keyPressEvent(QKeyEvent* event) override;

private:
    struct FormatRow {
        QLineEdit* value = nullptr;
        QPushButton* copy = nullptr;
    };

    FormatRow makeFormatRow(const QString& labelText, QGridLayout* layout, int row);
    void startScreenPick();
    void setCurrentColour(const QColor& colour);
    void copyFormat(const QLineEdit* field);
    QString hexText() const;
    QString rgbText() const;
    QString rgbaText() const;
    void updateCopyButtons();
    bool startPortalPick();
    bool startWaylandFallbackPick();
    bool startProcessPick(const QString& program, const QStringList& arguments);
    QColor parsePortalColour(const QVariant& value) const;

private slots:
    void onPortalResponse(uint response, const QVariantMap& results);
    void onProcessPickFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    QFrame* m_preview = nullptr;
    QLabel* m_status = nullptr;
    QPushButton* m_pickButton = nullptr;
    FormatRow m_hex;
    FormatRow m_rgb;
    FormatRow m_rgba;
    QColor m_currentColour = QColor(88, 166, 255);
    QPointer<ScreenPickerOverlay> m_overlay;
    QPointer<QProcess> m_pickProcess;
    QString m_activeRequestPath;
};
