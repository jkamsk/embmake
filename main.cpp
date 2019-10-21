#include <QCoreApplication>
#include <QStringList>
#include <qtp.h>
#include <iostream>
#include <QFile>
#include <config.h>

#define ARMSIZE 0
#define AVRSIZE 1

QString armnoneeabisize_binpath;
QString avrsize_binpath;

void message(QString err)
{
    std::cout << err.toStdString().c_str() << std::endl;
}
void error(QString err)
{
    std::cerr << err.toStdString().c_str() << std::endl;
}
int showHelp()
{
     QFile file(":/help.txt");
     if (file.open(QFile::ReadOnly))
     {
        QString hlps = QString(file.readAll());
        std::cout << hlps.toStdString().c_str() << std::endl;
     }
     return 0;
}
int calcArmAvrSize(QString sfile, QString flashsize, QString ramsize, int type)
{
    if (type == ARMSIZE)
        message("ARM flash/ram usage mode");
    else if(type == AVRSIZE)
        message("AVR flash/ram usage mode");
    QFileInfo finfo(sfile);
    if (!finfo.exists())
    {
        error(QString("File not exists, %1").arg(sfile));
        return -200;
    }
    if (!finfo.isReadable())
    {
        error(QString("File not readable, %1").arg(sfile));
        return -201;
    }
    bool ok = false;
    uint fsize = 0;
    uint rsize = 0;
    if (flashsize.startsWith("0x"))
        fsize = flashsize.toUInt(&ok, 16);
    else
        fsize = flashsize.toUInt(&ok, 10);
    if (!ok)
    {
        error(QString("FLASH size, Cannot convert %1 to unsigned int").arg(flashsize));
        return -202;
    }
    if (ramsize.startsWith("0x"))
        rsize = ramsize.toUInt(&ok, 16);
    else
        rsize = ramsize.toUInt(&ok, 10);
    if (!ok)
    {
        error(QString("RAM size, Cannot convert %1 to unsigned int").arg(ramsize));
        return -203;
    }

    QProcess getSize;
    QStringList args;
    args.append(sfile);
    if (type == ARMSIZE)
        getSize.start(armnoneeabisize_binpath, args);
    else if (type == AVRSIZE)
        getSize.start(avrsize_binpath, args);
    getSize.waitForFinished();
    int res = getSize.exitCode();
    if (res != 0)
    {
        error(getSize.readAllStandardError());
        return -204;
    }
    QString outp = getSize.readAll();
    QStringList lines = outp.split("\n");
    QString line;
    bool lineOk = false;
    for (int i=0;i<lines.count();i++)
    {
        line = lines.at(i);
        if (line.contains("text") && line.contains("data") &&
            line.contains("bss") && line.contains("dec") &&
            line.contains("hex"))
        {
            lineOk = true;
            continue;
        }
        if (lineOk)
        {
            line = line.replace("\t", "");
            QStringList fileds = line.split(" ");
            QString fld;
            uint text = 0;
            uint data = 0;
            uint bss = 0;
            int pos = 0; //0-text, 1-data, 2-bss
            for (int l=0;l<fileds.count();l++)
            {
                fld = fileds.at(l).trimmed();
                if (fld.isEmpty())
                    continue;
                ok = false;
                switch(pos)
                {
                case 0:
                    text = fld.toUInt(&ok);
                    break;
                case 1:
                    data = fld.toUInt(&ok);
                    break;
                case 2:
                    bss = fld.toUInt(&ok);
                    break;
                }
                pos++;
                if (pos > 2)
                    break;
            }
            uint uflash = text + data;
            uint uram = data + bss;

            double drsize = static_cast<double>(rsize);
            double dfsize = static_cast<double>(fsize);
            double duflash = static_cast<double>(uflash);
            double duram = static_cast<double>(uram);

            double pflash = ((dfsize - (dfsize - duflash)) / dfsize)*100;
            double pram = ((drsize - (drsize - duram)) / drsize)*100;

            QString overload = "Ok";

            if (pflash > 100.0)
                overload = "!!! OVERFILLED";
            message(QString("Program memory usage : %1 bytes, %2 % Full, %3")
                    .arg(QString::number(uflash).rightJustified(10, ' '))
                    .arg(QString::number(pflash, 'f', 1).rightJustified(5, ' '))
                    .arg(overload));
            overload = "Ok";
            if (pram >100.0)
                overload = "!!! OVERFILLED";
            message(QString("Data memory usage    : %1 bytes, %2 % Full, %3")
                    .arg(QString::number(uram).rightJustified(10, ' '))
                    .arg(QString::number(pram, 'f', 1).rightJustified(5, ' '))
                    .arg(overload));
            return 0;
        }
    }
    return -205;
}
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QString selected_traget = "";
    QString file;
    int sizetype = ARMSIZE;
    bool help = false;
    QStringList sws = a.arguments();
    QString tmp;
    bool calcsize = false;
    QString armsizeFile;
    QString armsizeFlashSize;
    QString armsizeRamSize;
    bool showNotModified = false;
    armnoneeabisize_binpath = ARM_NONE_EABI_SIZE;
    avrsize_binpath = AVR_SIZE;
    for (int a=0;a<sws.count();a++)
    {
        tmp = sws.at(a);
        if (tmp == "-h" || tmp == "-help" || tmp == "--h" || tmp == "--help" || tmp == "\?" || tmp == "/?" || tmp == "?")
            help = true;
        else if (tmp == "-p")
        {
            if (a+1 >= sws.count())
            {
                error("Project file argument not specified");
                return -10;
            }
            else
            {
                file = sws.at(a+1);
                a++;
                continue;
            }
        }
        else if (tmp == "-t")
        {
            if (a+1 >= sws.count())
            {
                error("Build target not specified");
                return -11;
            }
            else
            {
                selected_traget = sws.at(a+1);
                a++;
                continue;
            }
        }        
        else if (tmp == "-dcmd")
        {
            Qtp::DCMD = true;
        }
        else if (tmp == "-armsize_path")
        {
            if (a+1 >= sws.count())
            {
                error("arm-none-eabi-size path not specified");
                return -15;
            }
            else
            {
                armnoneeabisize_binpath = sws.at(a+1);
                a++;
                continue;
            }
        }
        else if (tmp == "-armsize")
        {
            calcsize = true;
            sizetype = ARMSIZE;
        }
        else if (tmp == "-avrsize_path")
        {
            if (a+1 >= sws.count())
            {
                error("avr-size path not specified");
                return -16;
            }
            else
            {
                avrsize_binpath = sws.at(a+1);
                a++;
                continue;
            }
        }
        else if (tmp == "-avrsize")
        {
            calcsize = true;
            sizetype = AVRSIZE;
        }
        else if (tmp == "-snmd")
            showNotModified = true;
        else if (tmp == "-sfile")
        {
            if (a+1 >= sws.count())
            {
                error("file to calc size not specified");
                return -12;
            }
            else
            {
                armsizeFile = sws.at(a+1);
                a++;
                continue;
            }
        }
        else if (tmp == "-sflash")
        {
            if (a+1 >= sws.count())
            {
                error("size of flash not specified");
                return -13;
            }
            else
            {
                armsizeFlashSize = sws.at(a+1);
                a++;
                continue;
            }
        }
        else if (tmp == "-sram")
        {
            if (a+1 >= sws.count())
            {
                error("size of ram not specified");
                return -14;
            }
            else
            {
                armsizeRamSize = sws.at(a+1);
                a++;
                continue;
            }
        }
    }

    if (help)    
        return showHelp();

    if (calcsize)
        return calcArmAvrSize(armsizeFile, armsizeFlashSize.toLower(), armsizeRamSize.toLower(), sizetype);

    // Default make function
    if (file.isEmpty())
    {
        QDir pdir = QDir::current();
        pdir.setNameFilters(QStringList()<<"*.pro");
        QStringList fileList = pdir.entryList(QDir::NoDot|QDir::NoDotDot|QDir::Files);
        switch(fileList.count())
        {
        case 0:
            error("No project file specified");
            return -1;
        case 1:
            file = fileList.at(0);
            break;
        default:
            message(QString("Available %1 project files: ").arg(fileList.count()));
            for (int i=0;i<fileList.count();i++)
                message(QString(" project %1: %2").arg(i+1).arg(fileList.at(i)));
            error("Required specification of project file '-p option'");
            return -1;
        }
    }


    message(QString("Project file: %1").arg(file));
    QtpPtr proj = Qtp::New(file);
    message(QString("Working Path: %1").arg(proj->workingPath()));
    proj->make(selected_traget, showNotModified);

    return 0;
}
