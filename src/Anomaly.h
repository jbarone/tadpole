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

#ifndef ANOMALY_H
#define ANOMALY_H

#include "ILogParser.h"

#define FORWARD_JUMP_DELTA      3600
#define BACKWARD_JUMP_DELTA     300

enum anomaly_type_t { BACKWARD_JUMP_ANOMALY, FORWARD_JUMP_ANOMALY };

struct file_info
{
    std::string path;
    std::string name;
};

class Anomaly
{
    private:
        LogEvent* m_previous_event;
        LogEvent* m_next_event;
        anomaly_type_t m_type;
    public:
        anomaly_type_t getType() { return m_type; }
        LogEvent* getPreviousEvent() { return m_previous_event; }
        LogEvent* getNextEvent() { return m_next_event; }
        Anomaly(anomaly_type_t type, 
                LogEvent* previous_event, 
                LogEvent* next_event) : 
            m_type(type), 
            m_previous_event(previous_event), 
            m_next_event(next_event) {};
        Anomaly(Anomaly* anomaly) : 
            m_type(anomaly->getType()), 
            m_previous_event(anomaly->getPreviousEvent()), 
            m_next_event(anomaly->getNextEvent()) {};
};

class AnomalyPair
{
    private:
        Anomaly* m_previous;
        Anomaly* m_next;
    public:
        Anomaly* getPreviousAnomaly() { return m_previous; }
        Anomaly* getNextAnomaly() { return m_next; }
        AnomalyPair(Anomaly* previous, Anomaly* next) :
            m_previous(previous), m_next(next) {};
        bool intersects(AnomalyPair *pair);
};

class LoggedAnomalies
{
    private:
        LogInfo* m_loginfo;
        std::vector<AnomalyPair*> m_pairs;
    public:
        LoggedAnomalies(LogInfo* logInfo,
                std::vector<AnomalyPair*> pairs) :
            m_loginfo(logInfo), m_pairs(pairs) {};
        LogInfo* getLogInfo() { return m_loginfo; }
        std::vector<AnomalyPair*> getPairs() { return m_pairs; }
};

class LoggedAnomaly
{
    private:
        LogInfo *m_loginfo;
        AnomalyPair *m_pair;
    public:
        LoggedAnomaly(LogInfo *logInfo, AnomalyPair *pair) :
            m_loginfo(logInfo), m_pair(pair) {};
        LogInfo* getLogInfo() { return m_loginfo; }
        AnomalyPair* getPair() { return m_pair; }
};

class AnomalyCollection
{
    private:
        AnomalyPair *m_pair;
        std::vector<LoggedAnomaly*> m_logs;
        std::vector<file_info*> m_files;
    public:
        AnomalyCollection() {};
        void setPair(AnomalyPair *pair) { m_pair = pair; };
        AnomalyPair* getPair() { return m_pair; }
        void addLog(LoggedAnomaly* log);
        std::vector<LoggedAnomaly*> getLogs() { return m_logs; };
        std::vector<file_info*> getFiles() { return m_files; };
        void addFile(file_info* file);
};

#endif
