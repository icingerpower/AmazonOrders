#include <QSettings>
#include <QFile>

#include "../common/workingdirectory/WorkingDirectoryManager.h"

#include "ListOrderModel.h"

ListOrderModel::ListOrderModel(const QString &countryCode, QObject *parent)
{
    m_settingsKey = "ListOrderModel";
    m_settingsKey += countryCode;
    _loadFromSettings();
}

const QStringList &ListOrderModel::getFilePaths() const
{
    return m_filePaths;
}

void ListOrderModel::_loadFromSettings()
{
    auto settings = WorkingDirectoryManager::instance()->settings();
    if (settings->contains(m_settingsKey))
    {
        m_filePaths = settings->value(m_settingsKey).toStringList();
        for (int i=m_filePaths.size()-1; i>-1; --i)
        {
            if (!QFile::exists(m_filePaths[i]))
            {
                m_filePaths.removeAt(i);
            }
        }
        std::sort(m_filePaths.begin(), m_filePaths.end(), std::greater<QString>());
    }
}

void ListOrderModel::_saveInSettings()
{
    auto settings = WorkingDirectoryManager::instance()->settings();
    if (m_filePaths.size() > 0)
    {
        settings->setValue(m_settingsKey, m_filePaths);
    }
    else if (settings->contains(m_settingsKey))
    {
        settings->remove(m_settingsKey);
    }
}

void ListOrderModel::addFile(const QString &filePath)
{
    beginInsertRows(QModelIndex{}, 0, 0);
    m_filePaths.insert(0, filePath);
    _saveInSettings();
    endInsertRows();
}

void ListOrderModel::removeFile(const QModelIndex &index)
{
    beginRemoveRows(QModelIndex{}, index.row(), index.row());
    m_filePaths.removeAt(index.row());
    _saveInSettings();
    endRemoveRows();
}

int ListOrderModel::rowCount(const QModelIndex &) const
{
    return m_filePaths.size();
}

QVariant ListOrderModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::EditRole | role == Qt::DisplayRole)
    {
        return m_filePaths[index.row()];
    }
    return QVariant{};
}

Qt::ItemFlags ListOrderModel::flags(const QModelIndex &index) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}
