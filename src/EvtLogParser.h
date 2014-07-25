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

#ifndef EVT_LOG_PARSER_H
#define EVT_LOG_PARSER_H

#include "ILogParser.h"

class EvtLogParser : public ILogParser
{
    public:
        virtual std::vector<LogEvent*>
            parseLogFile(TSK_FS_FILE *file, const char *path);
        virtual std::string getExtension();
};

#endif
