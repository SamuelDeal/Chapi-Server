#include "qselectdialog.h"

#include <QComboBox>
#include <QListWidget>
#include <QLabel>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QStandardItem>
#include <QPushButton>

QSelectDialog::QSelectDialog(QWidget *parent) :
    QDialog(parent)
{

}

QList<QVariant> QSelectDialog::select(const QString &title, const QString &descr, QMap<QVariant, QString> values, QList<QVariant> selection, bool allowMultiple, QWidget *parent) {
    QSelectDialog dlg(parent);
    dlg.setWindowTitle(title);

    QLabel *label = new QLabel(descr, &dlg);
    label->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    dlg._btns = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dlg);


    QVBoxLayout *mainLayout = new QVBoxLayout(&dlg);
    //we want to let the input dialog grow to available size on Symbian.
    mainLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    mainLayout->addWidget(label);

    if(allowMultiple){
        dlg._multi = new QListWidget(&dlg);
        foreach(QVariant key, values.keys()){
            QListWidgetItem * item = new QListWidgetItem(values[key]);
            item->setData(Qt::UserRole, key);
            item->setCheckState(selection.contains(key) ? Qt::Checked : Qt::Unchecked);
            dlg._multi->addItem(item);
        }
        QObject::connect(dlg._multi, SIGNAL(itemChanged(QListWidgetItem*)), &dlg, SLOT(onItemChanged(QListWidgetItem*)));
        mainLayout->addWidget(dlg._multi);
        dlg.onItemChanged(NULL);
    }
    else{
        dlg._one = new QComboBox(&dlg);
        int index = 0;
        int selectionIndex = -1;
        foreach(QVariant key, values.keys()){
            dlg._one->addItem(values[key], key);
            if(selection.contains(key)){
                selectionIndex = index;
            }
            ++index;
        }
        if(selectionIndex != -1){
            dlg._one->setCurrentIndex(selectionIndex);
        }
        mainLayout->addWidget(dlg._one);
    }
    mainLayout->addWidget(dlg._btns);

    QObject::connect(dlg._btns, SIGNAL(accepted()), &dlg, SLOT(accept()));
    QObject::connect(dlg._btns, SIGNAL(rejected()), &dlg, SLOT(reject()));

    int result = dlg.exec();
    QList<QVariant> results;
    if(result != QDialog::Accepted){
        return results;
    }
    if(!allowMultiple){
        results.append(dlg._one->currentData());
    }
    else{
        for(int row = 0; row < dlg._multi->count(); row++) {
            QListWidgetItem *item = dlg._multi->item(row);
            if(item->checkState() == Qt::Checked){
                results.push_back(item->data(Qt::UserRole));
            }
        }
    }
    return results;
}

void QSelectDialog::onItemChanged(QListWidgetItem* item) {
    Q_UNUSED(item);

    bool foundChecked = false;
    for(int row = 0; row < _multi->count(); row++) {
        QListWidgetItem *item = _multi->item(row);
        if(item->checkState() == Qt::Checked){
            foundChecked = true;
            break;
        }
    }
    _btns->button(QDialogButtonBox::Ok)->setEnabled(foundChecked);
}
