#ifndef VSEARCHRESULTTREE_H
#define VSEARCHRESULTTREE_H

#include "vtreewidget.h"
#include "vsearch.h"


class VSearchResultTree : public VTreeWidget
{
    Q_OBJECT
public:
    explicit VSearchResultTree(QWidget *p_parent = nullptr);

    void updateResults(const QList<QSharedPointer<VSearchResultItem> > &p_items);

    void clearResults();

signals:
    void countChanged(int p_count);

private:
    void appendItem(const QSharedPointer<VSearchResultItem> &p_item);

    QVector<QSharedPointer<VSearchResultItem> > m_data;
};

#endif // VSEARCHRESULTTREE_H
