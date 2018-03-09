#include "vsearch.h"

#include "utils/vutils.h"
#include "vfile.h"
#include "veditarea.h"
#include "vmainwindow.h"
#include "vtableofcontent.h"

extern VMainWindow *g_mainWin;

VSearch::VSearch(bool *p_askedToStop)
    : m_askedToStop(p_askedToStop)
{
}

QSharedPointer<VSearchResult> VSearch::search(const VFile *p_file)
{
    Q_ASSERT(!askedToStop());

    QSharedPointer<VSearchResult> result(new VSearchResult(this));

    if (!testTarget(VSearchConfig::Note)) {
        qDebug() << "search is not applicable for note";
        result->m_state = VSearchState::Success;
        return result;
    }

    result->m_state = VSearchState::Busy;

    searchFirstPhase(p_file, result);

    result->m_state = VSearchState::Success;

    return result;
}

void VSearch::searchFirstPhase(const VFile *p_file,
                               const QSharedPointer<VSearchResult> &p_result) const
{
    Q_ASSERT(testTarget(VSearchConfig::Note));

    if (testObject(VSearchConfig::Name)) {
        QString text = p_file->getName();
        if (matchOneLine(text, m_searchReg)) {
            QSharedPointer<VSearchResultItem> item(new VSearchResultItem());
            item->m_text = text;
            item->m_path = p_file->fetchPath();

            p_result->addResultItem(item);
        }
    } else if (testObject(VSearchConfig::Outline)) {
        VSearchResultItem *item = searchForOutline(p_file);
        if (item) {
            p_result->addResultItem(QSharedPointer<VSearchResultItem>(item));
        }
    }
}

VSearchResultItem *VSearch::searchForOutline(const VFile *p_file) const
{
    VEditArea *area = g_mainWin->getEditArea();
    VEditTab *tab = area->getTab(p_file);
    if (!tab) {
        return NULL;
    }

    const VTableOfContent &toc = tab->getOutline();
    const QVector<VTableOfContentItem> &table = toc.getTable();
    VSearchResultItem *item = NULL;
    for (auto const & it: table) {
        if (it.isEmpty()) {
            continue;
        }

        if (!matchOneLine(it.m_name, m_searchReg)) {
            continue;
        }

        if (!item) {
            item = new VSearchResultItem();

            item->m_text = p_file->getName();
            item->m_path = p_file->fetchPath();
        }

        VSearchResultSubItem sitem(it.m_index, it.m_name);
        item->m_matches.append(sitem);
    }

    return item;
}
