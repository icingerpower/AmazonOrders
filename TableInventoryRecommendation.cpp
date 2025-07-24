#include <QRegularExpression>

#include "TableInventoryRecommendation.h"

const QStringList TableInventoryRecommendation::HEADER{"SKU", "Unit sold 30d", "Unit supply", "day supply", "Recommended Qty"};
const int TableInventoryRecommendation::IND_SKU{0};
const int TableInventoryRecommendation::IND_UNIT_SOLD_30DAYS{1};
const int TableInventoryRecommendation::IND_UNIT_SUPPLY{2};
const int TableInventoryRecommendation::IND_DAY_SUPPLY{3};
const int TableInventoryRecommendation::IND_RECOMMENDED_QTY{4};

TableInventoryRecommendation::TableInventoryRecommendation(QObject *parent)
    : QAbstractTableModel(parent)
{
}

TableInventoryRecommendation *TableInventoryRecommendation::instance()
{
    static TableInventoryRecommendation instance;
    return &instance;
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

QVariant TableInventoryRecommendation::data(
        const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
        return m_listOfVariantList[index.row()][index.column()];
    }
    return QVariant();
}

void TableInventoryRecommendation::pasteText(const QString &text)
{
    static const QRegularExpression reASIN("^B[0-9A-Z]{9,}$");
    static const QRegularExpression reFNSKU("^X[0-9A-Z]+$");
    static const QRegularExpression reUnits(R"(^(\\d+)\\s+units$)");
    static const QRegularExpression reSKU("^[A-Za-z0-9\\-]+$");
    int rowBefore = rowCount();

    // Split the whole paste by the trailing “Today” marker of each block
    static const QRegularExpression reToday("\\bToday\\b");
    const QStringList &blocks = text.split(reToday,
                                          Qt::SkipEmptyParts);

    for (const QString &blk : blocks)
    {
        // Normalize / clean lines
        QStringList lines;
        lines.reserve(64);
        for (QString l : blk.split('\n'))
        {
            l = l.trimmed();
            if (l.isEmpty())
                continue;
            if (l == "Action")
                continue;
            lines << l;
        }
        if (lines.isEmpty())
            continue;

        // Find key indices
        int idxASIN  = -1;
        int idxFNSKU = -1;
        int idxSKU   = -1;
        int idxDash  = -1; // the "--" separator

        for (int i = 0; i < lines.size(); ++i)
        {
            if (idxASIN  == -1 && reASIN.match(lines[i]).hasMatch())  idxASIN  = i;
            if (idxFNSKU == -1 && reFNSKU.match(lines[i]).hasMatch()) idxFNSKU = i;
            if (idxDash  == -1 && lines[i] == "--")                   idxDash  = i;
        }

        // Heuristic: first “token-looking” (letters/digits/-) line before ASIN/FNSKU
        for (int i = 0; i < lines.size(); ++i)
        {
            if ((idxASIN == -1  || i < idxASIN) &&
                    (idxFNSKU == -1 || i < idxFNSKU))
            {
                if (reSKU.match(lines[i]).hasMatch() &&
                        !reASIN.match(lines[i]).hasMatch() &&
                        !reFNSKU.match(lines[i]).hasMatch())
                {
                    idxSKU = i;
                    break;
                }
            }
        }

        if (idxSKU == -1 || idxDash == -1)
            continue; // can't reliably parse this block

        int unitsSold30   = 0;
        int daySupply     = 0;
        int unitSupply    = 0;
        int recommendedQty= 0;

        // From the separator on, follow the numeric pattern
        int i = idxDash + 1;

        // 1) first "<n> units"  => units sold last 30 days
        for (; i < lines.size(); ++i)
        {
            auto m = reUnits.match(lines[i]);
            if (m.hasMatch())
            {
                unitsSold30 = m.captured(1).toInt();
                ++i;
                break;
            }
        }

        // 2) next pure integer (skip €, m3, “Out of stock”, and "<n> units") => day supply
        for (; i < lines.size(); ++i)
        {
            if (lines[i].contains("€") || lines[i].contains("m3", Qt::CaseInsensitive))
                continue;
            if (reUnits.match(lines[i]).hasMatch())
                continue;
            if (lines[i].compare("Out of stock", Qt::CaseInsensitive) == 0)
                continue;

            bool ok = false;
            int v = lines[i].toInt(&ok);
            if (ok)
            {
                daySupply = v;
                ++i;
                break;
            }
        }

        // 3) next "<n> units"   => unit supply
        for (; i < lines.size(); ++i)
        {
            auto m = reUnits.match(lines[i]);
            if (m.hasMatch())
            {
                unitSupply = m.captured(1).toInt();
                ++i;
                break;
            }
        }

        // Optional “Out of stock”
        if (i < lines.size() && lines[i].contains("Out of stock", Qt::CaseInsensitive))
            ++i;

        // 4) next plain integer => recommended qty
        for (; i < lines.size(); ++i)
        {
            bool ok = false;
            int v = lines[i].toInt(&ok);
            if (ok)
            {
                recommendedQty = v;
                break;
            }
        }

        QVariantList row;
        row << lines[idxSKU]
               << unitsSold30
               << unitSupply
               << daySupply
               << recommendedQty;

        m_listOfVariantList << row;
    }
    if (m_listOfVariantList.size() > rowBefore)
    {
        beginInsertRows(QModelIndex{}, rowBefore, m_listOfVariantList.size()-1);
        endInsertRows();
    }
}

void TableInventoryRecommendation::clear()
{
    beginRemoveRows(QModelIndex{}, 0, rowCount() - 1);
    m_listOfVariantList.clear();
    endRemoveRows();
}
