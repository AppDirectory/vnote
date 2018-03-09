#ifndef VSEARCH_H
#define VSEARCH_H

#include <QString>
#include <QSharedPointer>
#include <QRegExp>
#include <QDebug>

class VFile;

struct VSearchConfig
{
    enum Scope
    {
        NoneScope = 0,
        CurrentNote = 0x1UL,
        OpenedNotes = 0x2UL,
        CurrentFolder = 0x4UL,
        CurrentNotebook = 0x8UL,
        AllNotebooks = 0x10UL
    };

    enum Object
    {
        NoneObject = 0,
        Name = 0x1UL,
        Content = 0x2UL,
        Outline = 0x4UL,
        Tag = 0x8UL
    };

    enum Target
    {
        NoneTarget = 0,
        Note = 0x1UL,
        Folder = 0x2UL,
        Notebook = 0x4UL
    };

    enum Engine
    {
        Internal = 0
    };

    enum Option
    {
        NoneOption = 0,
        CaseSensitive = 0x1UL,
        WholeWordOnly = 0x2UL,
        RegularExpression = 0x4UL
    };

    VSearchConfig()
        : VSearchConfig(Scope::NoneScope,
                        Object::NoneObject,
                        Target::NoneTarget,
                        Engine::Internal,
                        Option::NoneOption,
                        "")
    {
    }


    VSearchConfig(int p_scope,
                  int p_object,
                  int p_target,
                  int p_engine,
                  int p_option,
                  const QString &p_keyword)
        : m_scope(p_scope),
          m_object(p_object),
          m_target(p_target),
          m_engine(p_engine),
          m_option(p_option),
          m_keyword(p_keyword)
    {
    }

    int m_scope;
    int m_object;
    int m_target;
    int m_engine;
    int m_option;
    QString m_keyword;
};


enum class VSearchState
{
    Idle = 0,
    Busy,
    Success,
    Fail,
    Cancelled
};


struct VSearchResultSubItem
{
    VSearchResultSubItem()
        : m_lineNumber(-1)
    {
    }

    VSearchResultSubItem(int p_lineNumber,
                         const QString &p_text)
        : m_lineNumber(p_lineNumber),
          m_text(p_text)
    {
    }

    int m_lineNumber;

    QString m_text;
};


struct VSearchResultItem
{
    bool isEmpty() const
    {
        return m_path.isEmpty() && m_text.isEmpty();
    }

    QString toString() const
    {
        return QString("item text: [%1] path: [%2] subitems: %3")
                      .arg(m_text)
                      .arg(m_path)
                      .arg(m_matches.size());
    }

    // Path of the target.
    QString m_path;

    // Text to displayed. If empty, use @m_path instead.
    QString m_text;

    // Matched places within this item.
    QList<VSearchResultSubItem> m_matches;
};

struct VSearchResult
{
    friend class VSearch;

    VSearchResult(VSearch *p_search)
        : m_state(VSearchState::Idle),
          m_search(p_search)
    {
    }

    bool hasError() const
    {
        return !m_errMsg.isEmpty();
    }

    void addResultItem(const QSharedPointer<VSearchResultItem> &p_item)
    {
        m_items.append(p_item);
    }

    QString toString() const
    {
        QString str = QString("search result: state %1 err %2 items %3")
                             .arg((int)m_state)
                             .arg(!m_errMsg.isEmpty())
                             .arg(m_items.size());

        for (auto const & it : m_items) {
            qDebug() << it->toString();
        }

        return str;
    }

    bool hasResults() const
    {
        return !m_items.isEmpty();
    }

    VSearchState m_state;

    QString m_errMsg;

    QList<QSharedPointer<VSearchResultItem> > m_items;

private:
    VSearch *m_search;
};


class VSearch
{
public:
    explicit VSearch(bool *p_askedToStop = nullptr);

    void setConfig(QSharedPointer<VSearchConfig> p_config);

    // Search CurrentNote.
    QSharedPointer<VSearchResult> search(const VFile *p_file);

private:
    bool askedToStop() const;

    void searchFirstPhase(const VFile *p_file,
                          const QSharedPointer<VSearchResult> &p_result) const;

    bool testTarget(VSearchConfig::Target p_target) const;

    bool testObject(VSearchConfig::Object p_object) const;

    bool testOption(VSearchConfig::Option p_option) const;

    bool matchOneLine(const QString &p_text, const QRegExp &p_reg) const;

    VSearchResultItem *searchForOutline(const VFile *p_file) const;

    bool *m_askedToStop;

    QSharedPointer<VSearchConfig> m_config;

    QRegExp m_searchReg;
};

inline bool VSearch::askedToStop() const
{
    if (m_askedToStop) {
        return *m_askedToStop;
    }

    return false;
}

inline void VSearch::setConfig(QSharedPointer<VSearchConfig> p_config)
{
    m_config = p_config;

    // Compile m_searchReg.
    if (m_config->m_keyword.isEmpty()) {
        m_searchReg = QRegExp();
        return;
    }

    Qt::CaseSensitivity cs = testOption(VSearchConfig::CaseSensitive)
                             ? Qt::CaseSensitive : Qt::CaseInsensitive;
    if (testOption(VSearchConfig::RegularExpression)) {
        m_searchReg = QRegExp(m_config->m_keyword, cs);
    } else {
        QString pattern = QRegExp::escape(m_config->m_keyword);
        if (testOption(VSearchConfig::WholeWordOnly)) {
            pattern = "\\b" + pattern + "\\b";
        }

        m_searchReg = QRegExp(pattern, cs);
    }
}

inline bool VSearch::testTarget(VSearchConfig::Target p_target) const
{
    return p_target & m_config->m_target;
}

inline bool VSearch::testObject(VSearchConfig::Object p_object) const
{
    return p_object & m_config->m_object;
}

inline bool VSearch::testOption(VSearchConfig::Option p_option) const
{
    return p_option & m_config->m_option;
}

inline bool VSearch::matchOneLine(const QString &p_text, const QRegExp &p_reg) const
{
    return p_reg.indexIn(p_text) != -1;
}
#endif // VSEARCH_H
