#pragma once

#include <QObject>
#include <QString>
#include <QImage>
#include <QVector>
#include <QVariantMap>

struct XcfParsedLayer {
    QString name;
    int width;
    int height;
    int offsetX;
    int offsetY;
    qreal opacity;
    QString blendMode;
    QImage buffer;
};

class XcfFileStreamParser : public QObject
{
    Q_OBJECT

public:
    explicit XcfFileStreamParser(QObject *parent = nullptr);
    ~XcfFileStreamParser() override;

    Q_INVOKABLE bool parseXcfProject(const QString& filePath);
    Q_INVOKABLE bool saveXcfProject(const QString& filePath, const QVariantMap& projectMetadata, const QVariantList& layerList);

signals:
    void xcfParsed(bool success, const QVariantMap& projectMetadata, const QVariantList& layerList);
    void xcfSaved(bool success);

private:
    bool readBinaryHeader(QIODevice& device, QVariantMap& metadata);
};
