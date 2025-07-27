#ifndef LISTORDERMODEL_H
#define LISTORDERMODEL_H


#include <QAbstractListModel>

class ListOrderModel : public QAbstractListModel
{
    Q_OBJECT
public:
    ListOrderModel(const QString &countryCode, QObject *parent = nullptr);

    const QStringList &getFilePaths() const;

    void addFile(const QString &filePath);
    void removeFile(const QModelIndex &index);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

private:
    void _loadFromSettings();
    void _saveInSettings();
    QStringList m_filePaths;
    QString m_settingsKey;
};

#endif // LISTORDERMODEL_H
