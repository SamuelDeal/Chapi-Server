#include "async.h"

#include <QThreadPool>
#include <QApplication>

AsyncImpl* Async::_impl = NULL;

void Async::init() {
    if(_impl != NULL){
        return;
    }
    if(QApplication::instance()->thread() != QThread::currentThread()) {
        throw AsyncError("you must init Async in the main thread !");
    }
    _impl = new AsyncImpl();
}

void Async::run(const std::function <void (void)>&task,
                const std::function <void (void)>&onSuccess,
                const std::function <void (const QString&)>&onError
                ) {
    init();
    _impl->run(task, onSuccess, onError);
}

AsyncError::AsyncError(const QString &errorString) {
    _errorString = errorString;
}

const QString &AsyncError::errorString() const {
    return _errorString;
}

AsyncRuntime::AsyncRuntime(int id, const std::function <void (void)>&task, QObject *parent) : QObject(parent)
{
    _task = task;
    _taskId = id;
}

AsyncRuntime::~AsyncRuntime()
{}

void AsyncRuntime::run() {
    bool failed = true;
    try{
        _task();
        failed = false;
    }
    catch(AsyncError &err){
        emit error(_taskId, err.errorString());
    }

    if(!failed){
        emit success(_taskId);
    }
}


AsyncImpl::AsyncImpl(QObject *parent) : QObject(parent)
{
    _taskCount = 0;
}

AsyncImpl::~AsyncImpl()
{

}

void AsyncImpl::run(const std::function <void (void)>&task,
            const std::function <void (void)>&onSuccess,
            const std::function <void (const QString&)>&onError
         ) {
    int taskId = ++_taskCount;
    _tasks.insert(taskId, TaskDetails());
    _tasks[taskId].onSuccess = onSuccess;
    _tasks[taskId].onError = onError;

    AsyncRuntime *runtime = new AsyncRuntime(taskId, task);
    connect(runtime, SIGNAL(success(int)), this, SLOT(onSuccess(int)));
    connect(runtime, SIGNAL(error(int,QString)), this, SLOT(onError(int,QString)));
    QThreadPool::globalInstance()->start(runtime);
}

void AsyncImpl::onSuccess(int taskId) {
    if((taskId < 0) || !_tasks.contains(taskId)) {
        return;
    }

    TaskDetails details = _tasks[taskId];
    _tasks.remove(taskId);
    if(details.onSuccess != NULL){
        details.onSuccess();
    }
}

void AsyncImpl::onError(int taskId, QString errorString) {
    if((taskId < 0) || !_tasks.contains(taskId)) {
        return;
    }

    TaskDetails details = _tasks[taskId];
    _tasks.remove(taskId);
    if(details.onError != NULL){
        details.onError(errorString);
    }
}


