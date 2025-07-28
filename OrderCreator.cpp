#include <QFile>

#include "OrderCreator.h"

const QString OrderCreator::COL_QTY{"quantity"};
const QString OrderCreator::COL_SKU{"sku"};
const QString OrderCreator::COL_IMAGE{"image"};

OrderCreator::OrderCreator(const QStringList &xlsxFilePathsFrom,
                           const QMap<QString, int> &skusReco_quantity,
                           const QMap<QString, int> &skusNoInv_customReco,
                           const QString &imagePath)
{
    m_dirImages = QDir{imagePath};
    m_xlsxFilePathsFrom = xlsxFilePathsFrom;
    //m_skusReco_quantity = skusReco_quantity;
    //m_skusNoInv_customReco = skusNoInv_customReco;
    m_mergeSku_quantity = skusReco_quantity;
    for (auto it = skusNoInv_customReco.begin();
         it != skusNoInv_customReco.end(); ++it)
    {
        if (!m_mergeSku_quantity.contains(it.key()))
        {
            m_mergeSku_quantity[it.key()] = it.value();
        }
        else
        {
            m_mergeSku_quantity[it.key()] = qMax(m_mergeSku_quantity[it.key()], it.value());
        }
    }

    m_rowHeight = 10.;
}

void OrderCreator::prepareOrder()
{
    Q_ASSERT(m_xlsxFilePathsFrom.size() > 0); // Can't be empty as the UI will check this first
    QStringList xlsxFilePathsFromSorted{m_xlsxFilePathsFrom.begin(), m_xlsxFilePathsFrom.end()};
    std::sort(xlsxFilePathsFromSorted.begin(),
              xlsxFilePathsFromSorted.end(),
              std::greater<QString>());
    QList<QSharedPointer<QXlsx::Document>> documentsFrom;
    m_skuParent_rowValues.clear();
    m_sku_rowValues.clear();
    m_sku_imgFilePath.clear();
    for (const auto &xlsxFilePathFrom : xlsxFilePathsFromSorted)
    {
        QSharedPointer<QXlsx::Document> documentFrom{
            new QXlsx::Document{xlsxFilePathFrom}};
        documentsFrom << documentFrom;
        int indColSku = _getColPosSKU(*documentFrom);
        auto dim = documentFrom->dimension();
        int lastRow = dim.lastRow();
        const auto &colNames = _getDocColNames(*documentFrom);
        for (int row = 1; row<lastRow; ++row)
        {
            const QString &sku = _getSku(*documentFrom, row, indColSku);
            if (!sku.isEmpty() && !m_sku_rowValues.contains(sku))
            {
                auto rowValues = _getRowValues(*documentFrom, row, colNames);
                m_sku_rowValues[sku] = rowValues;
                const QString &skuParent = _getCjSkuParent(sku);
                if (skuParent != sku && !m_skuParent_rowValues.contains(skuParent))
                {
                    m_skuParent_rowValues[sku] = rowValues;
                }
            }
        }
    }
    auto firstDocument = documentsFrom[0];
    m_colNames = _getDocColNames(*firstDocument);
    m_colInfos = _getDocColInfos(*firstDocument);
    const auto &skusInYellow = _getSkusInYellow(
        *firstDocument, m_colNames);
    for (const auto &yellowSku : skusInYellow)
    {
        if (!m_mergeSku_quantity.contains(yellowSku))
        {
            m_mergeSku_quantity[yellowSku] = 0;
        }
    }
    m_rowHeight = firstDocument->rowHeight(2);

    /* // TEST
    auto dim = firstDocument->dimension();
    int firstRow  = dim.firstRow();   // typically 1
    int lastRow   = dim.lastRow();
    int firstCol  = dim.firstColumn();
    int lastCol   = dim.lastColumn();

    // 2) Shift rows 4…lastRow up to 3…(lastRow–1)
    const int rowToRemove = 3;
    for (int r = rowToRemove + 1; r <= lastRow; ++r) {
        for (int c = firstCol; c <= lastCol; ++c) {
            // read value and format from the “source” cell
            QVariant     v   = firstDocument->read(r, c);
            auto cell = firstDocument->cellAt(r, c);
            QXlsx::Format fmt = cell ? cell->format() : QXlsx::Format();

            // write it one row up
            firstDocument->write(r - 1, c, v, fmt);
        }
    }

    // 3) Clear out the old last row (now duplicated)
    for (int c = firstCol; c <= lastCol; ++c) {
        firstDocument->write(lastRow, c, QVariant(), QXlsx::Format());
    }

    // …now you can re‑apply heights, save, etc.
    for (int i = 0; i < lastRow - 1; ++i) {
        firstDocument->setRowHeight(i + 1, rowHeight);
    }
    firstDocument->saveAs("/home/cedric/Dropbox/Divers/Commandes-produits/2025-07-25-1688-060-clothes-DE/2025-07-25__607010__stock-1688-061-clothes-DE__FTONGX__TODOEUR-BON-COMMANDE-V01-TEST.xlsx");
//*/


}

QList<QStringList> OrderCreator::getSkuNoImages()
{
    QList<QStringList> skuNoImages;
    QStringList extentions{"jpg", "jpeg", "png", "gif", "bmp"};
    QStringList nameFilter;
    for (const auto &ext : extentions)
    {
        nameFilter << "*." + ext;
        nameFilter << "*." + ext.toUpper();
    }
    const auto &imageFilePaths = m_dirImages.entryInfoList(
        nameFilter, QDir::Files, QDir::Name);
    m_sku_imgFilePath.clear();
    //QStringList skusWithImagesSorted;
    QMap<QString, QStringList> parent_skus;
    for (const auto &imageFilePath : imageFilePaths)
    {
        const auto &sku = imageFilePath.baseName();
        const auto &parentSku = _getCjSkuParent(sku);
        parent_skus[parentSku] << sku;
        //skusWithImagesSorted << sku;
        m_sku_imgFilePath[sku] = imageFilePath.absoluteFilePath();
    }
    for (auto it = m_mergeSku_quantity.begin();
         it != m_mergeSku_quantity.end(); ++it)
    {
        const auto &sku = it.key();
        const auto &parentSku = _getCjSkuParent(sku);
        if (!m_sku_imgFilePath.contains(sku)
             && !m_sku_imgFilePath.contains(parentSku))
        {
            if (parent_skus.contains(parentSku))
            {
                const auto &skuCandidtates = parent_skus[parentSku];
                int indexCandidate = 0;
                while (indexCandidate + 1 < skuCandidtates.size()
                       && sku > skuCandidtates[indexCandidate + 1])
                {
                    ++indexCandidate;
                }
                const auto &skuCandidtate = skuCandidtates[indexCandidate];
                m_sku_imgFilePath[sku] = m_sku_imgFilePath[skuCandidtate];
            }
            else
            {
                skuNoImages << QStringList{parentSku, sku};
            }
        }
        else if (!m_sku_imgFilePath.contains(sku))
        {
            m_sku_imgFilePath[sku] = m_sku_imgFilePath[parentSku];
        }
    }
    return skuNoImages;
}

void OrderCreator::createOrder(
    const QString &xlsxFilePathTo)
{
    if (QFile::exists(xlsxFilePathTo))
    {
        QFile::remove(xlsxFilePathTo);
    }
    QXlsx::Document docTo(xlsxFilePathTo);
    const auto &secondRow = m_sku_rowValues.begin().value();
    auto defautFormatSku = (*secondRow)[COL_SKU].format; // We assume files were correct with values
    //TODO write colNames in the first row
    int row = 1;
    for (int i = 0; i < m_colNames.size(); ++i) {
        docTo.write(1, i + 1, m_colNames.at(i));
        docTo.setColumnFormat(i + 1, m_colInfos[i].format);
        docTo.setColumnWidth(i + 1, m_colInfos[i].width);
    }
    int imageHeight = int(m_rowHeight*1.1 + 0.5);
    for (auto it = m_mergeSku_quantity.begin();
         it != m_mergeSku_quantity.end(); ++it)
    {
        RowValues rowValues;
        const auto &sku = it.key();
        if (m_sku_rowValues.contains(sku))
        {
            rowValues = m_sku_rowValues[sku];
            (*rowValues)[COL_QTY].value = it.value();
        }
        else
        {
            const auto &skuParent = _getCjSkuParent(sku);
            if (m_skuParent_rowValues.contains(skuParent))
            {
                rowValues = m_skuParent_rowValues[skuParent];
                (*rowValues)[COL_QTY].value = it.value();
            }
            else
            {
                rowValues = RowValues::create();
                CellInfo cellSku;
                cellSku.value = it.key();
                cellSku.format = defautFormatSku;
                CellInfo cellQty;
                cellQty.value = it.value();
                cellQty.format = defautFormatSku;
                rowValues->insert(COL_SKU, cellSku);
                rowValues->insert(COL_QTY, cellQty);
            }
        }
        for (int col = 0; col < m_colNames.size(); ++col)
        {
            const auto &colName = m_colNames[col];
            if (colName == COL_IMAGE)
            {
                Q_ASSERT(m_sku_imgFilePath.contains(sku));
                const auto &imageFilePath = m_sku_imgFilePath[sku];
                QImage image{imageFilePath};
                image = image.scaled(imageHeight, imageHeight);
                docTo.insertImage(row, col, image);
            }
            else if (rowValues->contains(colName))
            {
                const CellInfo &cellInfo = (*rowValues)[colName];
                docTo.write(row + 1, col + 1, cellInfo.value, cellInfo.format);
            }
        }
        docTo.setRowHeight(row + 1, m_rowHeight);
        ++row;
    }
    docTo.save();
}

int OrderCreator::_getColPosQty(
    const QXlsx::Document &document) const
{
    return _getColPos(document, COL_QTY);
}

int OrderCreator::_getColPosSKU(
    const QXlsx::Document &document) const
{
    return _getColPos(document, COL_SKU);
}

int OrderCreator::_getColPos(
    const QXlsx::Document &document, const QString &colName) const
{
    const QXlsx::CellRange dim = document.dimension();
    // If there's no data at all, bail out
    if (dim.firstRow() < 0)
    {
        return -1;
    }

    const int headerRow = dim.firstRow();
    const int firstCol  = dim.firstColumn();
    const int lastCol   = dim.lastColumn();

    // Scan across the header row looking for “SKU”
    for (int col = firstCol; col <= lastCol; ++col)
    {
        QVariant v = document.read(headerRow, col);
        if (!v.isNull())
        {
            QString txt = v.toString().trimmed();
            if (txt.compare(colName, Qt::CaseInsensitive) == 0)
            {
                return col - 1;
            }
        }
    }
    // Not found
    return -1;

}

int OrderCreator::_getQuantity(
    const QXlsx::Document &document, int row, int col) const
{
    if (auto cell = document.cellAt(row + 1, col + 1))
    {
        return cell->value().toInt();
    }
    return 0;
}

QString OrderCreator::_getSku(
    const QXlsx::Document &document, int row, int col) const
{
    if (auto cell = document.cellAt(row + 1, col + 1))
    {
        return cell->value().toString();
    }
    return 0;
}

QString OrderCreator::_getCjSkuParent(
    const QString &sku) const
{
    if (sku.contains("CJ"))
    {
        if (sku.contains("-"))
        {
            QStringList elements{sku.trimmed().split("-")};
            static QSet<QString> endDouble{"NARROW", "WIDE"};
            if (endDouble.contains(elements.last()))
            {
                elements.takeLast();
            }
            elements.takeLast();
            return elements.join("-");
        }
        else if (sku.size() == 15)
        {
            return sku.left(sku.size() - 4);
        }
    }
    return sku;
}

QStringList OrderCreator::_getDocColNames(
    const QXlsx::Document &document) const
{
    auto dim = document.dimension();
    QStringList colNames;
    if (dim.lastRow() > 0)
    {
        for (int col = 0; col <dim.lastColumn(); ++col)
        {
            if (auto cell = document.cellAt(1, col+1))
            {
                QString colName = cell->value().toString().trimmed().toLower();
                if (colName.isEmpty())
                {
                    return colNames;
                }
                colNames << colName;
            }
        }
    }
    return colNames;
}

QList<OrderCreator::ColInfo> OrderCreator::_getDocColInfos(
    QXlsx::Document &document) const
{
    QList<ColInfo> colInfos;
    auto dim = document.dimension();
    if (dim.lastRow() > 0)
    {
        for (int col = 0; col < dim.lastColumn(); ++col)
        {
            ColInfo colInfo;
            colInfo.width = document.columnWidth(col + 1);
            colInfo.format = document.columnFormat(col + 1);
            colInfos << colInfo;
        }
    }
    return colInfos;
}

OrderCreator::RowValues OrderCreator::_getRowValues(
    QXlsx::Document &document, int row, const QStringList &colNames) const
{
    RowValues rowValues = RowValues::create();
    for (int col = 0; col < colNames.size(); ++col)
    {
        /*
        QImage imgTEMP;
        if (document.getImage(row, col, imgTEMP))
        {
            int TEMP=10;++TEMP;
        }
        //*/
        const QString &colName = colNames[col];
        if (!colName.isEmpty())
        {
            CellInfo cellInfo;
            if (auto cell = document.cellAt(row + 1, col + 1)) {
                cellInfo.value  = cell->readValue();   // QVariant
                cellInfo.format = cell->format();      // Format of that cell :contentReference[oaicite:0]{index=0}
                if (!cellInfo.value.toString().contains(".htm"))
                {
                    cellInfo.format.setFontColor(Qt::black);
                }
            }
            (*rowValues)[colName] = std::move(cellInfo);
        }
    }
    return rowValues;
}

bool OrderCreator::_isYellow(
    const RowValues &rowValues) const
{
    if (rowValues->contains(COL_QTY))
    {
        return (*rowValues)[COL_QTY].format.patternBackgroundColor() == Qt::yellow;
    }
    return false;
}

QStringList OrderCreator::_getSkusInYellow(
    QXlsx::Document &document, const QStringList &colNames) const
{
    QStringList skus;
    auto dim = document.dimension();
    for (int row = 1; row < dim.lastRow(); ++row)
    {
        auto rowValues = _getRowValues(document, row, colNames);
        if (_isYellow(rowValues) && rowValues->contains(COL_SKU))
        {
            const QString &sku = rowValues->value(COL_SKU).value.toString();
            skus << sku;
        }
    }
    return skus;
}

