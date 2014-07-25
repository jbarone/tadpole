/*
 *   This file is part of TADpole.
 *
 *   TADpole is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   TADpole is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with TADpole.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <algorithm>
#include <config.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <time.h>
#include <tsk3/libtsk.h>
#include "LogProcessor.h"
#include "FileProcessor.h"
#include "Options.h"

#define SPACER "  "

static TSK_TCHAR *progname;

bool collectionSortFunction (AnomalyCollection* c1, AnomalyCollection* c2)
{
    return (c1->getLogs().size() > c2->getLogs().size());
}

void spacer(int count)
{
    for (int i = 0; i < count; i++)
        std::cout << SPACER;
}

void writeTime(time_t* t)
{
    struct tm * timeinfo = localtime(t);
    std::cout << (timeinfo->tm_year + 1900) << "-";
    std::cout.width(2);
    std::cout.fill('0');
    std::cout << (timeinfo->tm_mon + 1) << "-"; //month is 0 indexed
    std::cout.width(2);
    std::cout.fill('0');
    std::cout << timeinfo->tm_mday << "T";
    std::cout.width(2);
    std::cout.fill('0');
    std::cout << timeinfo->tm_hour << ":";
    std::cout.width(2);
    std::cout.fill('0');
    std::cout << timeinfo->tm_min << ":";
    std::cout.width(2);
    std::cout.fill('0');
    std::cout << timeinfo->tm_sec;
}

static void usage ()
{
    std::cerr << "usage: " << progname << " [options] image [image]"
        << std::endl;
    std::cerr << "\tOPTIONS:" << std::endl;
    std::cerr << "\t-i imgtype: The format of the image file\n"
        << "\t\t(use '-i list' for supported types)" << std::endl;
    std::cerr << "\t-f: Scan files in image for anomalies in MAC time" << std::endl;
    std::cerr << "\t-x: Output in XML format" << std::endl;
    std::cerr << "\t-v: verbose output to stderr" << std::endl;
    std::cerr << std::endl;

    exit(1);
}

int main (int argc, char* argv1[])
{
    TSK_IMG_TYPE_ENUM imgtype = TSK_IMG_TYPE_DETECT;
    int ch;
    TSK_TCHAR **argv;
    LogProcessor lp;
    time_t temptime;

#ifdef TSK_WIN32
    argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (argv == NULL) 
    {
        std::cerr << "Error getting wide arguments!" << std::endl;
        exit(1);
    }
#else
    argv = (TSK_TCHAR **)argv1;
#endif

    progname = argv[0];
    setlocale(LC_ALL, "");

    while ((ch = GETOPT(argc, argv, _TSK_T("hlfvi:x"))) > 0 )
    {
        switch (ch)
        {
            case _TSK_T('?'):
            default:
                std::cerr << "Invalid argument: " << argv[OPTIND] << std::endl;
            case _TSK_T('h'):
                usage();

            case _TSK_T('i'):
                if (TSTRCMP(OPTARG, _TSK_T("list")) == 0)
                {
                    tsk_img_type_print(stderr);
                    exit(1);
                }
                imgtype = tsk_img_type_toid(OPTARG);
                if (imgtype == TSK_IMG_TYPE_UNSUPP)
                {
                    std::cerr << "Unsupported image type: " << OPTARG;
                    usage();
                }
                break;

            case _TSK_T('v'):
                tsk_verbose++;
                break;

            case _TSK_T('f'):
                opt.processFiles = 1;
                break;

            case _TSK_T('x'):
                opt.xml = 1;
                break;
        }
    }

    if (OPTIND >= argc)
    {
        //print usage
        std::cerr << "Missing image name!" << std::endl;
        usage();
        exit(1);
    }

    if (lp.openImage(argc - OPTIND, &argv[OPTIND], imgtype, 0))
    {
        tsk_error_print(stderr);
        exit(1);
    }

    if (lp.findAndProcessLogs())
    {
        tsk_error_print(stderr);
        exit(1);
    }

    std::vector<AnomalyCollection*> collections = lp.getAnomalyCollections();

    if (opt.processFiles)
    {
        FileProcessor fp(&collections);
        if (fp.openImage(argc - OPTIND, &argv[OPTIND], imgtype, 0))
        {
            tsk_error_print(stderr);
            exit(1);
        }

        if (fp.findAndProcessFiles())
        {
            tsk_error_print(stderr);
            exit(1);
        }
    }

    //report
    std::vector<AnomalyCollection*>::iterator it;
    sort(collections.begin(), collections.end(), collectionSortFunction);
    if (opt.xml)
    {
        std::cout << "<?xml version=\"1.0\"?>" << std::endl;
        std::cout << "<anomalies>" << std::endl;
        for (it=collections.begin(); it != collections.end(); it++)
        {
            spacer(1);
            std::cout << "<anomaly>" << std::endl;
            temptime = (*it)->getPair()->getPreviousAnomaly()->getPreviousEvent()->getDateCreated();
            spacer(2);
            std::cout << "<realstartcreated>";
            writeTime(&temptime);
            std::cout << "</realstartcreated>" << std::endl;
            temptime = (*it)->getPair()->getPreviousAnomaly()->getPreviousEvent()->getDateWritten();
            spacer(2);
            std::cout << "<realstartwritten>";
            writeTime(&temptime);
            std::cout << "</realstartwritten>" << std::endl;
            temptime = (*it)->getPair()->getNextAnomaly()->getNextEvent()->getDateCreated();
            spacer(2);
            std::cout << "<realendcreated>";
            writeTime(&temptime);
            std::cout << "</realendcreated>" << std::endl;
            temptime = (*it)->getPair()->getNextAnomaly()->getNextEvent()->getDateWritten();
            spacer(2);
            std::cout << "<realendwritten>";
            writeTime(&temptime);
            std::cout << "</realendwritten>" << std::endl;
            temptime = (*it)->getPair()->getPreviousAnomaly()->getNextEvent()->getDateCreated();
            spacer(2);
            std::cout << "<anomalystartcreated>";
            writeTime(&temptime);
            std::cout << "</anomalystartcreated>" << std::endl;
            temptime = (*it)->getPair()->getPreviousAnomaly()->getNextEvent()->getDateWritten();
            spacer(2);
            std::cout << "<anomalystartwritten>";
            writeTime(&temptime);
            std::cout << "</anomalystartwritten>" << std::endl;
            temptime = (*it)->getPair()->getNextAnomaly()->getPreviousEvent()->getDateCreated();
            spacer(2);
            std::cout << "<anomalyendcreated>";
            writeTime(&temptime);
            std::cout << "</anomalyendcreated>" << std::endl;
            temptime = (*it)->getPair()->getNextAnomaly()->getPreviousEvent()->getDateWritten();
            spacer(2);
            std::cout << "<anomalyendwritten>";
            writeTime(&temptime);
            std::cout << "</anomalyendwritten>" << std::endl;
            spacer(2);
            std::cout << "<logs>" << std::endl;
            std::vector<LoggedAnomaly*> logs = (*it)->getLogs();
            std::vector<LoggedAnomaly*>::iterator lit = logs.begin();
            for ( ; lit != logs.end(); lit++)
            {
                spacer(3);
                std::cout << "<log>" << std::endl;
                spacer(4);
                std::cout << "<path>" << (*lit)->getLogInfo()->getPath() << "</path>" << std::endl;
                spacer(4);
                std::cout << "<name>" << (*lit)->getLogInfo()->getName() << "</name>" << std::endl;
                spacer(4);
                std::cout << "<times>" << std::endl;
                temptime = (*lit)->getPair()->getPreviousAnomaly()->getPreviousEvent()->getDateCreated();
                spacer(5);
                std::cout << "<realstartcreated>";
                writeTime(&temptime);
                std::cout << "</realstartcreated>" << std::endl;
                temptime = (*lit)->getPair()->getPreviousAnomaly()->getPreviousEvent()->getDateWritten();
                spacer(5);
                std::cout << "<realstartwritten>";
                writeTime(&temptime);
                std::cout << "</realstartwritten>" << std::endl;
                temptime = (*lit)->getPair()->getNextAnomaly()->getNextEvent()->getDateCreated();
                spacer(5);
                std::cout << "<realendcreated>";
                writeTime(&temptime);
                std::cout << "</realendcreated>" << std::endl;
                temptime = (*lit)->getPair()->getNextAnomaly()->getNextEvent()->getDateWritten();
                spacer(5);
                std::cout << "<realendwritten>";
                writeTime(&temptime);
                std::cout << "</realendwritten>" << std::endl;
                temptime = (*lit)->getPair()->getPreviousAnomaly()->getNextEvent()->getDateCreated();
                spacer(5);
                std::cout << "<anomalystartcreated>";
                writeTime(&temptime);
                std::cout << "</anomalystartcreated>" << std::endl;
                temptime = (*lit)->getPair()->getPreviousAnomaly()->getNextEvent()->getDateWritten();
                spacer(5);
                std::cout << "<anomalystartwritten>";
                writeTime(&temptime);
                std::cout << "</anomalystartwritten>" << std::endl;
                temptime = (*lit)->getPair()->getNextAnomaly()->getPreviousEvent()->getDateCreated();
                spacer(5);
                std::cout << "<anomalyendcreated>";
                writeTime(&temptime);
                std::cout << "</anomalyendcreated>" << std::endl;
                temptime = (*lit)->getPair()->getNextAnomaly()->getPreviousEvent()->getDateWritten();
                spacer(5);
                std::cout << "<anomalyendwritten>";
                writeTime(&temptime);
                std::cout << "</anomalyendwritten>" << std::endl;
                spacer(4);
                std::cout << "</times>" << std::endl;
                spacer(3);
                std::cout << "</log>" << std::endl;
            }
            spacer(2);
            std::cout << "</logs>" << std::endl;

            if ((*it)->getFiles().size() > 0) {
                spacer(2);
                std::cout << "<files>" << std::endl;
                std::vector<file_info*> files = (*it)->getFiles();
                std::vector<file_info*>::iterator fit = files.begin();
                for ( ; fit != files.end(); fit++)
                {
                    spacer(3);
                    std::cout << "<file>" << std::endl;
                    spacer(4);
                    std::cout << "<path>" << (*fit)->path << "</path>" << std::endl;
                    spacer(4);
                    std::cout << "<name>" << (*fit)->name << "</name>" << std::endl;
                    spacer(3);
                    std::cout << "</file>" << std::endl;
                }
                spacer(2);
                std::cout << "</files>" << std::endl;
            }

            spacer(1);
            std::cout << "</anomaly>" << std::endl;
        }
        std::cout << "</anomalies>" << std::endl;
    }
    else
    {
        for (it=collections.begin(); it != collections.end(); it++)
        {
            std::cout << "Anomaly" << std::endl;
            spacer(2);
            std::cout << "real    (created): ";
            temptime = (*it)->getPair()->getPreviousAnomaly()->getPreviousEvent()->getDateCreated();
            writeTime(&temptime);
            std::cout << " - ";
            temptime = (*it)->getPair()->getNextAnomaly()->getNextEvent()->getDateCreated();
            writeTime(&temptime);
            std::cout << std::endl;
            spacer(2);
            std::cout << "real    (written): ";
            temptime = (*it)->getPair()->getPreviousAnomaly()->getPreviousEvent()->getDateWritten();
            writeTime(&temptime);
            std::cout << " - ";
            temptime = (*it)->getPair()->getNextAnomaly()->getNextEvent()->getDateWritten();
            writeTime(&temptime);
            std::cout << std::endl;
            spacer(2);
            std::cout << "anomaly (created): ";
            temptime = (*it)->getPair()->getPreviousAnomaly()->getNextEvent()->getDateCreated();
            writeTime(&temptime);
            std::cout << " - ";
            temptime = (*it)->getPair()->getNextAnomaly()->getPreviousEvent()->getDateCreated();
            writeTime(&temptime);
            std::cout << std::endl;
            spacer(2);
            std::cout << "anomaly (written): ";
            temptime = (*it)->getPair()->getPreviousAnomaly()->getNextEvent()->getDateWritten();
            writeTime(&temptime);
            std::cout << " - ";
            temptime = (*it)->getPair()->getNextAnomaly()->getPreviousEvent()->getDateWritten();
            writeTime(&temptime);
            std::cout << std::endl;

            spacer(2);
            std::cout << "logs:" << std::endl;
            std::vector<LoggedAnomaly*> logs = (*it)->getLogs();
            std::vector<LoggedAnomaly*>::iterator lit = logs.begin();
            for ( ; lit != logs.end(); lit++)
            {
                spacer(4);
                std::cout << (*lit)->getLogInfo()->getPath();
                std::cout << (*lit)->getLogInfo()->getName() << std::endl;
            }

            if ((*it)->getFiles().size() > 0) {
                spacer(2);
                std::cout << "files:" << std::endl;
                std::vector<file_info*> files = (*it)->getFiles();
                std::vector<file_info*>::iterator fit = files.begin();
                for ( ; fit != files.end(); fit++)
                {
                    spacer(4);
                    std::cout << (*fit)->path << (*fit)->name << std::endl;
                }
            }
        }
    }

    return 0;
}
