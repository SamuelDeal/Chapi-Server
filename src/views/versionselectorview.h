#ifndef VERSIONSELECTORVIEW_H
#define VERSIONSELECTORVIEW_H

#include <QDialog>

class VersionList;
class Version;
class QListWidgetItem;
class QComboBox;
class QDialogButtonBox;

class VersionSelectorView : public QDialog
{
    Q_OBJECT
public:
    static Version* select(const QString &currenrVersionName, VersionList &versionList, QWidget *parent = NULL);


    explicit VersionSelectorView(QWidget *parent);
    ~VersionSelectorView();

private:
    Version* getVersion();
    void init(const QString &currentVersionName, VersionList &versionList);

    QComboBox *_combo;
    QDialogButtonBox *_btns;
    QList<Version*> _versions;
    int _currentVersionIndex;

signals:

public slots:
    void onItemChanged(int);
};

#endif // VERSIONSELECTORVIEW_H
