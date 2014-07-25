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

#include <iostream>
#include <iomanip>
#include <string>
#include <algorithm>
#include "LogProcessor.h"
#include "EvtLogParser.h"
#include "EvtxLogParser.h"

bool hasEnding (std::string const &fullString, std::string const &ending)
{
    std::string lf(fullString);
    std::string le(ending);
    
    std::transform(lf.begin(), lf.end(), lf.begin(), ::tolower);
    std::transform(le.begin(), le.end(), le.begin(), ::tolower);

    if (lf.length() > le.length())
        return (0 == lf.compare(
                     lf.length() - le.length(), 
                     le.length(),
                     le));
    else
        return false;
}

LogProcessor::LogProcessor()
{
    m_parsers.push_back(new EvtLogParser());
    m_parsers.push_back(new EvtxLogParser());
}

std::vector<Anomaly*> getAnomalies(std::vector<LogEvent*> events)
{
    std::vector<Anomaly*> anomalies;

    if (events.size() > 0)
    {
        LogEvent* previous = events[0];
        for (int i = 1; i < events.size(); i++)
        {
            LogEvent* next = events[i];

            //compare to previous
            if (next->getDateCreated() + BACKWARD_JUMP_DELTA < 
                    previous->getDateCreated()
                    || next->getDateWritten() + BACKWARD_JUMP_DELTA < 
                    previous->getDateWritten())
            {
                //add anomaly to list
                anomalies.push_back(
                        new Anomaly(BACKWARD_JUMP_ANOMALY, 
                            new LogEvent(previous), new LogEvent(next)));
            }
            else if (next->getDateCreated() - FORWARD_JUMP_DELTA >
                        previous->getDateCreated() ||
                        next->getDateWritten() - FORWARD_JUMP_DELTA >
                        previous->getDateWritten())
            {
                anomalies.push_back(
                        new Anomaly(FORWARD_JUMP_ANOMALY, 
                            new LogEvent(previous), new LogEvent(next)));
            }

            //make previous
            previous = next;
        }
    }

    return anomalies;
}

std::vector<AnomalyPair*> getPairs(std::vector<Anomaly*> anomalies)
{
    std::vector<AnomalyPair*> pairs;

    if (anomalies.size() > 0)
    {
        Anomaly* previous = anomalies[0];
        for (int i = 1; i < anomalies.size(); i++)
        {
            Anomaly* next = anomalies[i];
            if(previous->getType() != next->getType())
            {
                pairs.push_back(
                        new AnomalyPair(
                            new Anomaly(previous),
                            new Anomaly(next)));
            }

            //make previous
            previous = next;
        }
    }

    return pairs;
}

TSK_RETVAL_ENUM 
LogProcessor::processFile(TSK_FS_FILE* fs_file, const char *path)
{
    if (isDotDir(fs_file, path))
        return TSK_OK;
    else if (isDir(fs_file))
        return TSK_OK;

    std::string name(fs_file->name->name);

    for (int i = 0; i < m_parsers.size(); i++)
    {
        if (hasEnding(name, m_parsers[i]->getExtension()))
        {
            //parse log file
            std::vector<LogEvent*> events =
                m_parsers[i]->parseLogFile(fs_file, path);
            if (tsk_verbose)
            {
                std::cerr << "Events found in "
                    << path
                    << fs_file->name->name 
                    << std::endl;
                for (int i = 0; i < events.size(); i++)
                {
                    time_t time = events[i]->getDateCreated();
                    std::cerr << "id: " << std::setw(8) << std::left 
                        << events[i]->getEventId() << 
                        " Event timestamp: " << ctime(&time);
                }
            }

            //extract anomalies from events
            std::vector<Anomaly*> anomalies =
                getAnomalies(events);

            //delete events
            while (events.size() > 0)
            {
                delete events.back();
                events.pop_back();
            }

            if (tsk_verbose && anomalies.size() > 0)
            {
                std::cerr << "Anomalies found in " 
                    << path
                    << fs_file->name->name 
                    << std::endl;
                for (int a = 0; a < anomalies.size(); a++)
                {
                    time_t ptime = 
                        anomalies[a]->getPreviousEvent()->getDateCreated();
                    time_t ntime = 
                        anomalies[a]->getNextEvent()->getDateCreated();
                    std::cerr << "type: " << anomalies[a]->getType() 
                        << std::endl;
                    std::cerr << "\tprev: " << ctime(&ptime);
                    std::cerr << "\tnext: " << ctime(&ntime);
                }
            }

            //extact pairs of anomalies
            std::vector<AnomalyPair*> pairs =
                getPairs(anomalies);

            //delete anomalies
            while (anomalies.size() > 0)
            {
                delete anomalies.back();
                anomalies.pop_back();
            }

            if (tsk_verbose && pairs.size() > 0)
                std::cerr << "Anomalious Pairs found in "
                    << path
                    << fs_file->name->name 
                    << std::endl;

            //Store anomalies if they exist
            if (pairs.size() > 0)
            {
                std::string strname(fs_file->name->name);
                std::string strpath(path);
                m_loggedAnomalies.push_back(
                        new LoggedAnomalies(
                            new LogInfo(fs_file, path), 
                            pairs));
                //LogInfo li(fs_file, path);
                //std::cout << "LogInfo: " << li.getPath() << li.getName()
                    //<< std::endl;
            }
        }
    }

    return TSK_OK;
}

bool LogProcessor::findAndProcessLogs()
{
    bool result = findFilesInImg();

    m_collections.clear();

    if (!result)
    {
        for (int i = 0; i < m_loggedAnomalies.size(); i++)
        {
            for (int p = 0; p < m_loggedAnomalies[i]->getPairs().size(); p++)
            {

                bool found = false;

                for (int a = 0; a < m_collections.size(); a++)
                {
                    if (m_loggedAnomalies[i]->getPairs()[p]->intersects(
                                m_collections[a]->getPair()))
                    {
                        m_collections[a]->addLog(new LoggedAnomaly(
                                    m_loggedAnomalies[i]->getLogInfo(),
                                    m_loggedAnomalies[i]->getPairs()[p]));
                        found = true;
                        break;
                    }
                }

                if (!found)
                {
                    AnomalyCollection *c = new AnomalyCollection();
                    c->setPair(m_loggedAnomalies[i]->getPairs()[p]);
                    c->addLog(new LoggedAnomaly(
                                m_loggedAnomalies[i]->getLogInfo(),
                                m_loggedAnomalies[i]->getPairs()[p]));
                    m_collections.push_back(c);
                }

            }
        }
    }
    
    return result;
}
