#pragma once

#include <QAbstractListModel>
#include <QString>
#include <QList>
#include <QUrl>

struct EffectItem {
    QString title;
    QUrl fileUrl;
    QString category;
};

class EffectHubModel : public QAbstractListModel {
    Q_OBJECT

public:
    enum Roles {
        TitleRole = Qt::UserRole + 1,
        FileUrlRole,
        CategoryRole
    };

    explicit EffectHubModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void loadEffects(const QString& category = "All");
    Q_INVOKABLE void search(const QString& query);

private:
    void scanDirectory(const QString& path);
    QList<EffectItem> m_effects;
    QList<EffectItem> m_allEffects;
    QString m_currentCategory;
    QString m_searchQuery;
};
