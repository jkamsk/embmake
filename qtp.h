#ifndef QTP_H
#define QTP_H

#include <qtplist.h>
#include <boost/shared_ptr.hpp>
#include <QHash>
#include <QDir>
#include <QDateTime>
#include <buildstep.h>

#define QTP_OP_NONE     0
#define QTP_OP_EQUAL    1
#define QTP_OP_ADD      2
#define QTP_OP_REM      3

#define QTP_SG_NO       0
#define QTP_SG_CREATED  1
#define QTP_SG_ITEMS    2

#define INFOFILE        "embmake.inf"

class Qtp;
typedef boost::shared_ptr<Qtp> QtpPtr;
class Qtp
{
    QDateTime Stamp;
    Qtp();
    QHash<QString, QtpListPtr> Collections;
    QString File;
    QDir WorkingDir;
    QList<BuildStepPtr> Steps;
    void readfile();
    void makeBuildSteps();
    void makeStampFile();
    void readStampFile();
    QString replaceByParam(QString in);
public:
    static bool DCMD;
    static void output(QString step, QString msg, bool err=false);    
    void touchInfFile();
    void readInfFile();
    QString workingPath();
    void fillArgs(QStringList * dst, QString colName, QString argPrefix="", QString argPostfix="");
    QStringList colParams(QString name);
    bool contains(QString col);
    QtpListPtr collection(QString col, bool create=false);
    void clear(QString col);
    void remove(QString col);
    void make(QString target="", bool showNotModified = false);
    static QtpPtr New(QString profile);
};

#endif // QTP_H

