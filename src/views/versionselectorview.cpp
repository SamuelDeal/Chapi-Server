#include "versionselectorview.h"

#include <QLabel>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QComboBox>
#include <QPushButton>
#include <QDebug>

#include "../models/versionlist.h"


VersionSelectorView::VersionSelectorView(QWidget *parent) : QDialog(parent)
{

}

VersionSelectorView::~VersionSelectorView()
{

}

class ReverseCompVersion {
public:    bool operator()(Version *a, Version *b) {
        return *b < *a;
    }
};

Version* VersionSelectorView::select(const QString &currentVersionName, VersionList &versionList, QWidget *parent) {
    VersionSelectorView dlg(parent);
    dlg._versions = versionList.getAll();
    if(dlg._versions.length() == 0){
        return NULL;
    }
    qSort(dlg._versions.begin(), dlg._versions.end(), ReverseCompVersion());
    dlg.init(currentVersionName, versionList);
    return dlg.getVersion();
}

void VersionSelectorView::init(const QString &currentVersionName, VersionList &versionList) {
    setWindowTitle("Choix de la versionde Chapi");

    QLabel *label = new QLabel("Choisissez la version à installer", this);
    label->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    _btns = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);


    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    //we want to let the input dialog grow to available size on Symbian.
    mainLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    mainLayout->addWidget(label);

    _combo = new QComboBox(this);



    _currentVersionIndex = -1;
    int i = 0;
    for(Version *version : _versions){
        _combo->addItem(version->name(), QVariant(i));
        if(version->name() == currentVersionName){
            _combo->setItemData(i, QColor(Qt::gray), Qt::TextColorRole);
            _currentVersionIndex = i;
        }
        ++i;
    }

    if(i != 0){
        _combo->setCurrentIndex(0);
        onItemChanged(0);
    }
    else{
        onItemChanged(-1);
    }
    mainLayout->addWidget(_combo);
    mainLayout->addWidget(_btns);

    connect(_btns, SIGNAL(accepted()), this, SLOT(accept()));
    connect(_btns, SIGNAL(rejected()), this, SLOT(reject()));
    connect(_combo, SIGNAL(currentIndexChanged(int)), this, SLOT(onItemChanged(int)));
}

Version* VersionSelectorView::getVersion() {
    int result = exec();
    if(result != QDialog::Accepted){
        return NULL;
    }
    result = _combo->currentIndex();
    if((result == _currentVersionIndex) || (result < 0) || (result >= _versions.length())) {
        return NULL;
    }
    return _versions[result];
}

void VersionSelectorView::onItemChanged(int index) {
    bool valid = (index != _currentVersionIndex) && (index >= 0) && (index < _versions.length());
    _btns->button(QDialogButtonBox::Ok)->setEnabled(valid);
}

