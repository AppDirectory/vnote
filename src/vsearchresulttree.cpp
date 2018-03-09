#include "vsearchresulttree.h"

VSearchResultTree::VSearchResultTree(QWidget *p_parent)
    : VTreeWidget(p_parent)
{
    setColumnCount(1);
    setHeaderHidden(true);
    // setContextMenuPolicy(Qt::CustomContextMenu);
}

void VSearchResultTree::updateResults(const QList<QSharedPointer<VSearchResultItem> > &p_items)
{
    clearResults();

    for (auto const & it : p_items) {
        appendItem(it);
    }

    emit countChanged(topLevelItemCount());
}

void VSearchResultTree::clearResults()
{
    clear();

    m_data.clear();

    emit countChanged(topLevelItemCount());
}

void VSearchResultTree::appendItem(const QSharedPointer<VSearchResultItem> &p_item)
{
    m_data.append(p_item);

    QTreeWidgetItem *item = new QTreeWidgetItem(this);
    item->setData(0, Qt::UserRole, m_data.size() - 1);
    item->setText(0, p_item->m_text.isEmpty() ? p_item->m_path : p_item->m_text);
    item->setToolTip(0, p_item->m_path);

    for (auto const & it: p_item->m_matches) {
        QTreeWidgetItem *subItem = new QTreeWidgetItem(item);
        QString text;
        if (it.m_lineNumber > -1) {
            text = QString("[%1] %2").arg(it.m_lineNumber).arg(it.m_text);
        } else {
            text = it.m_text;
        }

        subItem->setText(0, text);
        subItem->setToolTip(0, it.m_text);
    }
}
