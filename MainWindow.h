#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class ListOrderModel;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    ListOrderModel *getListOrderModel() const;

public slots:
    void browseImagePath();
    void addOrderFile();
    void removeOrderFile();
    void pasteInventoryRecommendation();
    void clearInventoryRecommendation();
    void clearInventoryNotRecommended();
    void clearFiltering();
    void createOrderFile();
    void filter();
    void filterReset();
    void saveRecommendation();
    void loadRecommendation();

private slots:
    void _onCountryChanged(const QString &countryCode);

private:
    Ui::MainWindow *ui;
    void _connectSlots();
    QString m_settingKeyImagePath;
};
#endif // MAINWINDOW_H
