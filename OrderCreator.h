#ifndef ORDERCREATOR_H
#define ORDERCREATOR_H

#include <QString>
#include <QDir>

#include <xlsxdocument.h>

class OrderCreator
{
public:
    OrderCreator(const QStringList &xlsxFilePathsFrom,
                 const QMap<QString, int> &skusReco_quantity,
                 const QMap<QString, int> &skusNoInv_customReco,
                 const QString &imagePath);
    static const QString COL_QTY;
    static const QString COL_SKU;
    static const QString COL_IMAGE;
    void prepareOrder();
    QList<QStringList> getSkuNoImages(); // Parent and sku
    void createOrder(const QString &xlsxFilePathTo);

private:
    int _getColPosQty(const QXlsx::Document &document) const;
    int _getColPosSKU(const QXlsx::Document &document) const;
    int _getColPos(
        const QXlsx::Document &document, const QString &colName) const;
    int _getQuantity(const QXlsx::Document &document, int row, int col) const;
    QString _getSku(const QXlsx::Document &document, int row, int col) const;
    QString _getCjSkuParent(const QString &sku) const;
    struct CellInfo {
        QXlsx::Format format;
        QVariant       value;
    };
    struct ColInfo
    {
        QXlsx::Format format;
        double width;
    };

    typedef QSharedPointer<QHash<QString, CellInfo>> RowValues;
    QStringList _getDocColNames(const QXlsx::Document &document) const;
    QList<ColInfo> _getDocColInfos(QXlsx::Document &document) const;
    RowValues _getRowValues(
        QXlsx::Document &document,
        int row,
        const QStringList &colNames) const;
    bool _isYellow(const RowValues &rowValues) const;
    QStringList _getSkusInYellow(
        QXlsx::Document &document,
        const QStringList &colNames) const;

private:
    QStringList m_xlsxFilePathsFrom;
    //QMap<QString, int> m_skusReco_quantity;
    //QMap<QString, int> m_skusNoInv_customReco;
    QMap<QString, int> m_mergeSku_quantity;
    QDir m_dirImages;
    QHash<QString, RowValues> m_skuParent_rowValues;
    QHash<QString, RowValues> m_sku_rowValues;
    QMap<QString, QString> m_sku_imgFilePath;
    QStringList m_colNames;
    QList<ColInfo> m_colInfos;
    double m_rowHeight;
};

#endif // ORDERCREATOR_H
