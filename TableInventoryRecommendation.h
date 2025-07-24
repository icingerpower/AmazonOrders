#ifndef TABLEINVENTORYRECOMMENDATION_H
#define TABLEINVENTORYRECOMMENDATION_H

#include <QAbstractTableModel>

class TableInventoryRecommendation : public QAbstractTableModel
{
    Q_OBJECT

public:
    static TableInventoryRecommendation *instance();

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

public slots:
    void pasteText(const QString &text);
    void clear();

private:
    explicit TableInventoryRecommendation(QObject *parent = nullptr);
    static const QStringList HEADER;
    static const int IND_SKU;
    static const int IND_UNIT_SOLD_30DAYS;
    static const int IND_UNIT_SUPPLY;
    static const int IND_DAY_SUPPLY;
    static const int IND_RECOMMENDED_QTY;
    QList<QVariantList> m_listOfVariantList;
};

#endif // TABLEINVENTORYRECOMMENDATION_H
