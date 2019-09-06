#include "qtp.h"
#include <QProcess>
#include <QFileInfo>
#include <QDir>
#include <QDateTime>
#include <iostream>

bool Qtp::DCMD = false;

void Qtp::makeStampFile()
{
    QFile f("embmake.stp");
    if (!f.open(QFile::WriteOnly))
    {
        output("STP", "stamp file embmake.stp cannot be created", true);
        return;
    }
    f.write(QDateTime::currentDateTime().toString(Qt::ISODate).toLatin1());
    f.close();
}
void Qtp::readStampFile()
{
    QFile f("embmake.stp");
    if (!f.open(QFile::ReadOnly))
    {
        output("STP", "stamp file embmake.stp cannot be read", true);
        return;
    }
    QString time = QString(f.readAll());
    f.close();
    Stamp = QDateTime::fromString(time, Qt::ISODate);
}
QString Qtp::replaceByParam(QString in)
{
    QString ret = "";
    QString itmPrfx;
    QString itmPstfx;
    QString ptrnPrfx;
    QString ptrnPstfx;
    QString bsArgColArg;
    QString bsArg = in;
    QString bsArgColName;
    QStringList bsArgColArgItems;
    if (bsArg.contains("{") && bsArg.contains("}"))
    {
        ptrnPrfx = "";
        ptrnPstfx = "";
        itmPrfx = "";
        itmPstfx = "";
        int Aix = bsArg.indexOf("{");
        int Bix = bsArg.indexOf("}");
        bsArgColArg = bsArg.mid(Aix+1, Bix-Aix-1);
        ptrnPrfx = bsArg.mid(0, Aix);
        ptrnPstfx = bsArg.mid(Bix+1, bsArg.count()-1);
        if (bsArgColArg.contains("|"))
        {
            bsArgColArgItems = bsArgColArg.split("|");
            int bsArgColArgIx = -1;
            itmPrfx = "";
            itmPstfx = "";
            QString itmp;
            for (int k=0;k<bsArgColArgItems.count();k++)
            {
                itmp = bsArgColArgItems.at(k).trimmed();
                if (!contains(itmp))
                {
                    if (k == 0)
                        itmPrfx = itmp;
                    if (bsArgColArgIx != -1)
                        if (k != bsArgColArgIx)
                            itmPstfx = itmp;
                    continue;
                }
                else
                {
                    bsArgColArgIx = k;
                    bsArgColName = itmp;
                }
            }
        }
        else
            bsArgColName = bsArgColArg.trimmed();
        QStringList colPrms = colParams(bsArgColName);
        QString str;
        if (colPrms.count() > 1)
            for (int cp=0;cp<colPrms.count();cp++)
            {
                if (!str.isEmpty())
                    str += " ";
                QString cprm = colPrms.at(cp);
                str += (itmPrfx + cprm + itmPstfx);
                //str += colPrms.at(cp);
            }
        else if (colPrms.count() == 1)
            str += ptrnPrfx + itmPrfx + colPrms.at(0) + itmPstfx + ptrnPstfx;
        ret = str;
    }
    else
        ret = in;
    return ret;
}
void Qtp::makeBuildSteps()
{
    readInfFile();
    Steps.clear();
    QtpListPtr targets = collection("TARGETS");
    if (!targets.get())
    {
        output("ERROR", "No TARGETS section");
        return;
    }
    if (targets->count() == 0)
    {
        output("ERROR", "Empty TARGETS section");
        return;
    }
    BuildStepPtr bs;
    QString bsName;
    QString trgName;
    bool trgDisabled;
    bool bsRunOnBack = false;
    QtpListPtr trgStepCol;
    //QString bsparam;
    QtpListPtr bsCol;
    QStringList tmpList;
    QString bsCmd, bsTypeS, col;
    QStringList args;
    QString bsArgColArg;
    QString bsArg;
    QString bsArgColName;
    QStringList bsArgColArgItems;
    QString ptrnPrfx;
    QString ptrnPstfx;
    BuildStep::bsType bsType = BuildStep::NotSet;
    QString itmPrfx;
    QString itmPstfx;

    for (int trgIx=0;trgIx<targets->count();trgIx++)
    {
        trgName = targets->at(trgIx);
        if (trgName.startsWith("!"))
        {
            trgName = trgName.replace("!", "");
            trgDisabled = true;
        }
        else
            trgDisabled = false;
        trgStepCol = collection(trgName);
        if (!trgStepCol.get())
        {
            output("Warning", QString("Undefined build target [%1]").arg(trgName));
            continue;
        }
        for (int trgStpIx=0;trgStpIx < trgStepCol->count();trgStpIx++)
        {
            //Name
            bsName = trgStepCol->at(trgStpIx);
            //bs = BuildStep::New(bsName);
            bsCol = collection(bsName);
            if (!bsCol.get())
            {
                output("Error", QString("Unusable Build Step definition [%1]").arg(bsName));
                continue;
            }
            if (bsCol->count() < 2)
            {
                output("Warning", QString("Unusable Build Step definition [%1]").arg(bsName));
                continue;
            }
            QString bsparam = bsCol->at(0);            
            col = "";
            bsRunOnBack = false;
            if (bsparam.contains(":"))
            {                
                tmpList = bsparam.split(":");
                if (tmpList.count() < 2)
                {
                    output("Warning", QString("Unusable Build Step definition [%1]").arg(bsName));
                    continue;
                }

                //Type

                bsTypeS = tmpList.at(0).toLower();
                if (bsTypeS.startsWith("&"))
                {
                    bsRunOnBack = true;
                    bsTypeS = bsTypeS.replace("&", "");
                }
                col = tmpList.at(1);
                if (bsTypeS == "foreach")
                    bsType = BuildStep::ForEach;
                else if (bsTypeS == "single")
                    bsType = BuildStep::Single;
                if (!contains(col))
                {
                    output("Warning", QString("Unusable Build Step definition [%1], collection not exists [%2]").arg(bsName).arg(col));
                    continue;
                }
            }
            else
            {
                if (bsparam.startsWith("&"))
                    bsRunOnBack = true;
                bsType = BuildStep::Single;
            }

            //Command
            bsCmd = bsCol->at(1);
            args.clear();

            //Params
            for (int c=2;c<bsCol->count();c++)
            {
                bsArg = bsCol->at(c);
                if (bsArg.contains("{") && bsArg.contains("}"))
                {
                    ptrnPrfx = "";
                    ptrnPstfx = "";
                    itmPrfx = "";
                    itmPstfx = "";
                    int Aix = bsArg.indexOf("{");
                    int Bix = bsArg.indexOf("}");
                    bsArgColArg = bsArg.mid(Aix+1, Bix-Aix-1);
                    ptrnPrfx = bsArg.mid(0, Aix);
                    ptrnPstfx = bsArg.mid(Bix+1, bsArg.count()-1);
                    if (bsArgColArg.contains("|"))
                    {
                        bsArgColArgItems = bsArgColArg.split("|");
                        int bsArgColArgIx = -1;
                        itmPrfx = "";
                        itmPstfx = "";
                        QString itmp;
                        for (int k=0;k<bsArgColArgItems.count();k++)
                        {
                            itmp = bsArgColArgItems.at(k).trimmed();
                            if (!contains(itmp))
                            {
                                if (k == 0)
                                    itmPrfx = itmp;
                                if (bsArgColArgIx != -1)
                                    if (k != bsArgColArgIx)
                                        itmPstfx = itmp;
                                continue;
                            }
                            else
                            {
                                bsArgColArgIx = k;
                                bsArgColName = itmp;
                            }
                        }
                    }
                    else
                        bsArgColName = bsArgColArg.trimmed();
                    QStringList colPrms = colParams(bsArgColName);
                    //QString str;

                    if (colPrms.count() > 1)
                        for (int cp=0;cp<colPrms.count();cp++)
                        {
                            //if (!str.isEmpty())
                                //str += " ";
                            QString cprm = replaceByParam(colPrms.at(cp));
                            args.append(itmPrfx + cprm + itmPstfx);
                            //str += colPrms.at(cp);
                        }
                    else if (colPrms.count() == 1)
                        args.append(ptrnPrfx + itmPrfx + replaceByParam(colPrms.at(0)) + itmPstfx + ptrnPstfx);
                    //str = ptrnPrfx + str + ptrnPstfx;
                    //QString ptrn = bsArg.replace(QString("{%1}").arg(bsArgColArg), str); //replace vsetkeho co je v {} teda itmPrfx a itmPstfx nie su potrebne
                    //args.append(ptrn);
                }
                else
                    args.append(bsArg);
            }

            switch(bsType)
            {
            case BuildStep::ForEach:{
                QtpListPtr items = collection(col);
                if (items.get())
                {
                    QString item;
                    for (int i=0;i<items->count();i++)
                    {
                        QStringList stepArgs;
                        stepArgs.append(args);
                        item = items->at(i);
                        QString arg;
                        for (int a=0;a<stepArgs.count();a++)
                        {
                            arg = stepArgs.at(a);
                            if (arg.contains("$item"))
                                arg = arg.replace("$item", item);
                            if (arg.contains("$ITEM"))
                                arg = arg.replace("$ITEM", item);
                            stepArgs.replace(a, arg);
                        }
                        bs = BuildStep::New(bsName, trgName, workingPath(), trgDisabled, item, bsRunOnBack);
                        bs->setCmd(bsCmd, stepArgs);
                        QFileInfo itemInfo(item);
                        if (!itemInfo.exists())
                            output("WRN", QString("Item not exists: %1").arg(item), true);
                        else
                        {
                            if (itemInfo.lastModified() > Stamp)
                                bs->setModified(true);
                            else
                                bs->setModified(false);
                        }
                        Steps.append(bs);
                    }
                }
            }
            break;
            case BuildStep::Single:{
                QtpListPtr items = collection(col);
                QString str;
                QString item;
                QString arg;
                QString iPrfx;
                QString iPstfx;

                for (int i=0;i<args.count();i++)
                {
                    arg = args.at(i);
                    if (!arg.contains("$items") && !arg.contains("$ITEMS"))
                        continue;
                    QStringList parts;
                    if (arg.contains("$items"))
                        parts = arg.split("$items");
                    else
                        parts = arg.split("$ITEMS");
                    if (parts.count() >= 1)
                        iPrfx = parts.at(0);
                    if (parts.count() >= 2)
                        iPstfx = parts.at(1);
                }
                str = "";
                QStringList itemsArgs;
                if (items.get())
                    for (int i=0;i<items->count();i++)
                    {
                        item = items->at(i);
                        //if (!str.isEmpty())
                          //  str += " ";
                        itemsArgs.append(iPrfx + item + iPstfx);
                    }
                QStringList stepArgs;
                stepArgs.append(args);
                arg = "";
                int stepArgsIxtoInsertItemArgs = -1;
                for (int a=0;a<stepArgs.count();a++)
                {
                    arg = stepArgs.at(a);
                    if (arg.contains("$items"))
                    {
                        arg = str;
                        stepArgsIxtoInsertItemArgs = a;
                    }
                    if (arg.contains("$ITEMS"))
                    {
                        arg = str;
                        stepArgsIxtoInsertItemArgs = a;
                    }
                    //stepArgs.replace(a, arg);

                }
                stepArgs.removeAt(stepArgsIxtoInsertItemArgs);
                for (int i=0;i<itemsArgs.count();i++)
                {
                    stepArgs.insert(stepArgsIxtoInsertItemArgs, itemsArgs.at(i));
                    stepArgsIxtoInsertItemArgs++;
                }
                bs = BuildStep::New(bsName, trgName, workingPath(), trgDisabled, "", bsRunOnBack);
                bs->setCmd(bsCmd, stepArgs);
                Steps.append(bs);
            }
            break;
            case BuildStep::NotSet:
            default:
                break;
            } //switch
        }
    }
}
void Qtp::output(QString step, QString msg, bool err)
{
    if (err)
        std::cerr << QString("%1 [%2]").arg(step).arg(msg).toStdString().c_str() << std::endl;
    else
        std::cout << QString("%1 [%2]").arg(step).arg(msg).toStdString().c_str() << std::endl;
}
QString Qtp::workingPath()
{
    return WorkingDir.path();
}
void Qtp::touchInfFile()
{
    QString infofpath = WorkingDir.absoluteFilePath(INFOFILE);
    QFileInfo finfo(infofpath);
    if (finfo.exists())
    {
        QFile f(infofpath);
        if (!f.remove())
            output("UIF", QString("cannot remove old info file: %1").arg(infofpath), true);
    }
    QFile f (infofpath);
    if (!f.open(QFile::WriteOnly))
    {
        output("UIF", QString("cannot write info file: %1").arg(infofpath), true);
        return;
    }
    f.write(QDateTime::currentDateTime().toString(Qt::ISODate).toLatin1());
    f.close();
}
void Qtp::readInfFile()
{
    QString infofpath = WorkingDir.absoluteFilePath(INFOFILE);
    QFileInfo finfo(infofpath);
    if (!finfo.exists())
    {
        output("UIF", QString("No info file -> Build all"), false);
        Stamp.setDate(QDate(2000, 1, 1));
        Stamp.setTime(QTime(0, 0, 0, 0));
        return;
    }
    QFile f (infofpath);
    if (!f.open(QFile::ReadOnly))
    {
        output("UIF", QString("Cannot read info file -> Build all"), false);
        Stamp.setDate(QDate(2000, 1, 1));
        Stamp.setTime(QTime(0, 0, 0, 0));
        return;
    }
    QString str = QString(f.readAll());
    Stamp = QDateTime::fromString(str, Qt::ISODate);
    output("UIF", QString("Build from %1").arg(str.replace("T", " ")), false);
}
void Qtp::fillArgs(QStringList * dst, QString colName, QString argPrefix, QString argPostfix)
{
    if (dst == 0L)
        return;
    if (!contains(colName))
        return;
    QtpListPtr col = collection(colName);
    QString tmp = "";
    for (int i=0;i<col->count();i++)
    {
        if (!argPrefix.isEmpty())
            tmp = argPrefix;
        tmp += col->at(i);
        if (!argPostfix.isEmpty())
            tmp += argPostfix;
        dst->append(tmp);
        tmp = "";
    }
}
QStringList Qtp::colParams(QString name)
{
    QStringList prms;
    if (contains(name))
    {
        QtpListPtr col = collection(name);
        for (int i=0;i<col->count();i++)
            prms.append(col->at(i));
    }
    return prms;
}
void Qtp::readfile()
{
    if (File.isEmpty())
        return;
    QFileInfo finfo(File);
    WorkingDir = finfo.dir();
    QFile f(File);
    if (!f.open(QFile::ReadOnly))
        return;
    QString pro = QString(f.readAll());
    QStringList lines = pro.split("\n");
    QString line;
    QStringList fields;
    QString field;
    QtpListPtr list;

    int op = QTP_OP_NONE;
    int stage = QTP_SG_NO;
    bool inMultiLine = false;
    for (int i=0;i<lines.count();i++)
    {
        line = lines.at(i);
        if (line.trimmed().isEmpty())
            continue;
        if (line.startsWith("#"))
            continue;
        fields = line.trimmed().split(" ");
        for (int f=0;f<fields.count();f++)
        {
            field = fields.at(f).trimmed();
            if (field.isEmpty())
                continue;
            if (field.contains("/_/"))
                field.replace("/_/", " ");
            if (list.get() == 0L && stage == QTP_SG_NO && !inMultiLine)
            {
                list = collection(field, true);
                stage = QTP_SG_CREATED;
                continue;
            }
            if (op == QTP_OP_NONE)
            {
                if (field == "=")
                {
                    op = QTP_OP_EQUAL;
                    stage = QTP_SG_ITEMS;
                    continue;
                }
                else if (field == "+=")
                {
                    op = QTP_OP_ADD;
                    stage = QTP_SG_ITEMS;
                    continue;
                }
                else if (field == "-=")
                {
                    op = QTP_OP_REM;
                    stage = QTP_SG_ITEMS;
                    continue;
                }
            }
            if (stage == QTP_SG_ITEMS)
            {
                if (field != "\\")
                {
                    if (list.get())
                    {
                        switch(op)
                        {
                        case QTP_OP_EQUAL:
                            list->clear();
                        case QTP_OP_ADD:
                            list->add(field);
                            break;
                        case QTP_OP_REM:
                            list->remove(field);
                            break;
                        }
                    }
                }
                else
                    inMultiLine = true;
            }
        }
        if ((inMultiLine && field != "\\") || !inMultiLine)
        {
            inMultiLine = false;
            stage = QTP_SG_NO;
            list = QtpListPtr();
            op = QTP_OP_NONE;
        }
    }
}
void Qtp::clear(QString col)
{
    if (contains(col))
        Collections.value(col)->clear();
}
bool Qtp::contains(QString col)
{
    return Collections.contains(col);
}
QtpListPtr Qtp::collection(QString col, bool create)
{
    if (contains(col))
        return Collections.value(col);
    if (!create)
        return QtpListPtr();
    QtpListPtr colptr = QtpList::New(col);
    Collections.insert(col, colptr);
    return colptr;
}
void Qtp::remove(QString col)
{
    if (contains(col))
        Collections.remove(col);
}
void Qtp::make(QString target, bool showNotModified)
{
    bool clean = false;
    if (target.toLower() == "clean")
        clean = true;
    QStringList targetChain;
    if (target.isEmpty())
    {
        QtpListPtr dtch = collection("DTARGETS");
        if (dtch.get() == 0L)
        {
            output("MAKE", "No target specified and DTARGETS no specified", true);
            return;
        }
        for (int i=0;i<dtch->count();i++)
            targetChain.append(dtch->at(i));
    }
    else
        targetChain.append(target);

    BuildStepPtr bs;
    QString trg;
    for (int t=0;t<targetChain.count();t++)
    {
        trg = targetChain.at(t);
        for (int i=0;i<Steps.count();i++)
        {
            bs = Steps.at(i);
            if (!trg.isEmpty())
                if (trg != bs->target())
                    continue;
            if (bs->disabled())
                continue;
            if (!clean)
                if (!bs->modified())
                {
                    if (showNotModified)
                        output(QString("%1/%2").arg(bs->target()).arg(bs->name()), QString("%1 - not modified").arg(bs->item()));
                    continue;
                }
            output(QString("%1/%2").arg(bs->target()).arg(bs->name()), bs->item());
            if (DCMD)
                output("DCMD", bs->FullCmd);
            if (bs->run() != 0)
            {
                output("ERR", QString("Cannot run %1").arg(bs->Cmd), true);
                break;
            }
        }
    }
    if (!clean)
        touchInfFile();
}
QtpPtr Qtp::New(QString profile)
{
    QtpPtr ret(new Qtp());
    ret->File = profile;
    ret->readfile();
    ret->makeBuildSteps();
    return ret;
}
Qtp::Qtp()
{
    Stamp = QDateTime(QDate(1970, 1, 1), QTime(0,0,0));
}
