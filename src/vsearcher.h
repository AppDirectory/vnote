#ifndef VSEARCHER_H
#define VSEARCHER_H

#include <QWidget>
#include <QSharedPointer>
#include "vsearch.h"

class QComboBox;
class QCheckBox;
class QPushButton;
class QLabel;
class VSearchResultTree;
class QProgressBar;
class QPlainTextEdit;

class VSearcher : public QWidget
{
    Q_OBJECT
public:
    explicit VSearcher(QWidget *p_parent = nullptr);

private:
    void startSearch();

private:
    void setupUI();

    void initUIFields();

    void setProgressVisible(bool p_visible);

    void appendLogLine(const QString &p_text);

    void handleInputChanged();

    void animateSearchClick();

    void updateItemToComboBox(QComboBox *p_comboBox);

    // Get the OR of the search options.
    int getSearchOption() const;

    void updateNumLabel(int p_count);

    QComboBox *m_keywordCB;

    // All notebooks, current notebook, and so on.
    QComboBox *m_searchScopeCB;

    // Name, content, tag.
    QComboBox *m_searchObjectCB;

    // Notebook, folder, note.
    QComboBox *m_searchTargetCB;

    QComboBox *m_filePatternCB;

    QComboBox *m_searchEngineCB;

    QCheckBox *m_caseSensitiveCB;

    QCheckBox *m_wholeWordOnlyCB;

    QCheckBox *m_regularExpressionCB;

    QPushButton *m_searchBtn;

    QPushButton *m_clearBtn;

    QPushButton *m_advBtn;

    QPushButton *m_consoleBtn;

    QLabel *m_numLabel;

    VSearchResultTree *m_results;

    QProgressBar *m_proBar;

    QPushButton *m_cancelBtn;

    QPlainTextEdit *m_consoleEdit;

    bool m_inSearch;

    bool m_askedToStop;

    VSearch m_search;
};

#endif // VSEARCHER_H
