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

#ifndef FILE_PROCESSOR_H
#define FILE_PROCESSOR_H

#include <tsk3/libtsk.h>
#include <vector>
#include "Anomaly.h"

class FileProcessor : public TskAuto
{
    private:
        std::vector<AnomalyCollection*>* m_collections;
    public:
        FileProcessor(std::vector<AnomalyCollection*>* collections);
        virtual TSK_RETVAL_ENUM processFile
            (TSK_FS_FILE *fs_file, const char *path);
        bool findAndProcessFiles();
};

#endif
