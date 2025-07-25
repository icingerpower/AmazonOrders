#include <QDateTime>
#include <QSettings>

#include "../common/workingdirectory/WorkingDirectoryManager.h"

#include "ListPatternTemplates.h"

ListPatternTemplates::ListPatternTemplates(QObject *parent)
    : QAbstractListModel(parent)
{
    m_settingsKey = "ListPatternTemplates";
    _loadFromSettings();
}

void ListPatternTemplates::_loadFromSettings()
{
    auto settings = WorkingDirectoryManager::instance()->settings();
    m_listOfStringList = settings->value(m_settingsKey).value<QList<QStringList>>();
}

void ListPatternTemplates::_saveInSettings()
{
    auto settings = WorkingDirectoryManager::instance()->settings();
    if (m_listOfStringList.size() > 0)
    {
        settings->setValue(m_settingsKey, QVariant::fromValue(m_listOfStringList));
    }
    else if (settings->contains(m_settingsKey))
    {
        settings->remove(m_settingsKey);
    }
}

ListPatternTemplates *ListPatternTemplates::instance()
{
    static ListPatternTemplates instance;
    return &instance;
}

void ListPatternTemplates::addTemplate(const QString &name)
{
    QString id = name + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss_zzz");
    beginInsertRows(QModelIndex{}, rowCount()-1, rowCount()-1);
    m_listOfStringList << QStringList{name, id};
    _saveInSettings();
    endInsertRows();
}

void ListPatternTemplates::removeTemplate(const QModelIndex &index)
{
    beginRemoveRows(QModelIndex{}, index.row(), index.row());
    m_listOfStringList.removeAt(index.row());
    _saveInSettings();
    endRemoveRows();
}

const QString &ListPatternTemplates::getId(int row) const
{
    return m_listOfStringList[row].last();
}

const QString &ListPatternTemplates::getId(const QModelIndex &index) const
{
    return getId(index.row());
}

const QString &ListPatternTemplates::getLastId() const
{
    return m_listOfStringList.last().last();
}

int ListPatternTemplates::rowCount(const QModelIndex &) const
{
    return m_listOfStringList.size();
}

QVariant ListPatternTemplates::data(const QModelIndex &index, int role) const
{
    if (index.isValid())
    {
        if (role == Qt::EditRole || role == Qt::DisplayRole)
        {
            return m_listOfStringList[index.row()][0];
        }
    }
    return QVariant();
}

bool ListPatternTemplates::setData(
        const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::EditRole
            && data(index, role) != value)
    {
        m_listOfStringList[index.row()][0] = value.toString();
        _saveInSettings();
        emit dataChanged(index, index);
        return true;
    }
    return false;
}

Qt::ItemFlags ListPatternTemplates::flags(const QModelIndex &index) const
{
    return Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled;
}

