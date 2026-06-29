#pragma once

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QSharedMemory>

class PluginIpcManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isIpcConnected READ isIpcConnected NOTIFY ipcConnectionChanged)

public:
    explicit PluginIpcManager(QObject *parent = nullptr);
    ~PluginIpcManager() override;

    bool isIpcConnected() const { return m_isConnected; }

    Q_INVOKABLE bool initializeSharedMemory(const QString& sharedKey, int sizeBytes);
    Q_INVOKABLE void disconnectIpc();
    Q_INVOKABLE bool sendPluginMessage(const QString& pluginName, const QVariantMap& message);

signals:
    void ipcConnectionChanged();
    void pluginMessageReceived(const QString& pluginName, const QVariantMap& message);

private:
    QSharedMemory m_sharedMem;
    bool m_isConnected = false;
};
