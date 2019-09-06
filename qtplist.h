#ifndef QTPLIST_H
#define QTPLIST_H

#include <QString>
#include <QList>
#include <boost/shared_ptr.hpp>

class QtpList;
typedef boost::shared_ptr<QtpList> QtpListPtr;
class QtpList
{
    QString Name;
    QList<QString> Items;
    QtpList(QString name);
public:
    void add(QString);
    void remove(QString);
    bool contains(QString);
    int count();
    QString at(int);
    void clear();
    QString name();
    static QtpListPtr New(QString name);
};

#endif // QTPLIST_H
