#ifndef DIALOGFILTEROUT_H
#define DIALOGFILTEROUT_H

#include <QDialog>
#include <QItemSelection>
#include <QSet>

class ListPatternNames;
class ListPatternSkus;

namespace Ui {
class DialogFilterOut;
}

class DialogFilterOut : public QDialog
{
    Q_OBJECT

public:
    explicit DialogFilterOut(QWidget *parent = nullptr);
    ~DialogFilterOut();
    bool isWhiteList() const;
    const QStringList &getPatternNames() const;
    const QStringList &getPatternSkus() const;

public slots:
    void addTemplate();
    void removeTemplateSelected();
    void addTitlePattern();
    void removeTitlePattern();
    void addSkuPattern();
    void removeSkuPattern();

private slots:
    void onTemplateSelected(const QItemSelection &selected,
                            const QItemSelection &deselected);

private:
    Ui::DialogFilterOut *ui;
    void _connectSlots();
    void _setPatternEditionEnabled(bool enable);
    ListPatternNames *m_listPatternNames;
    ListPatternSkus *m_listPatternSkus;
    void _clearListPatterns();
    QSet<QString> m_templateIdsWhiteList;
    void _saveWhiteList();
    void _loadWhiteList();
    QString m_settingsKeyWhiteList;
};

#endif // DIALOGFILTEROUT_H
