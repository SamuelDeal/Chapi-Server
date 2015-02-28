#ifndef ERRORLIST_H
#define ERRORLIST_H

#include <list>
#include <functional>
#include <QString>

class QWidget;

class ErrorList {
public:
    typedef std::function <void (void)> ErrorCallback;

    ErrorList(const QString &title, QWidget *parentWidget = NULL);

    void addError(bool critical, const QString &description, const ErrorCallback &onIgnore, const ErrorCallback &onAbort);
    void run(bool yesToAllEnabled = true);
    bool isAborted() const;

private:
    enum Status {
        NotRunned,
        Ignored,
        Aborted
    };

    struct ErrorData {
        ErrorData(bool, const QString&, const ErrorCallback&, const ErrorCallback&);
        ErrorData(const ErrorData &err);

        bool critical;
        QString descr;
        ErrorCallback onIgnore;
        ErrorCallback onAbort;
    };

    static bool compData(const ErrorData &a, const ErrorData &b);

    std::list<ErrorData> _errors;
    unsigned int _nbrWarnings;
    unsigned int _nbrCritical;
    Status _status;
    QWidget *_parentWidget;
    QString _title;
};

#endif // ERRORLIST_H
