#ifndef DIALOGMISSINGSKUS_H
#define DIALOGMISSINGSKUS_H

#include <QDialog>

namespace Ui {
class DialogMissingSkus;
}

class DialogMissingSkus : public QDialog
{
    Q_OBJECT

public:
    explicit DialogMissingSkus(
        const QList<QStringList> &slistOfSkuskus, QWidget *parent = nullptr);
    ~DialogMissingSkus();

private:
    Ui::DialogMissingSkus *ui;
};

#endif // DIALOGMISSINGSKUS_H
