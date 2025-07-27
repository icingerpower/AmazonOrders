#ifndef TABLEINVENTORYRECOMMENDATION_H
#define TABLEINVENTORYRECOMMENDATION_H

#include <QAbstractTableModel>
#include <QImage>


class TableInventoryRecommendation : public QAbstractTableModel
{
    Q_OBJECT

public:
    static const int IND_SKU;
    static const int IND_TITLE;
    static const int IND_UNIT_SOLD_30DAYS;
    static const int IND_UNIT_SUPPLY;
    static const int IND_DAY_SUPPLY;
    static const int IND_RECOMMENDED_QTY;
    static const int IND_PRICE;
    static const int IND_FNSKU;
    static const int IND_ASIN;
    static TableInventoryRecommendation *instance();

    QMap<QString, int> get_skusReco_quantity() const;
    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    void clear(const QStringList &patternSkus, const QStringList &patternTitles, bool isWhiteList);

public slots:
    int pasteText(const QString &text);
    void clear();
    void clearNotRecommended();
    void save();
    void load();

private:
    explicit TableInventoryRecommendation(QObject *parent = nullptr);
    static const QStringList HEADER;
    QList<QVariantList> m_listOfVariantList;
    bool _lineMatch(int indexLine,
                    const QStringList &patternSkus,
                    const QStringList &patternTitles,
                    bool isWhiteList) const;
    QString m_settingsKey;


};

#endif // TABLEINVENTORYRECOMMENDATION_H
