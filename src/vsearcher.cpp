#include "vsearcher.h"

#include <QtWidgets>
#include <QCoreApplication>

#include "vlineedit.h"
#include "utils/vutils.h"
#include "utils/viconutils.h"
#include "vsearch.h"
#include "vsearchresulttree.h"
#include "vmainwindow.h"

extern VMainWindow *g_mainWin;

VSearcher::VSearcher(QWidget *p_parent)
    : QWidget(p_parent),
      m_inSearch(false),
      m_askedToStop(false),
      m_search(&m_askedToStop)
{
    setupUI();

    initUIFields();

    handleInputChanged();
}

void VSearcher::setupUI()
{
    // Keyword.
    m_keywordCB = VUtils::getComboBox(this);
    m_keywordCB->setEditable(true);
    m_keywordCB->setLineEdit(new VLineEdit(this));
    m_keywordCB->setToolTip(tr("Keywords to search for"));
    connect(m_keywordCB, &QComboBox::currentTextChanged,
            this, &VSearcher::handleInputChanged);
    connect(m_keywordCB->lineEdit(), &QLineEdit::returnPressed,
            this, &VSearcher::animateSearchClick);

    // Scope.
    m_searchScopeCB = VUtils::getComboBox(this);
    m_searchScopeCB->setToolTip(tr("Scope to search"));

    // Object.
    m_searchObjectCB = VUtils::getComboBox(this);
    m_searchObjectCB->setToolTip(tr("Object to search"));

    // Target.
    m_searchTargetCB = VUtils::getComboBox(this);
    m_searchTargetCB->setToolTip(tr("Target to search"));

    // Pattern.
    m_filePatternCB = VUtils::getComboBox(this);
    m_filePatternCB->setEditable(true);
    m_filePatternCB->setLineEdit(new VLineEdit(this));
    m_filePatternCB->setToolTip(tr("Pattern to filter the files to be searched"));

    // Engine.
    m_searchEngineCB = VUtils::getComboBox(this);
    m_searchEngineCB->setToolTip(tr("Engine to execute the search"));

    // Case sensitive.
    m_caseSensitiveCB = new QCheckBox(tr("&Case sensitive"), this);

    // Whole word only.
    m_wholeWordOnlyCB = new QCheckBox(tr("&Whole word only"), this);

    // Regular expression.
    m_regularExpressionCB = new QCheckBox(tr("Re&gular expression"), this);

    QFormLayout *advLayout = new QFormLayout();
    advLayout->addRow(tr("File pattern:"), m_filePatternCB);
    advLayout->addRow(tr("Engine:"), m_searchEngineCB);
    advLayout->addRow(m_caseSensitiveCB);
    advLayout->addRow(m_wholeWordOnlyCB);
    advLayout->addRow(m_regularExpressionCB);
    advLayout->setContentsMargins(0, 0, 0, 0);

    QWidget *advWidget = new QWidget(this);
    advWidget->setLayout(advLayout);
    advWidget->hide();

    // Search button.
    m_searchBtn = new QPushButton(VIconUtils::buttonIcon(":/resources/icons/search.svg"), "", this);
    m_searchBtn->setToolTip(tr("Search"));
    m_searchBtn->setProperty("FlatBtn", true);
    connect(m_searchBtn, &QPushButton::clicked,
            this, &VSearcher::startSearch);

    // Clear button.
    m_clearBtn = new QPushButton(VIconUtils::buttonDangerIcon(":/resources/icons/clear_search.svg"), "", this);
    m_clearBtn->setToolTip(tr("Clear Results"));
    m_clearBtn->setProperty("FlatBtn", true);
    connect(m_clearBtn, &QPushButton::clicked,
            this, [this]() {
                m_results->clearResults();
            });

    // Advanced button.
    m_advBtn = new QPushButton(VIconUtils::buttonIcon(":/resources/icons/search_advanced.svg"),
                               "",
                               this);
    m_advBtn->setToolTip(tr("Advanced Settings"));
    m_advBtn->setProperty("FlatBtn", true);
    m_advBtn->setCheckable(true);
    connect(m_advBtn, &QPushButton::toggled,
            this, [this, advWidget](bool p_checked) {
                advWidget->setVisible(p_checked);
            });

    // Console button.
    m_consoleBtn = new QPushButton(VIconUtils::buttonIcon(":/resources/icons/search_console.svg"),
                                   "",
                                   this);
    m_consoleBtn->setToolTip(tr("Console"));
    m_consoleBtn->setProperty("FlatBtn", true);
    m_consoleBtn->setCheckable(true);
    connect(m_consoleBtn, &QPushButton::toggled,
            this, [this](bool p_checked) {
                m_consoleEdit->setVisible(p_checked);
            });

    m_numLabel = new QLabel(this);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addWidget(m_searchBtn);
    btnLayout->addWidget(m_clearBtn);
    btnLayout->addWidget(m_advBtn);
    btnLayout->addWidget(m_consoleBtn);
    btnLayout->addStretch();
    btnLayout->addWidget(m_numLabel);
    btnLayout->setContentsMargins(0, 0, 3, 0);

    // Progress bar.
    m_proBar = new QProgressBar(this);
    m_proBar->setRange(0, 0);

    // Cancel button.
    m_cancelBtn = new QPushButton(VIconUtils::buttonIcon(":/resources/icons/close.svg"),
                                  "",
                                  this);
    m_cancelBtn->setToolTip(tr("Cancel"));
    m_cancelBtn->setProperty("FlatBtn", true);
    connect(m_cancelBtn, &QPushButton::clicked,
            this, [this]() {
                if (m_inSearch) {
                    m_askedToStop = true;
                    appendLogLine(tr("Cancelling the export..."));
                }
            });

    QHBoxLayout *proLayout = new QHBoxLayout();
    proLayout->addWidget(m_proBar);
    proLayout->addWidget(m_cancelBtn);
    proLayout->setContentsMargins(0, 0, 0, 0);

    // Console.
    m_consoleEdit = new QPlainTextEdit();
    m_consoleEdit->setReadOnly(true);
    m_consoleEdit->setLineWrapMode(QPlainTextEdit::WidgetWidth);
    m_consoleEdit->setProperty("LineEdit", true);
    m_consoleEdit->setPlaceholderText(tr("Output logs will be shown here"));
    m_consoleEdit->setMaximumHeight(50);
    m_consoleEdit->hide();

    // List.
    m_results = new VSearchResultTree(this);
    m_results->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    connect(m_results, &VSearchResultTree::countChanged,
            this, [this](int p_count) {
                m_clearBtn->setEnabled(p_count > 0);
                updateNumLabel(p_count);
            });

    QFormLayout *formLayout = new QFormLayout();
    formLayout->addRow(tr("Keywords:"), m_keywordCB);
    formLayout->addRow(tr("Scope:"), m_searchScopeCB);
    formLayout->addRow(tr("Object:"), m_searchObjectCB);
    formLayout->addRow(tr("Target:"), m_searchTargetCB);
    formLayout->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->addLayout(btnLayout);
    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(advWidget);
    mainLayout->addLayout(proLayout);
    mainLayout->addWidget(m_consoleEdit);
    mainLayout->addWidget(m_results);

    setLayout(mainLayout);
}

void VSearcher::initUIFields()
{
    m_keywordCB->setCurrentText("saas");

    m_consoleBtn->setChecked(true);

    // Scope.
    m_searchScopeCB->addItem(tr("Current Note"), VSearchConfig::CurrentNote);
    m_searchScopeCB->addItem(tr("Opened Notes"), VSearchConfig::OpenedNotes);
    m_searchScopeCB->addItem(tr("Current Folder"), VSearchConfig::CurrentFolder);
    m_searchScopeCB->addItem(tr("Current Notebook"), VSearchConfig::CurrentNotebook);
    m_searchScopeCB->addItem(tr("All Notebooks"), VSearchConfig::AllNotebooks);

    // Object.
    m_searchObjectCB->addItem(tr("Name"), VSearchConfig::Name);
    m_searchObjectCB->addItem(tr("Content"), VSearchConfig::Content);
    m_searchObjectCB->addItem(tr("Outline"), VSearchConfig::Outline);

    // Target.
    m_searchTargetCB->addItem(tr("Note"), VSearchConfig::Note);
    m_searchTargetCB->addItem(tr("Folder"), VSearchConfig::Folder);
    m_searchTargetCB->addItem(tr("Notebook"), VSearchConfig::Notebook);
    m_searchTargetCB->addItem(tr("All"),
                              VSearchConfig::Note
                              | VSearchConfig:: Folder
                              | VSearchConfig::Notebook);

    // Engine.
    m_searchEngineCB->addItem(tr("Internal"), VSearchConfig::Internal);

    setProgressVisible(false);

    m_clearBtn->setEnabled(false);
}

void VSearcher::setProgressVisible(bool p_visible)
{
    m_proBar->setVisible(p_visible);
    m_cancelBtn->setVisible(p_visible);
}

void VSearcher::appendLogLine(const QString &p_text)
{
    m_consoleEdit->appendPlainText(">>> " + p_text);
    m_consoleEdit->ensureCursorVisible();
    QCoreApplication::sendPostedEvents();
}

void VSearcher::handleInputChanged()
{
    bool readyToSearch = true;

    // Keyword.
    QString keyword = m_keywordCB->currentText();
    readyToSearch = !keyword.isEmpty();

    m_searchBtn->setEnabled(readyToSearch);
}

void VSearcher::startSearch()
{
    if (m_inSearch) {
        return;
    }

    qDebug() << "start search";

    m_searchBtn->setEnabled(false);
    setProgressVisible(true);
    m_results->clearResults();
    m_askedToStop = false;
    m_inSearch = true;

    updateItemToComboBox(m_keywordCB);

    QSharedPointer<VSearchConfig> config(new VSearchConfig(m_searchScopeCB->currentData().toInt(),
                                                           m_searchObjectCB->currentData().toInt(),
                                                           m_searchTargetCB->currentData().toInt(),
                                                           m_searchEngineCB->currentData().toInt(),
                                                           getSearchOption(),
                                                           m_keywordCB->currentText()));
    m_search.setConfig(config);

    QSharedPointer<VSearchResult> result;
    switch (config->m_scope) {
    case VSearchConfig::CurrentNote:
        result = m_search.search(g_mainWin->getCurrentFile());
        break;

    case VSearchConfig::OpenedNotes:
        break;

    case VSearchConfig::CurrentFolder:
        break;

    case VSearchConfig::CurrentNotebook:
        break;

    case VSearchConfig::AllNotebooks:
        break;

    default:
        break;
    }

    Q_ASSERT(result->m_state != VSearchState::Idle);

    switch (result->m_state) {
    case VSearchState::Busy:
        qDebug() << "search is on going";
        return;

    case VSearchState::Success:
        qDebug() << "search succeed";
        qDebug() << result->toString();
        break;

    case VSearchState::Fail:
        qDebug() << "search fail";
        VUtils::showMessage(QMessageBox::Warning,
                            tr("Warning"),
                            tr("Search failed."),
                            result->m_errMsg,
                            QMessageBox::Ok,
                            QMessageBox::Ok,
                            this);
        break;

    case VSearchState::Cancelled:
        Q_ASSERT(m_askedToStop);
        appendLogLine(tr("User cancelled the search. Aborted!"));
        m_askedToStop = false;
        break;

    default:
        break;
    }

    if (result->hasResults()) {
        m_results->updateResults(result->m_items);
    }

    if (result->m_state != VSearchState::Fail
        && result->hasError()) {
        VUtils::showMessage(QMessageBox::Warning,
                            tr("Warning"),
                            tr("Errors found during search."),
                            result->m_errMsg,
                            QMessageBox::Ok,
                            QMessageBox::Ok,
                            this);
    }

    m_inSearch = false;
    m_searchBtn->setEnabled(true);
    setProgressVisible(false);
}

void VSearcher::animateSearchClick()
{
    m_searchBtn->animateClick();
}

void VSearcher::updateItemToComboBox(QComboBox *p_comboBox)
{
    QString text = p_comboBox->currentText();
    if (!text.isEmpty() && p_comboBox->findText(text) == -1) {
        p_comboBox->addItem(text);
    }
}

int VSearcher::getSearchOption() const
{
    int ret = VSearchConfig::NoneOption;

    if (m_caseSensitiveCB->isChecked()) {
        ret |= VSearchConfig::CaseSensitive;
    }

    if (m_wholeWordOnlyCB->isChecked()) {
        ret |= VSearchConfig::WholeWordOnly;
    }

    if (m_regularExpressionCB->isChecked()) {
        ret |= VSearchConfig::RegularExpression;
    }

    return ret;
}

void VSearcher::updateNumLabel(int p_count)
{
    m_numLabel->setText(tr("%1 Items").arg(p_count));
}
