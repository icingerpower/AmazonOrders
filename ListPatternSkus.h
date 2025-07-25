#ifndef LISTPATTERNSKUS_H
#define LISTPATTERNSKUS_H

#include <QAbstractListModel>

class ListPatternSkus : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit ListPatternSkus(const QString &id, QObject *parent = nullptr);
    void addPattern(const QString &pattern);
    void removePattern(const QModelIndex &index);

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    virtual QString postFixSettings() const;

    const QStringList &patterns() const;

public slots:
    void load();

private:
    QStringList m_patterns;
    QString m_id;
    QString m_settingsKey;
    void _saveInSettings();
};

#endif // LISTPATTERNSKUS_H
