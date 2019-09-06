#include "buildstep.h"
#include <QDir>

QString BuildStep::item()
{
    return Item;
}
QString BuildStep::name()
{
    return Name;
}
bool BuildStep::disabled()
{
    return DisabledByTarget;
}
QString BuildStep::target()
{
    return Target;
}
QString BuildStep::setCmd(QString cmd, QStringList args)
{
    Cmd = cmd;
    Args = args;
    QString ret;
    ret += cmd;
    for (int i=0;i<args.count(); i++)
    {
        if (!ret.isEmpty())
            ret += " ";
        ret += args.at(i);
    }
    FullCmd = ret;
    return ret;
}
bool BuildStep::modified()
{
    return Modified;
}
void BuildStep::setModified(bool m)
{
    Modified = m;
}
int BuildStep::run(QString cmdc, QStringList args)
{
    FullCmd = setCmd(cmdc, args);
    return run();
}
int BuildStep::run()
{
    QProcess proc;
    if (!WorkingPath.isEmpty())
    {
        QDir::setCurrent(WorkingPath);
        proc.setWorkingDirectory(WorkingPath);
    }
    proc.closeReadChannel(QProcess::StandardOutput);
    proc.closeReadChannel(QProcess::StandardError);
    int res = -1;
    if (RunOnBackground)
        res = QProcess::startDetached(Cmd, Args);
    else
        res = proc.execute(Cmd, Args);
    //Output = proc.readAll();
    return res;
}
BuildStepPtr BuildStep::New(QString name, QString target, QString workingDir, bool disabledByTarget, QString item, bool runOnBack)
{
    BuildStepPtr ret(new BuildStep());
    ret->Name = name;
    ret->Item = item;
    ret->Target = target;
    ret->WorkingPath = workingDir;
    ret->RunOnBackground = runOnBack;
    ret->DisabledByTarget = disabledByTarget;
    return ret;
}
BuildStep::BuildStep()
{
    Type = NotSet;
    DisabledByTarget = false;
    RunOnBackground = false;
    Modified = true;
}
