#include <QFileDialog>
#include <QMessageBox>
#include <QClipboard>
#include <QApplication>
#include <QSettings>

#include "../common/workingdirectory/WorkingDirectoryManager.h"

#include "DialogFilterOut.h"
#include "DialogMissingSkus.h"
#include "ListOrderModel.h"
#include "TableInventoryRecommendation.h"
#include "OrderCreator.h"

#include "MainWindow.h"
#include "./ui_MainWindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_settingKeyImagePath = "imagePath";
    const auto &countryCode = ui->comboCountry->currentText();
    ListOrderModel *listOrderModel = new ListOrderModel{countryCode, ui->listViewOrderFiles};
    ui->listViewOrderFiles->setModel(listOrderModel);
    ui->tableViewRecommended->setModel(TableInventoryRecommendation::instance());
    ui->tableViewRecommended->horizontalHeader()->resizeSection(TableInventoryRecommendation::IND_SKU, 200);
    ui->tableViewRecommended->horizontalHeader()->resizeSection(TableInventoryRecommendation::IND_TITLE, 400);
    auto settings = WorkingDirectoryManager::instance()->settings();
    ui->lineEditPathImage->setText(
        settings->value(m_settingKeyImagePath, QString{}).toString());
    _connectSlots();
}

void MainWindow::_connectSlots()
{
    connect(ui->comboCountry,
            &QComboBox::currentTextChanged,
            this,
            &MainWindow::_onCountryChanged);
    connect(ui->buttonBrowseImagePath,
            &QPushButton::clicked,
            this,
            &MainWindow::browseImagePath);
    connect(ui->buttonAddOrder,
            &QPushButton::clicked,
            this,
            &MainWindow::addOrderFile);
    connect(ui->buttonRemoveOrder,
            &QPushButton::clicked,
            this,
            &MainWindow::removeOrderFile);
    connect(ui->buttonImportReport,
            &QPushButton::clicked,
            this,
            &MainWindow::importInventoryRecommendation);
    connect(ui->buttonPasteRecommended,
            &QPushButton::clicked,
            this,
            &MainWindow::pasteInventoryRecommendation);
    connect(ui->buttonClearRecommended,
            &QPushButton::clicked,
            this,
            &MainWindow::clearInventoryRecommendation);
    connect(ui->buttonClearNotRecommended,
            &QPushButton::clicked,
            this,
            &MainWindow::clearInventoryNotRecommended);
    connect(ui->buttonClearFiltering,
            &QPushButton::clicked,
            this,
            &MainWindow::clearFiltering);
    connect(ui->buttonFilter,
            &QPushButton::clicked,
            this,
            &MainWindow::filter);
    connect(ui->buttonFilterReset,
            &QPushButton::clicked,
            this,
            &MainWindow::filterReset);
    connect(ui->buttonCreateOrder,
            &QPushButton::clicked,
            this,
            &MainWindow::createOrderFile);
    connect(ui->buttonSaveRecommentation,
            &QPushButton::clicked,
            this,
            &MainWindow::saveRecommendation);
    connect(ui->buttonLoadRecommentation,
            &QPushButton::clicked,
            this,
            &MainWindow::loadRecommendation);
}

MainWindow::~MainWindow()
{
    delete ui;
}

ListOrderModel *MainWindow::getListOrderModel() const
{
    return static_cast<ListOrderModel *>(ui->listViewOrderFiles->model());
}

void MainWindow::browseImagePath()
{
    auto settings = WorkingDirectoryManager::instance()->settings();
    QString lastDir{settings->value(m_settingKeyImagePath, QDir{}.path()).toString()};
    const QString &dirPath = QFileDialog::getExistingDirectory(
                this
                , tr("Order file")
                , lastDir
                //, nullptr
                //, QFileDialog::DontUseNativeDialog);
                );
    if (!dirPath.isEmpty())
    {
        settings->setValue(m_settingKeyImagePath, dirPath);
        ui->lineEditPathImage->setText(dirPath);
    }
}

void MainWindow::_onCountryChanged(const QString &countryCode)
{
    auto previousModel = ui->listViewOrderFiles->model();
    ListOrderModel *listOrderModel = new ListOrderModel{
            countryCode, ui->listViewOrderFiles};
    ui->listViewOrderFiles->setModel(listOrderModel);
    previousModel->deleteLater();
}

void MainWindow::addOrderFile()
{
    QSettings settings;
    const QString key{"MainWindow__addOrderFile"};
    QDir lastDir{settings.value(key, QDir{}.path()).toString()};
    const QString &filePath = QFileDialog::getOpenFileName(
                this
                , tr("Order file")
                , lastDir.path()
                , QString{"Xlsx (*.xlsx *.XLSX)"}
                //, nullptr
                //, QFileDialog::DontUseNativeDialog);
                );
    if (!filePath.isEmpty())
    {
        settings.setValue(key, QFileInfo{filePath}.dir().path());
        getListOrderModel()->addFile(filePath);
    }
}

void MainWindow::removeOrderFile()
{
    const auto &selIndexes = ui->listViewOrderFiles->selectionModel()->selectedIndexes();
    if (selIndexes.size() > 0)
    {
        getListOrderModel()->removeFile(selIndexes.first());
    }
}

void MainWindow::importInventoryRecommendation()
{
        QSettings settings;
        const QString key{"MainWindow__importInventoryRecommendation"};
        QDir lastDir{settings.value(key, QDir{}.path()).toString()};
        const QString &filePath = QFileDialog::getOpenFileName(
            this
            , tr("Inventory file")
            , lastDir.path()
            , QString{"CSV (*.csv *.CSV)"}
            //, nullptr
            //, QFileDialog::DontUseNativeDialog);
            );
        if (!filePath.isEmpty())
        {
            settings.setValue(key, QFileInfo{filePath}.dir().path());
            TableInventoryRecommendation::instance()->importCsvRecommendation(
                filePath);
        }
}

void MainWindow::pasteInventoryRecommendation()
{
    auto clipboard = QApplication::clipboard();
    const auto &text = clipboard->text();
    const auto &textTrimmed = text.trimmed();
    if (!textTrimmed.isEmpty())
    {
        int nAdded = TableInventoryRecommendation::instance()->pasteText(textTrimmed);
        if (nAdded > 0)
        {
            QMessageBox::information(
                this,
                tr("Rows added"),
                QString::number(nAdded) + tr(" rows we added"));
        }
        else
        {
            QMessageBox::information(
                this,
                tr("No rows added"),
                tr("No rows were added. Please check your copy paste"));
        }
    }
}

void MainWindow::clearInventoryRecommendation()
{
    TableInventoryRecommendation::instance()->clear();
}

void MainWindow::clearInventoryNotRecommended()
{
    TableInventoryRecommendation::instance()->clearNotRecommended();
}

void MainWindow::clearFiltering()
{
    DialogFilterOut dialog;
    dialog.exec();
    if (dialog.result() == QDialog::Accepted)
    {
        TableInventoryRecommendation::instance()->clear(
                    dialog.getPatternSkus()
                    , dialog.getPatternNames()
                    , dialog.isWhiteList()
                    );
    }
}

void MainWindow::createOrderFile()
{
    const auto &filePathsFrom = getListOrderModel()->getFilePaths();
    if (filePathsFrom.size() == 0)
    {
        QMessageBox::warning(
            this,
            tr("No file paths"),
            tr("You need to add previous order files"));
    }
    else
    {
        QSettings settings;
        const QString key{"MainWindow__createOrderFile"};
        QDir lastDir{settings.value(key, QDir{}.path()).toString()};
        QString filePath = QFileDialog::getSaveFileName(
            this
            , tr("Order file")
            , lastDir.path()
            , QString{"Xlsx (*.xlsx *.XLSX)"}
            //, nullptr
            //, QFileDialog::DontUseNativeDialog);
            );
        if (!filePath.isEmpty())
        {
            if (!filePath.endsWith(".xlsx", Qt::CaseInsensitive))
            {
                filePath += ".xlsx";
            }
            settings.setValue(key, QFileInfo{filePath}.dir().path());

            OrderCreator orderCreator{
                filePathsFrom
                , TableInventoryRecommendation::instance()->get_skusReco_quantity()
                , TableInventoryRecommendation::instance()->get_skusNoInv_customReco()
                , ui->lineEditPathImage->text()
            };
            orderCreator.prepareOrder();
            const auto &imageMissingSuks = orderCreator.getSkuNoImages();
            if (imageMissingSuks.size() > 0)
            {
                DialogMissingSkus dialog{imageMissingSuks};
                dialog.exec();
            }
            else
            {
                orderCreator.createOrder(filePath);
            }
        }
    }
}

void MainWindow::filter()
{
    const QString &textFilter = ui->lineEditFilter->text();
    int rowCount = TableInventoryRecommendation::instance()->rowCount();
    for (int i=0; i<rowCount; ++i)
    {
        const QString &sku = TableInventoryRecommendation::instance()->getSku(i);
        if (sku.contains(textFilter, Qt::CaseInsensitive))
        {
            ui->tableViewRecommended->setRowHidden(i, false);
            continue;
        }
        const QString &title = TableInventoryRecommendation::instance()->getTitle(i);
        if (title.contains(textFilter, Qt::CaseInsensitive))
        {
            ui->tableViewRecommended->setRowHidden(i, false);
            continue;
        }
        ui->tableViewRecommended->setRowHidden(i, true);
    }
}

void MainWindow::filterReset()
{
    int rowCount = TableInventoryRecommendation::instance()->rowCount();
    for (int i=0; i<rowCount; ++i)
    {
        ui->tableViewRecommended->setRowHidden(i, false);
    }
}

void MainWindow::saveRecommendation()
{
    TableInventoryRecommendation::instance()->save(
        ui->comboCountry->currentText());
}

void MainWindow::loadRecommendation()
{
    TableInventoryRecommendation::instance()->load(
        ui->comboCountry->currentText());
}


