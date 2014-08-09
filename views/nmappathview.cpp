#include "nmappathview.h"

#include <QSpacerItem>
#include <qgridlayout.h>
#include <qpushbutton.h>
#include <qfiledialog.h>
#include <QCloseEvent>

NmapPathView::NmapPathView(QWidget *parent) :
    QMessageBox(parent)
{
    setWindowTitle("Chapi Serveur");
    setIcon(QMessageBox::Warning);
    setTextFormat(Qt::RichText);
    setText("Localisation de NMap");
    setInformativeText("Ce logiciel requière l'utilitaire NMap.\n"
                       "Merci de l'installer et de réessayer.\n"
                       "\n"
                       "Si vous l'avez déjà installé, merci de préciser son emplacement");
    QSpacerItem* horizontalSpacer = new QSpacerItem(480, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
    QGridLayout* layout = (QGridLayout*)this->layout();
    layout->addItem(horizontalSpacer, layout->rowCount(), 0, 1, layout->columnCount());

    _retryButton = addButton(tr("&Réessayer"), QMessageBox::ActionRole);
    _searchButton = addButton(tr("&Parcourir"), QMessageBox::ActionRole);
    _closeButton = addButton(tr("&Abandonner"), QMessageBox::ActionRole);

    setEscapeButton(_closeButton);
    setDefaultButton(_closeButton);
    setDetailedText("Nmap est un utilitare libre, développé par Insecure.Com LLC, "
        "permettant de scanner un réseau local.\n"
        "\n"
        "Il sert, au sein de Chapi Server, à détecter les appareils sur le même réseau.\n"
        "\n"
        "Vous pouvez le télécharger à l'adresse suivante:\n"
        "http://nmap.org/download.html"
                    );
    setModal(true);
    _isCanceled = true;
    _newPath = "";
}

int NmapPathView::exec() {
    int result = QMessageBox::exec();
    if(clickedButton() == _retryButton){
        _isCanceled = false;
    }
    else if(clickedButton() == _searchButton) {
        _newPath = QFileDialog::getExistingDirectory(this, tr("Emplacement de NMap"),
            "/home", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
        if(_newPath != ""){
            _isCanceled = false;
        }
    }
    return result;
}

bool NmapPathView::isCanceled() {
    return _isCanceled;
}

QString NmapPathView::newPath() {
    return _newPath;
}
