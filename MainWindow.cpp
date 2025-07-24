#include <QFileDialog>
#include <QClipboard>
#include <QApplication>
#include <QSettings>

#include "ListOrderModel.h"
#include "TableInventoryRecommendation.h"

#include "MainWindow.h"
#include "./ui_MainWindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    const auto &countryCode = ui->comboCountry->currentText();
    ListOrderModel *listOrderModel = new ListOrderModel{countryCode, ui->listViewOrderFiles};
    ui->listViewOrderFiles->setModel(listOrderModel);
    ui->tableViewRecommended->setModel(TableInventoryRecommendation::instance());
    _connectSlots();
}

void MainWindow::_connectSlots()
{
    connect(ui->comboCountry,
            &QComboBox::currentTextChanged,
            this,
            &MainWindow::_onCountryChanged);
    connect(ui->buttonAddOrder,
            &QPushButton::clicked,
            this,
            &MainWindow::addOrderFile);
    connect(ui->buttonRemoveOrder,
            &QPushButton::clicked,
            this,
            &MainWindow::removeOrderFile);
    connect(ui->buttonPasteRecommended,
            &QPushButton::clicked,
            this,
            &MainWindow::pasteInventoryRecommendation);
    connect(ui->buttonClearRecommended,
            &QPushButton::clicked,
            this,
            &MainWindow::clearInventoryRecommendation);
    connect(ui->buttonSave,
            &QPushButton::clicked,
            this,
            &MainWindow::save);
}

MainWindow::~MainWindow()
{
    delete ui;
}

ListOrderModel *MainWindow::getListOrderModel() const
{
    return static_cast<ListOrderModel *>(ui->listViewOrderFiles->model());
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

void MainWindow::pasteInventoryRecommendation()
{
    auto clipboard = QApplication::clipboard();
    const auto &text = clipboard->text();
    const auto &textTrimmed = text.trimmed();
    if (!textTrimmed.isEmpty())
    {
        TableInventoryRecommendation::instance()->pasteText(textTrimmed);
    }
}

void MainWindow::clearInventoryRecommendation()
{
    TableInventoryRecommendation::instance()->clear();
}

void MainWindow::save()
{

}

