#include "qlineview.h"

#include <QApplication>
#include <QClipboard>
#include <QKeyEvent>
#include <QMenu>

QLineView::QLineView(QWidget *parent) :
    QTableView(parent)
{
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this, SIGNAL(customContextMenuRequested(QPoint)), SLOT(customMenuRequested(QPoint)));

    _copyAction = new QAction(tr("&Copier la sélection"), this);
    _copyAction->setShortcuts(QKeySequence::Copy);
    _copyAction->setStatusTip(tr("Copie les lignes sélectionnées"));
    connect(_copyAction, SIGNAL(triggered()), this, SLOT(copy()));

    _copyAllAction = new QAction(tr("&Tout copier"), this);
    _copyAllAction->setShortcuts(QKeySequence::New);
    _copyAllAction->setStatusTip(tr("Copie toutes les lignes"));
    connect(_copyAllAction, SIGNAL(triggered()), this, SLOT(copyAll()));
}

void QLineView::customMenuRequested(QPoint pos){
    Q_UNUSED(pos);

    QMenu *menu=new QMenu(this);
    menu->addAction(_copyAction);
    menu->addAction(_copyAllAction);
    menu->popup(viewport()->mapToGlobal(pos));
}

void QLineView::copySelection(QModelIndexList indexes) {
    QModelIndex previous = indexes.first();
    indexes.removeFirst();
    QString selected_text;
    QModelIndex current;
    Q_FOREACH(current, indexes)
    {
      QVariant data = model()->data(previous);
      QString text = data.toString();
      // At this point `text` contains the text in one cell
      selected_text.append(text);
      // If you are at the start of the row the row number of the previous index
      // isn't the same.  Text is followed by a row separator, which is a newline.
      if (current.row() != previous.row())
      {
        selected_text.append(QLatin1Char('\n'));
      }
      // Otherwise it's the same row, so append a column separator, which is a tab.
      else
      {
        selected_text.append(QLatin1Char('\t'));
      }
      previous = current;
    }

    // add last element
    selected_text.append(model()->data(current).toString());
    selected_text.append(QLatin1Char('\n'));
    QApplication::clipboard()->setText(selected_text);
}

void QLineView::copy() {
    QItemSelectionModel * selection = selectionModel();
      QModelIndexList indexes = selection->selectedIndexes();

      if(indexes.size() < 1)
        return;

      // QModelIndex::operator < sorts first by row, then by column.
      // this is what we need
      std::sort(indexes.begin(), indexes.end());

      // You need a pair of indexes to find the row changes
      copySelection(indexes);
}

void QLineView::copyAll() {
    if( model()->rowCount() == 0){
        return;
    }

    QString selected_text;
    for(int i=0; i < model()->rowCount(); i++){
        QStringList texts;
        for(int j=0; j < model()->columnCount(); j++){
            texts.append(model()->data(model()->index(i, j)).toString());
        }
        selected_text += texts.join(QLatin1Char('\t'));
        selected_text += QLatin1Char('\n');
    }
    QApplication::clipboard()->setText(selected_text);
}

void QLineView::keyPressEvent(QKeyEvent * event) {
    if(event->matches(QKeySequence::Copy)) {
        copy();
    }
    else {
        QTableView::keyPressEvent(event);
    }
}
