#ifndef LISTPATTERNTEMPLATES_H
#define LISTPATTERNTEMPLATES_H

#include <QAbstractListModel>

class ListPatternTemplates : public QAbstractListModel
{
    Q_OBJECT

public:
    static ListPatternTemplates *instance();
    void addTemplate(const QString &name);
    void removeTemplate(const QModelIndex &index);
    const QString &getId(int row) const;
    const QString &getId(const QModelIndex &index) const;
    const QString &getLastId() const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

private:
    void _loadFromSettings();
    void _saveInSettings();
    explicit ListPatternTemplates(QObject *parent = nullptr);
    QList<QStringList> m_listOfStringList;
    QString m_settingsKey;
};

#endif // LISTPATTERNTEMPLATES_H
