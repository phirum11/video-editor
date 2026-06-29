#pragma once

#include <QQuickItem>
#include <QString>
#include <QColor>
#include <QVector>

class HardwareWaveformItem : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(QString filePath READ filePath WRITE setFilePath NOTIFY filePathChanged)
    Q_PROPERTY(double sourceDuration READ sourceDuration WRITE setSourceDuration NOTIFY sourceDurationChanged)
    Q_PROPERTY(double pixelsPerSecond READ pixelsPerSecond WRITE setPixelsPerSecond NOTIFY pixelsPerSecondChanged)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    
public:
    explicit HardwareWaveformItem(QQuickItem *parent = nullptr);
    ~HardwareWaveformItem() override;

    QString filePath() const;
    void setFilePath(const QString &filePath);

    double sourceDuration() const;
    void setSourceDuration(double sourceDuration);

    double pixelsPerSecond() const;
    void setPixelsPerSecond(double pixelsPerSecond);

    QColor color() const;
    void setColor(const QColor &color);

protected:
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *updatePaintNodeData) override;

signals:
    void filePathChanged();
    void sourceDurationChanged();
    void pixelsPerSecondChanged();
    void colorChanged();
    
    // Internal signal to trigger update on main thread after async decode
    void peaksReady(const QVector<float> &peaks);

private slots:
    void onPeaksReady(const QVector<float> &peaks);

private:
    void fetchPeaksAsync();

    QString m_filePath;
    double m_sourceDuration = 0.0;
    double m_pixelsPerSecond = 100.0;
    QColor m_color = QColor("#009090");
    
    QVector<float> m_peaks;
    bool m_loading = false;
};
