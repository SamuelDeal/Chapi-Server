#ifndef QSELECTDIALOG_H
#define QSELECTDIALOG_H

#include <QDialog>

class QComboBox;
class QListWidget;
class QDialogButtonBox;
class QListWidgetItem;


class QSelectDialog : public QDialog
{
    Q_OBJECT
public:
    static QList<QVariant> select(const QString &title, const QString &descr, QMap<QVariant, QString> values, QList<QVariant> selection, bool allowMultiple = false, QWidget *parent = NULL);

    explicit QSelectDialog(QWidget *parent);

private:
    QComboBox *_one;
    QListWidget *_multi;
    QDialogButtonBox *_btns;

signals:

public slots:
    void onItemChanged(QListWidgetItem*);
};


#endif // QSELECTDIALOG_H
