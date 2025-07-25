#ifndef LISTPATTERNNAMES_H
#define LISTPATTERNNAMES_H

#include "ListPatternSkus.h"
#include <QObject>

class ListPatternNames : public ListPatternSkus
{
public:
    explicit ListPatternNames(
            const QString &id, QObject *parent = nullptr);
    QString postFixSettings() const override;
};

#endif // LISTPATTERNNAMES_H
