#include "DialogMissingSkus.h"
#include "ui_DialogMissingSkus.h"

DialogMissingSkus::DialogMissingSkus(
    const QList<QStringList> &listOfSkus, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogMissingSkus)
{
    ui->setupUi(this);
    ui->tableWidget->setRowCount(listOfSkus.size());
    int i = 0;
    for (const auto &skus : listOfSkus)
    {
        for (int j=0; j<skus.size(); ++j)
        {
            ui->tableWidget->setItem(
                i, j, new QTableWidgetItem{skus[j]});
        }
        ++i;
    }
}

DialogMissingSkus::~DialogMissingSkus()
{
    delete ui;
}
