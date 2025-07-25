#include "ListPatternNames.h"

ListPatternNames::ListPatternNames(const QString &id, QObject *parent)
    : ListPatternSkus{id, parent}
{
}

QString ListPatternNames::postFixSettings() const
{
    return "Names";
}
