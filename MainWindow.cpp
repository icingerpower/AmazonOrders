#include <QFileDialog>
#include <QMessageBox>
#include <QClipboard>
#include <QApplication>
#include <QSettings>
#include <QFile>
#include <algorithm>

#include <xlsxdocument.h>

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
    m_settingKeySourcingCostFile = "sourcingCostFile";
    m_settingKeyPricePerKilo = "pricePerKilo";
    const auto &countryCode = ui->comboCountry->currentText();
    ListOrderModel *listOrderModel = new ListOrderModel{countryCode, ui->listViewOrderFiles};
    ui->listViewOrderFiles->setModel(listOrderModel);
    ui->tableViewRecommended->setModel(TableInventoryRecommendation::instance());
    ui->tableViewRecommended->horizontalHeader()->resizeSection(TableInventoryRecommendation::IND_SKU, 200);
    ui->tableViewRecommended->horizontalHeader()->resizeSection(TableInventoryRecommendation::IND_TITLE, 400);
    auto settings = WorkingDirectoryManager::instance()->settings();
    ui->lineEditPathImage->setText(
        settings->value(m_settingKeyImagePath, QString{}).toString());
    ui->lineEditSourcingCostFile->setText(
        settings->value(m_settingKeySourcingCostFile, QString{}).toString());
    ui->spinBoxPricePerKilo->setValue(
        settings->value(m_settingKeyPricePerKilo, 5.0).toDouble());
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
    connect(ui->buttonBrowseSourcingCost,
            &QPushButton::clicked,
            this,
            &MainWindow::browseSourcingCostFile);
    connect(ui->buttonGenSourcingCost,
            &QPushButton::clicked,
            this,
            &MainWindow::genSourcingCost);
    connect(ui->spinBoxPricePerKilo,
            &QDoubleSpinBox::valueChanged,
            this,
            [this](double value) {
                WorkingDirectoryManager::instance()->settings()->setValue(
                    m_settingKeyPricePerKilo, value);
            });
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

void MainWindow::browseSourcingCostFile()
{
    auto settings = WorkingDirectoryManager::instance()->settings();
    const QString &currentPath = settings->value(m_settingKeySourcingCostFile, QString{}).toString();
    const QString &startDir = currentPath.isEmpty()
        ? QDir{}.path()
        : QFileInfo{currentPath}.dir().path();
    const QString &filePath = QFileDialog::getOpenFileName(
        this,
        tr("Sourcing cost file"),
        startDir,
        QString{"Xlsx (*.xlsx *.XLSX)"}
    );
    if (!filePath.isEmpty())
    {
        settings->setValue(m_settingKeySourcingCostFile, filePath);
        ui->lineEditSourcingCostFile->setText(filePath);
    }
}

void MainWindow::genSourcingCost()
{
    const QString &sourcePath = ui->lineEditSourcingCostFile->text();
    if (sourcePath.isEmpty())
    {
        QMessageBox::warning(this, tr("No file"), tr("Please select a sourcing cost file first."));
        return;
    }
    QFileInfo fi{sourcePath};
    const QString outputPath = fi.dir().filePath(fi.baseName() + "-FILLED." + fi.suffix());
    if (QFile::exists(outputPath))
        QFile::remove(outputPath);
    if (!QFile::copy(sourcePath, outputPath))
    {
        QMessageBox::warning(this, tr("Copy failed"), tr("Could not create output file:\n") + outputPath);
        return;
    }
    _fillSourcingCost(outputPath);
}

void MainWindow::_fillSourcingCost(const QString &filePath)
{
    // Helper: find 1-based column index matching header name (exact, case-insensitive)
    auto findCol = [](const QXlsx::Document &doc, const QString &name) -> int {
        auto dim = doc.dimension();
        for (int col = dim.firstColumn(); col <= dim.lastColumn(); ++col)
        {
            QVariant v = doc.read(dim.firstRow(), col);
            if (!v.isNull() && v.toString().trimmed().toLower() == name)
                return col;
        }
        return -1;
    };

    // --- Step 1: scan order files most-recent to oldest, collect price + weight per SKU ---
    struct SkuInfo { double price; double weightGrams; QString sourceFile; };
    QHash<QString, SkuInfo> skuInfo;
    QHash<QString, QString> fnsku_sku; // FNSKU -> SKU for fallback matching in sourcing file

    QStringList orderPaths = getListOrderModel()->getFilePaths();
    std::sort(orderPaths.begin(), orderPaths.end(), std::greater<QString>());

    for (const QString &orderPath : orderPaths)
    {
        QXlsx::Document orderDoc{orderPath};
        auto dim = orderDoc.dimension();
        if (dim.firstRow() < 0) continue;

        int cSku    = findCol(orderDoc, "sku");
        int cFnsku  = findCol(orderDoc, "fnsku");
        int cWeight = findCol(orderDoc, "unit weight");
        int cPrice  = findCol(orderDoc, "unit price");
        if (cSku < 0 || cPrice < 0 || cWeight < 0) continue;

        const QString orderFileName = QFileInfo(orderPath).fileName();

        for (int row = dim.firstRow() + 1; row <= dim.lastRow(); ++row)
        {
            const QString sku = orderDoc.read(row, cSku).toString().trimmed();
            if (sku.isEmpty() || skuInfo.contains(sku)) continue; // most-recent wins

            bool priceOk, weightOk;
            double price  = orderDoc.read(row, cPrice).toDouble(&priceOk);
            double weight = orderDoc.read(row, cWeight).toDouble(&weightOk);
            if (!priceOk || price <= 0 || !weightOk || weight < 0) continue;

            skuInfo[sku] = {price, weight, orderFileName};
            if (cFnsku >= 0)
            {
                const QString fnsku = orderDoc.read(row, cFnsku).toString().trimmed();
                if (!fnsku.isEmpty())
                    fnsku_sku[fnsku] = sku;
            }
        }
    }

    // --- Step 2: fill "Seller New Cost" in the copied sourcing xlsx ---
    QXlsx::Document doc{filePath};
    auto dim = doc.dimension();
    if (dim.firstRow() < 0) { doc.save(); return; }

    int cSku         = findCol(doc, "sku");
    int cFnsku       = findCol(doc, "fnsku");
    int cAmazonPrice = findCol(doc, "amazon estimated cost");
    int cSellerCost  = findCol(doc, "seller new cost");
    if (cSellerCost < 0) { doc.save(); return; }

    struct FilledRow { int row; QString sourceFile; };
    QVector<FilledRow> filledRows;

    int totalWithAmazon = 0;
    int amazonHigherOrEqualCount = 0;
    double ratioSum = 0.0;

    for (int row = dim.firstRow() + 1; row <= dim.lastRow(); ++row)
    {
        // Resolve SKU: try direct column first, fall back to FNSKU reverse-lookup
        QString sku;
        if (cSku >= 0)
            sku = doc.read(row, cSku).toString().trimmed();
        if (sku.isEmpty() && cFnsku >= 0)
            sku = fnsku_sku.value(doc.read(row, cFnsku).toString().trimmed());
        if (sku.isEmpty() || !skuInfo.contains(sku)) continue;

        const SkuInfo &info = skuInfo[sku];
        const double pricePerKilo = ui->spinBoxPricePerKilo->value();
        double newPrice = info.price + (info.weightGrams / 1000.0) * pricePerKilo;

        if (cAmazonPrice >= 0)
        {
            bool ok;
            double amazonPrice = doc.read(row, cAmazonPrice).toDouble(&ok);
            if (ok)
            {
                totalWithAmazon++;
                ratioSum += (amazonPrice / newPrice) * 100.0;
                if (newPrice <= amazonPrice)
                {
                    amazonHigherOrEqualCount++;
                    continue;
                }
            }
        }

        doc.write(row, cSellerCost, newPrice);
        filledRows.append({row, info.sourceFile});
    }

    doc.save();

    // --- Step 3: create -WITH-FILE variant with an extra "Source File" column ---
    QFileInfo fi{filePath};
    const QString withFilePath = fi.dir().filePath(fi.baseName() + "-WITH-FILE." + fi.suffix());
    if (QFile::exists(withFilePath))
        QFile::remove(withFilePath);

    const int cSourceFile = dim.lastColumn() + 1;
    doc.write(dim.firstRow(), cSourceFile, QString("Source File"));
    for (const FilledRow &fr : filledRows)
        doc.write(fr.row, cSourceFile, fr.sourceFile);

    doc.saveAs(withFilePath);

    // --- Step 4: show statistics ---
    QString statsMsg;
    if (totalWithAmazon > 0)
    {
        double pctHigher = 100.0 * amazonHigherOrEqualCount / totalWithAmazon;
        double avgRatio  = ratioSum / totalWithAmazon;
        statsMsg = tr("Rows with Amazon price data: %1\n"
                      "Amazon price ≥ invoice (skipped): %2 (%3%)\n"
                      "Avg Amazon / invoice ratio: %4%")
                   .arg(totalWithAmazon)
                   .arg(amazonHigherOrEqualCount)
                   .arg(pctHigher, 0, 'f', 1)
                   .arg(avgRatio, 0, 'f', 1);
    }
    else
    {
        statsMsg = tr("No rows with Amazon price data found.");
    }
    QMessageBox::information(this, tr("Sourcing Cost Statistics"), statsMsg);
}

