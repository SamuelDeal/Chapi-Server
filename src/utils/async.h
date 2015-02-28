#ifndef ASYNC_H
#define ASYNC_H

#include <QObject>
#include <QRunnable>
#include <QMap>
#include <functional>

class AsyncImpl;

class AsyncError {
public:
    AsyncError(const QString &errorString);
    const QString &errorString() const;

private:
    QString _errorString;
};


class Async {
public:
   static void init();
   static void run(const std::function <void (void)>&task,
                   const std::function <void (void)>&onSuccess = NULL,
                   const std::function <void (const QString&)>&onError = NULL
                   );

private:
    Async();
    Async(const Async& that) = delete;

    static AsyncImpl *_impl;
};



class AsyncRuntime : public QObject, public QRunnable {
    Q_OBJECT

    friend class Async;
    friend class AsyncImpl;

    explicit AsyncRuntime(int id, const std::function <void (void)>&task, QObject *parent = 0);
    virtual ~AsyncRuntime();

    virtual void run();

private:
    std::function <void (void)> _task;
    int _taskId;

signals:
    void success(int taskId);
    void error(int taskId, QString errorString);
};




class AsyncImpl : public QObject
{
    Q_OBJECT

    friend class Async;

    struct TaskDetails {
        std::function <void (void)> onSuccess;
        std::function <void (const QString&)> onError;
    };

    explicit AsyncImpl(QObject *parent = 0);
    ~AsyncImpl();

    void run(const std::function <void (void)>&task,
                const std::function <void (void)>&onSuccess = NULL,
                const std::function <void (const QString&)>&onError = NULL
             );

    QMap<int, TaskDetails> _tasks;
    int _taskCount;

public slots:
    void onSuccess(int taskId);
    void onError(int taskId, QString errorString);
};





#endif // ASYNC_H
