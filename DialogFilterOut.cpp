#include "../common/workingdirectory/WorkingDirectoryManager.h"

#include <QSettings>
#include <QInputDialog>

#include "ListPatternTemplates.h"
#include "ListPatternNames.h"
#include "ListPatternSkus.h"

#include "DialogFilterOut.h"
#include "ui_DialogFilterOut.h"

DialogFilterOut::DialogFilterOut(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogFilterOut)
{
    ui->setupUi(this);
    ui->listViewTemplates->setModel(ListPatternTemplates::instance());
    _connectSlots();
    m_listPatternSkus = nullptr;
    m_listPatternNames = nullptr;
    m_settingsKeyWhiteList = "DialogFilterOutTemplateIdsWhiteList";
    _loadWhiteList();
}

void DialogFilterOut::_connectSlots()
{
    connect(ui->buttonAddTemplate,
            &QPushButton::clicked,
            this,
            &DialogFilterOut::addTemplate);
    connect(ui->buttonRemoveTemplate,
            &QPushButton::clicked,
            this,
            &DialogFilterOut::removeTemplateSelected);
    connect(ui->buttonAddSku,
            &QPushButton::clicked,
            this,
            &DialogFilterOut::addSkuPattern);
    connect(ui->buttonRemoveSku,
            &QPushButton::clicked,
            this,
            &DialogFilterOut::removeSkuPattern);
    connect(ui->buttonAddTitle,
            &QPushButton::clicked,
            this,
            &DialogFilterOut::addTitlePattern);
    connect(ui->buttonRemoveTitle,
            &QPushButton::clicked,
            this,
            &DialogFilterOut::removeTitlePattern);
    connect(ui->listViewTemplates->selectionModel(),
            &QItemSelectionModel::selectionChanged,
            this,
            &DialogFilterOut::onTemplateSelected);
    connect(ui->checkBoxIsWhiteList,
            &QCheckBox::clicked,
            this,
            [this](bool checked){
        const auto &selIndexes = ui->listViewTemplates->selectionModel()->selectedIndexes();
        if (selIndexes.size() > 0)
        {
            const QString &idTemplate = ListPatternTemplates::instance()->getId(
                        selIndexes.first());
            if (checked)
            {
                m_templateIdsWhiteList.insert(idTemplate);
                _saveWhiteList();
            }
            else
            {
                m_templateIdsWhiteList.remove(idTemplate);
                _saveWhiteList();
            }
        }
    });
}

void DialogFilterOut::_setPatternEditionEnabled(bool enable)
{
    ui->buttonAddSku->setEnabled(enable);
    ui->buttonAddTitle->setEnabled(enable);
    ui->buttonRemoveTitle->setEnabled(enable);
    ui->buttonRemoveSku->setEnabled(enable);
    ui->listViewSkus->setEnabled(enable);
    ui->listViewTitles->setEnabled(enable);
    ui->checkBoxIsWhiteList->setEnabled(enable);
}

void DialogFilterOut::_clearListPatterns()
{
    if (m_listPatternSkus != nullptr)
    {
        ui->listViewSkus->setModel(nullptr);
        ui->listViewTitles->setModel(nullptr);
        m_listPatternSkus->deleteLater();
        m_listPatternNames->deleteLater();
    }
    m_listPatternSkus = nullptr;
    m_listPatternNames = nullptr;
}

void DialogFilterOut::_saveWhiteList()
{
    auto settings = WorkingDirectoryManager::instance()->settings();
    settings->setValue(m_settingsKeyWhiteList, QVariant::fromValue(m_templateIdsWhiteList));
}

void DialogFilterOut::_loadWhiteList()
{
    auto settings = WorkingDirectoryManager::instance()->settings();
    m_templateIdsWhiteList = settings->value(m_settingsKeyWhiteList).value<QSet<QString>>();
}

DialogFilterOut::~DialogFilterOut()
{
    delete ui;
}

bool DialogFilterOut::isWhiteList() const
{
    const auto &selIndexes = ui->listViewTemplates->selectionModel()->selectedIndexes();
    if (selIndexes.size() > 0)
    {
        const QString &idTemplate = ListPatternTemplates::instance()->getId(
                    selIndexes.first());
        return m_templateIdsWhiteList.contains(idTemplate);
    }
    return false;
}

const QStringList &DialogFilterOut::getPatternNames() const
{
    if (m_listPatternNames != nullptr)
    {
        return m_listPatternNames->patterns();
    }
    static const QStringList empty;
    return empty;
}

const QStringList &DialogFilterOut::getPatternSkus() const
{
    if (m_listPatternSkus != nullptr)
    {
        return m_listPatternSkus->patterns();
    }
    static const QStringList empty;
    return empty;
}

void DialogFilterOut::addTemplate()
{
    const QString &name = QInputDialog::getText(
                this,
                tr("Template name"),
                tr("Enter the template name"));
    if (!name.isEmpty())
    {
        ListPatternTemplates::instance()->addTemplate(name);
        m_templateIdsWhiteList.insert(ListPatternTemplates::instance()->getLastId());
        _saveWhiteList();
    }
}

void DialogFilterOut::removeTemplateSelected()
{
    const auto &selIndexes = ui->listViewTemplates->selectionModel()->selectedIndexes();
    if (selIndexes.size() > 0)
    {
        const auto &firstIndex = selIndexes.first();
        const auto &templateId = ListPatternTemplates::instance()->getId(firstIndex);
        ListPatternTemplates::instance()->removeTemplate(selIndexes.first());
        m_templateIdsWhiteList.remove(templateId);
        _saveWhiteList();
    }
}

void DialogFilterOut::addTitlePattern()
{
    const QString &pattern = QInputDialog::getText(
                this,
                tr("Product title pattern"),
                tr("Enter the product title pattern"));
    if (!pattern.isEmpty())
    {
        m_listPatternNames->addPattern(pattern);
    }
}

void DialogFilterOut::removeTitlePattern()
{
    const auto &selIndexes = ui->listViewTitles->selectionModel()->selectedIndexes();
    if (selIndexes.size() > 0)
    {
        m_listPatternNames->removePattern(selIndexes.first());
    }
}

void DialogFilterOut::addSkuPattern()
{
    const QString &pattern = QInputDialog::getText(
                this,
                tr("Product SKU pattern"),
                tr("Enter the product SKU pattern"));
    if (!pattern.isEmpty())
    {
        m_listPatternSkus->addPattern(pattern);
    }
}

void DialogFilterOut::removeSkuPattern()
{
    const auto &selIndexes = ui->listViewSkus->selectionModel()->selectedIndexes();
    if (selIndexes.size() > 0)
    {
        m_listPatternSkus->removePattern(selIndexes.first());
    }
}

void DialogFilterOut::onTemplateSelected(
        const QItemSelection &selected, const QItemSelection &deselected)
{
    if (selected.size() > 0)
    {
        _setPatternEditionEnabled(true);
        _clearListPatterns();
        const QString &idTemplate = ListPatternTemplates::instance()->getId(
                    selected.first().indexes().first());
        m_listPatternSkus = new ListPatternSkus{idTemplate, ui->listViewSkus};
        m_listPatternSkus->load();
        ui->listViewSkus->setModel(m_listPatternSkus);
        m_listPatternNames = new ListPatternNames{idTemplate, ui->listViewTitles};
        m_listPatternNames->load();
        ui->listViewTitles->setModel(m_listPatternNames);
        ui->checkBoxIsWhiteList->setChecked(
                    m_templateIdsWhiteList.contains(idTemplate));
    }
    else if (deselected.size() > 0)
    {
        _clearListPatterns();
    }
}
