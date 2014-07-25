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

#ifndef ILOG_PARSER_H
#define ILOG_PARSER_H

#include <tsk3/libtsk.h>
#include <string>
#include <vector>

class LogInfo
{
    private:
        std::string m_path;
        std::string m_name;
    public:
        LogInfo (TSK_FS_FILE* fs_file, const char *path) :
            m_path(path), m_name(fs_file->name->name) {};
        std::string getPath() { return m_path; }
        std::string getName() { return m_name; }
};

class LogEvent
{
    private:
        int m_dateCreated;
        int m_dateWritten;
        int m_eventId;
    public:
        LogEvent(LogEvent* event) :
            m_eventId(event->getEventId()),
            m_dateCreated(event->getDateCreated()),
            m_dateWritten(event->getDateWritten()) {}
        LogEvent(int id, int created, int written) :
            m_eventId(id), m_dateCreated(created), m_dateWritten(written) {}
        void setDateCreated(int date) { m_dateCreated = date; }
        void setDateWritten(int date) { m_dateWritten = date; }
        void setEventId(int id) { m_eventId = id; }
        int getDateCreated() { return m_dateCreated; }
        int getDateWritten() { return m_dateWritten; }
        int getEventId() { return m_eventId; }
};

class ILogParser
{
    public:
        virtual std::vector<LogEvent*> 
            parseLogFile(TSK_FS_FILE *file, const char *path) = 0;
        virtual std::string getExtension() = 0;
};

#endif
