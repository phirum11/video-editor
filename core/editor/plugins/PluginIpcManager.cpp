#include "PluginIpcManager.h"
#include <QDebug>

PluginIpcManager::PluginIpcManager(QObject *parent)
    : QObject(parent)
{
}

PluginIpcManager::~PluginIpcManager()
{
    disconnectIpc();
}

bool PluginIpcManager::initializeSharedMemory(const QString& sharedKey, int sizeBytes)
{
    if (m_isConnected) {
        disconnectIpc();
    }

    m_sharedMem.setKey(sharedKey);
    if (m_sharedMem.isAttached()) {
        m_sharedMem.detach();
    }

    if (m_sharedMem.create(sizeBytes)) {
        m_isConnected = true;
        emit ipcConnectionChanged();
        qDebug() << "PluginIpcManager initialized shared memory segment:" << sharedKey << "Size:" << sizeBytes;
        return true;
    }

    qWarning() << "PluginIpcManager failed to create shared memory:" << m_sharedMem.errorString();
    return false;
}

void PluginIpcManager::disconnectIpc()
{
    if (m_sharedMem.isAttached()) {
        m_sharedMem.detach();
    }
    if (m_isConnected) {
        m_isConnected = false;
        emit ipcConnectionChanged();
        qDebug() << "PluginIpcManager IPC disconnected";
    }
}

bool PluginIpcManager::sendPluginMessage(const QString& pluginName, const QVariantMap& message)
{
    qDebug() << "PluginIpcManager sending message to plugin:" << pluginName << message;
    if (!m_isConnected) {
        qWarning() << "PluginIpcManager IPC not connected, message dropped.";
        return false;
    }
    // Simulate successful IPC handoff
    return true;
}
