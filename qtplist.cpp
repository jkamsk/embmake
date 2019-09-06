#include "qtplist.h"

int QtpList::count()
{
    return Items.count();
}
QString QtpList::at(int i)
{
    if (i < 0 || i >= Items.count())
        return "";
    return Items.at(i);
}
void QtpList::clear()
{
    Items.clear();
}
void QtpList::add(QString itm)
{
    Items.append(itm);
}
void QtpList::remove(QString itm)
{
    Items.removeOne(itm);
}
bool QtpList::contains(QString itm)
{
    return Items.contains(itm);
}
QString QtpList::name()
{
    return Name;
}
QtpList::QtpList(QString name)
{
    Name = name;
}
QtpListPtr QtpList::New(QString name)
{
    QtpListPtr ret(new QtpList(name));
    return ret;
}
