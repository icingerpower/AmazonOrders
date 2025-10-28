#include <QRegularExpression>
#include <QSharedPointer>

#include "../common/workingdirectory/WorkingDirectoryManager.h"
#include "../common/utils/CsvReader.h"

#include "TableInventoryRecommendation.h"

const QStringList TableInventoryRecommendation::HEADER{
    "SKU"
    , "Title"
    , "Unit sold 30d"
    , "Unit supply"
    , "day supply"
    , "Recommended Qty"
    , "Price"
    , "FNSKU"
    , "ASIN"
};
const int TableInventoryRecommendation::IND_SKU{0};
const int TableInventoryRecommendation::IND_TITLE{1};
const int TableInventoryRecommendation::IND_UNIT_SOLD_30DAYS{2};
const int TableInventoryRecommendation::IND_UNIT_SUPPLY{3};
const int TableInventoryRecommendation::IND_DAY_SUPPLY{4};
const int TableInventoryRecommendation::IND_RECOMMENDED_QTY{5};
const int TableInventoryRecommendation::IND_PRICE{6};
const int TableInventoryRecommendation::IND_FNSKU{7};
const int TableInventoryRecommendation::IND_ASIN{8};

TableInventoryRecommendation::TableInventoryRecommendation(QObject *parent)
    : QAbstractTableModel(parent)
{
    m_settingsKey = "TableInventoryRecommendation";
}

bool TableInventoryRecommendation::_lineMatch(
        int indexLine, const QStringList &patternSkus, const QStringList &patternTitles, bool isWhiteList) const
{
    if (patternSkus.size() == 0 && patternTitles.size() == 0)
    {
        return true;
    }
    if (patternSkus.size() > 0)
    {
        const QString &sku = m_listOfVariantList[indexLine][IND_SKU].toString();
        for (const auto &patternSku : patternSkus)
        {
            if (sku.contains(patternSku, Qt::CaseInsensitive))
            {
                return isWhiteList;
            }
        }
    }
    if (patternTitles.size() > 0)
    {
        const QString &title = m_listOfVariantList[indexLine][IND_TITLE].toString();
        for (const auto &patternTitle : patternTitles)
        {
            if (title.contains(patternTitle, Qt::CaseInsensitive))
            {
                return isWhiteList;
            }
        }
    }
    return !isWhiteList;
}

TableInventoryRecommendation *TableInventoryRecommendation::instance()
{
    static TableInventoryRecommendation instance;
    return &instance;
}

QString TableInventoryRecommendation::getSku(int row) const
{
    return m_listOfVariantList[row][IND_SKU].toString();
}

QString TableInventoryRecommendation::getTitle(int row) const
{
    return m_listOfVariantList[row][IND_TITLE].toString();
}

QMap<QString, int> TableInventoryRecommendation::get_skusNoInv_customReco() const
{
    QMap<QString, int> skusNoInv_quantitySold;
    for (const auto &variantList : m_listOfVariantList)
    {
        int quantityReco = variantList[IND_RECOMMENDED_QTY].toInt();
        if (quantityReco == 0)
        {
            int quantityLeft = variantList[IND_UNIT_SUPPLY].toInt();
            int quantityLeftDays = variantList[IND_DAY_SUPPLY].toInt();
            int quantitySold30days = variantList[IND_UNIT_SOLD_30DAYS].toInt();
            if (quantityLeft == 0 || quantityLeftDays < 45)
            {
                const auto &sku = variantList[IND_SKU].toString();
                skusNoInv_quantitySold[sku] = qMax(quantitySold30days, 1);
            }
        }
    }
    return skusNoInv_quantitySold;
}

QMap<QString, int> TableInventoryRecommendation::get_skusReco_quantity() const
{
    QMap<QString, int> skusReco_quantity;
    for (const auto &variantList : m_listOfVariantList)
    {
        int quantity = variantList[IND_RECOMMENDED_QTY].toInt();
        if (quantity > 0)
        {
            const auto &sku = variantList[IND_SKU].toString();
            skusReco_quantity[sku] = quantity;
        }
    }
    return skusReco_quantity;
}

QVariant TableInventoryRecommendation::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Horizontal)
        {
            return HEADER[section];
        }
        else
        {
            return QString::number(section + 1);
        }
    }
    return QVariant{};
}

int TableInventoryRecommendation::rowCount(const QModelIndex &parent) const
{
    return m_listOfVariantList.size();
}

int TableInventoryRecommendation::columnCount(const QModelIndex &) const
{
    return HEADER.size();
}

void TableInventoryRecommendation::sort(int column, Qt::SortOrder order)
{
    if (order == Qt::DescendingOrder)
    {
        std::sort(m_listOfVariantList.begin(), m_listOfVariantList.end(), [column](
                  const QVariantList &variantList1, const QVariantList &variantList2){
            static const auto typeString = QVariant{QString{}}.typeId();
            static const auto typeInt = QVariant{0}.typeId();
            static const auto typeDouble = QVariant{9.99}.typeId();
            if (variantList1[column].typeId() == typeString)
            {
                return variantList1[column].toString() < variantList2[column].toString();
            }
            else if (variantList1[column].typeId() == typeInt)
            {
                return variantList1[column].toInt() < variantList2[column].toInt();
            }
            else if (variantList1[column].typeId() == typeDouble)
            {
                return variantList1[column].toDouble() < variantList2[column].toDouble();
            }
            return false;
        });
    }
    else
    {
         std::sort(m_listOfVariantList.begin(), m_listOfVariantList.end(), [column](
                  const QVariantList &variantList1, const QVariantList &variantList2){
            static const auto typeString = QVariant{QString{}}.typeId();
            static const auto typeInt = QVariant{0}.typeId();
            static const auto typeDouble = QVariant{9.99}.typeId();
            if (variantList1[column].typeId() == typeString)
            {
                return variantList1[column].toString() > variantList2[column].toString();
            }
            else if (variantList1[column].typeId() == typeInt)
            {
                return variantList1[column].toInt() > variantList2[column].toInt();
            }
            else if (variantList1[column].typeId() == typeDouble)
            {
                return variantList1[column].toDouble() > variantList2[column].toDouble();
            }
            return false;
        });
    }
    emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1));
}

QVariant TableInventoryRecommendation::data(
        const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
        return m_listOfVariantList[index.row()][index.column()];
    }
    return QVariant();
}

void TableInventoryRecommendation::clear(
        const QStringList &patternSkus, const QStringList &patternTitles, bool isWhiteList)
{
    for (int i = m_listOfVariantList.size()-1; i>-1; --i)
    {
        if (!_lineMatch(i, patternSkus, patternTitles, isWhiteList))
        {
            beginRemoveRows(QModelIndex{}, i, i);
            qDebug() << "Romeving :" << m_listOfVariantList[i][IND_SKU].toString() << m_listOfVariantList[i][IND_TITLE].toString();
            m_listOfVariantList.removeAt(i);
            endRemoveRows();
        }
    }
}

int TableInventoryRecommendation::pasteText(const QString &text)
{
    int nAddedLines = 0;
    int rowBefore = rowCount();
    QStringList lines;
    int indexAction = text.indexOf("Action");
    if (indexAction > -1)
    {
        lines = text.right(text.size() - indexAction - 6).split("\n");
    }
    else
    {
        lines = text.split("\n");
    }
    for (int i=lines.size()-1; i>-1; --i)
    {
        lines[i] = lines[i].trimmed();
        if (lines[i].isEmpty())
        {
            lines.removeAt(i);
        }
    }
    QString supplierMark{"--"};
    int indexDashDash = lines.indexOf(supplierMark);
    if (indexDashDash == -1 || !lines[indexDashDash-1].startsWith("X"))
    {
        supplierMark = "unassigned";
        indexDashDash = lines.indexOf(supplierMark);
    }
    while (indexDashDash != -1)
    {
        int indexSku = indexDashDash - 4;
        int indexTitle = indexDashDash - 3;
        int indexASIN = indexDashDash - 2;
        int indexFNSKU = indexDashDash - 1;
        int indexSoldLast30days = indexDashDash + 2;
        int addExtraViewForecast = lines.contains("View forecast", Qt::CaseInsensitive) ? 1:0;
        int indexSalePrice = indexDashDash + 3 + addExtraViewForecast;
        int indexInvLeftDays = indexDashDash + 4 + addExtraViewForecast;
        int indexInvLeft = indexDashDash + 5 + addExtraViewForecast;
        int indexInvRecommendation = indexDashDash + 6 + addExtraViewForecast;
        if (lines[indexInvRecommendation].contains("stock", Qt::CaseInsensitive))
        {
            ++indexInvRecommendation;
        }
        if (indexSku >= 0 && indexInvLeft < lines.size())
        {
            const QString &sku = lines[indexSku];
            const QString &title = lines[indexTitle];
            const QString &ASIN = lines[indexASIN];
            const QString &FNSKU = lines[indexFNSKU];
            QString soldLast30days = lines[indexSoldLast30days].split(" ")[0].replace(",", "");
            QString salePrice = lines[indexSalePrice];
            while (!salePrice.isEmpty() && !salePrice.at(0).isDigit())
            {
                salePrice.remove(0, 1);
            }
            const QString &invLeftDays = lines[indexInvLeftDays].replace("--", "0").replace("+", "");
            QString invLeft = lines[indexInvLeft].split(" ")[0];
            QString invRecommendation = lines[indexInvRecommendation];
            if (invRecommendation.startsWith("No", Qt::CaseInsensitive))
            {
                invRecommendation = "0";
            }
            bool okSoldLast30days{false};
            bool okInvRecommendation{false};
            bool okSalePrice{false};
            bool okInvLeftDays{false};
            bool okInvLeft{false};
            m_listOfVariantList << QVariantList{
                                   sku
                                   , title
                                   , soldLast30days.toInt(&okSoldLast30days)
                                   , invLeft.toInt(&okInvLeft)
                                   , invLeftDays.toInt(&okInvLeftDays)
                                   , invRecommendation.toInt(&okInvRecommendation)
                                   , salePrice.toDouble(&okSalePrice)
                                   , FNSKU
                                   , ASIN
                                    };
            ++nAddedLines;
            Q_ASSERT(okSoldLast30days);
            Q_ASSERT(okInvRecommendation);
            Q_ASSERT(okSalePrice);
            Q_ASSERT(okInvLeftDays);
            Q_ASSERT(okInvLeft);
        }
        else
        {
            Q_ASSERT(false);
            // TODO raise Exception
        }
        lines.remove(indexDashDash);
        indexDashDash = lines.indexOf(supplierMark);
    }
    if (m_listOfVariantList.size() > rowBefore)
    {
        beginInsertRows(QModelIndex{}, rowBefore, m_listOfVariantList.size()-1);
        endInsertRows();
    }
    return nAddedLines;
}

void TableInventoryRecommendation::clear()
{
    if (m_listOfVariantList.size() > 0)
    {
        beginRemoveRows(QModelIndex{}, 0, rowCount() - 1);
        m_listOfVariantList.clear();
        endRemoveRows();
    }
}

void TableInventoryRecommendation::clearNotRecommended()
{
    for (int i=m_listOfVariantList.size()-1; i>-1; --i)
    {
        if (m_listOfVariantList[i][IND_RECOMMENDED_QTY].toInt() == 0)
        {
            beginRemoveRows(QModelIndex{}, i, i);
            m_listOfVariantList.removeAt(i);
            endRemoveRows();
        }
    }
}

void TableInventoryRecommendation::save(const QString &countryCode)
{
    auto settings = WorkingDirectoryManager::instance()->settings();
    if (m_listOfVariantList.size() > 0)
    { // We only save if values, to avoid overwritting existing saved values by accident
        settings->setValue(m_settingsKey + countryCode
                           , QVariant::fromValue(m_listOfVariantList));
    }
}

void TableInventoryRecommendation::load(const QString &countryCode)
{
    clear();
    auto settings = WorkingDirectoryManager::instance()->settings();
    const auto &settingsKeyCountryCode = m_settingsKey + countryCode;
    if (settings->contains(settingsKeyCountryCode))
    {
        auto listOfVariantList
            = settings->value(settingsKeyCountryCode).value<QList<QVariantList>>();
        if (listOfVariantList.size() > 0)
        {
            beginInsertRows(QModelIndex{}, 0, listOfVariantList.size() - 1);
            m_listOfVariantList = std::move(listOfVariantList);
            endInsertRows();
        }
    }
}

Qt::ItemFlags TableInventoryRecommendation::flags(
    const QModelIndex &) const
{
    return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

void TableInventoryRecommendation::importCsvRecommendation(
    const QString &filePath)
{
    if (m_listOfVariantList.size() > 0)
    {
        beginRemoveRows(QModelIndex{}, 0, m_listOfVariantList.size()-1);
        m_listOfVariantList.clear();
        endRemoveRows();
    }
    CsvReader csvReader{filePath, ",", "\""};
    if (csvReader.readAll())
    {
        auto dataRode = csvReader.dataRode();
        int posMerchantSKU = dataRode->header.pos("Merchant SKU");
        int posProductName = dataRode->header.pos("Product Name");
        int posMerchantFNSKU = dataRode->header.pos("FNSKU");
        int posProductASIN = dataRode->header.pos("ASIN");
        int posSoldLast30days = dataRode->header.pos("Units Sold Last 30 Days");
        int posUnitsTotal = dataRode->header.pos("Total Units");
        //int posUnitsInbound = dataRode->header.pos("Inbound");
        int posUnitsAvailable = dataRode->header.pos("Available");
        int posTotalDays = dataRode->header.pos("Total Days of Supply (including units from open shipments)");
        int posRecoUnits = dataRode->header.pos("Recommended replenishment qty");
        int posRecoShipDate = dataRode->header.pos("Recommended ship date");
        int posPrice = dataRode->header.pos("Price");
        int nRows = 0;
        QList<QVariantList> listOfVariantList;
        for (const auto &elements : dataRode->lines)
        {
            const QString &sku = elements[posMerchantSKU];
            const QString &title = elements[posProductName];
            const QString &FNSKU = elements[posMerchantFNSKU];
            const QString &ASIN = elements[posProductASIN];
            int soldLast30days = elements[posSoldLast30days].toInt();
            int unitsTotal = elements[posUnitsTotal].toInt();
            //int unitsInbound = elements[posUnitsInbound].toInt();
            int unitsAvailable = elements[posUnitsAvailable].toInt();
            int totalDays = elements[posTotalDays].toInt();
            int recoUnits = elements[posRecoUnits].toInt();
            const auto &recoShipDate = QDate::fromString(
                elements[posRecoShipDate], "MM/dd/yyyy");
            double price = elements[posPrice].toDouble();
            int correctedReco = recoUnits;
            int daysToShip = QDate::currentDate().daysTo(recoShipDate);
            if (correctedReco > 1)
            {
                if (unitsAvailable == 0)
                {
                    correctedReco *= 1.3 + 1;
                }
                else if (daysToShip > 120)
                {
                    correctedReco *= 0.8;
                }
            }
            if (correctedReco == 0)
            {
                if (unitsTotal == 0 && soldLast30days > 0)
                {
                    correctedReco = 2;
                }
            }
            if (correctedReco > 0)
            {
                listOfVariantList << QVariantList{
                    sku
                    , title
                    , soldLast30days
                    , unitsAvailable
                    , totalDays
                    , correctedReco
                    , price
                    , FNSKU
                    , ASIN
                };
                ++nRows;
            }
        }
        beginInsertRows(QModelIndex{}, 0, listOfVariantList.size() - 1);
        m_listOfVariantList = std::move(listOfVariantList);
        endInsertRows();
    }

    QFile file{filePath};
    if (file.open(QFile::ReadOnly))
    {
        QTextStream stream{&file};
        file.close();
    }
}

