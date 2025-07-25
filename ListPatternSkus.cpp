#include <QSettings>

#include "../common/workingdirectory/WorkingDirectoryManager.h"

#include "ListPatternSkus.h"

ListPatternSkus::ListPatternSkus(
        const QString &id, QObject *parent)
    : QAbstractListModel(parent)
{
    m_id = id;
}

void ListPatternSkus::addPattern(const QString &pattern)
{
    beginInsertRows(QModelIndex{}, 0, 0);
    m_patterns.insert(0, pattern);
    _saveInSettings();
    endInsertRows();
}

void ListPatternSkus::removePattern(const QModelIndex &index)
{
    if (index.isValid())
    {
        beginRemoveRows(QModelIndex{}, index.row(), index.row());
        m_patterns.removeAt(index.row());
        _saveInSettings();
        endRemoveRows();
    }
}

void ListPatternSkus::load()
{
    m_settingsKey = "ListPattern" + postFixSettings() + m_id;
    auto settings = WorkingDirectoryManager::instance()->settings();
    QStringList stringList = settings->value(m_settingsKey).toStringList();
    if (m_patterns.size() > 0)
    {
        beginRemoveRows(QModelIndex{}, 0, m_patterns.size()-1);
        m_patterns.clear();
        endRemoveRows();
    }
    beginInsertRows(QModelIndex{}, 0, stringList.size()-1);
    std::sort(stringList.begin(), stringList.end());
    m_patterns = std::move(stringList);
    endInsertRows();
}

const QStringList &ListPatternSkus::patterns() const
{
    return m_patterns;
}

void ListPatternSkus::_saveInSettings()
{
    Q_ASSERT(!m_settingsKey.isEmpty());
    auto settings = WorkingDirectoryManager::instance()->settings();
    if (m_patterns.size() > 0)
    {
        settings->setValue(m_settingsKey, m_patterns);
    }
    else if (settings->contains(m_settingsKey))
    {
        settings->remove(m_settingsKey);
    }
}

int ListPatternSkus::rowCount(const QModelIndex &) const
{
    return m_patterns.size();
}

QVariant ListPatternSkus::data(const QModelIndex &index, int role) const
{
    if (index.isValid())
    {
        if (role == Qt::DisplayRole || role == Qt::EditRole)
        {
            return m_patterns[index.row()];
        }
    }
    return QVariant();
}

bool ListPatternSkus::setData(
        const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::EditRole
            && data(index, role) != value)
    {
        m_patterns[index.row()] = value.toString();
        _saveInSettings();
        emit dataChanged(index, index);
        return true;
    }
    return false;
}

Qt::ItemFlags ListPatternSkus::flags(const QModelIndex &index) const
{
    return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QString ListPatternSkus::postFixSettings() const
{
    return "Skus";
}

