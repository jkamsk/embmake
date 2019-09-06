#ifndef BUILDSTEP_H
#define BUILDSTEP_H

#include <QString>
#include <QProcess>
#include <boost/shared_ptr.hpp>

class BuildStep;
typedef boost::shared_ptr<BuildStep> BuildStepPtr;

class BuildStep
{
public:
    enum bsType
    {
        NotSet,
        ForEach,
        Single
    };
protected:
    BuildStep();
    bsType Type;
    QString Name;
    QString WorkingPath;
    QString Item;    
    QStringList Items;
    QString Cmd;
    QStringList Args;
    QString FullCmd;
    QString Target;
    bool DisabledByTarget;
    bool RunOnBackground;
    bool Modified;
public:
    bool modified();
    void setModified(bool m);
    void setWorkingDir(QString dirpath);
    QString setCmd(QString setCmd, QStringList args);
    QString Output;
    bool disabled();
    QString target();
    QString name();
    QString item();
    static BuildStepPtr New(QString name, QString target, QString workingDir="", bool disabledByTarget=false, QString item="", bool runOnBack=false);
    int run(QString setCmd, QStringList args);
    int run();
    friend class Qtp;
};

#endif // BUILDSTEP_H
