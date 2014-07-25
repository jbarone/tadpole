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
#include "FileProcessor.h"

FileProcessor::FileProcessor(std::vector<AnomalyCollection*>* collections)
{
    m_collections = collections;
}

TSK_RETVAL_ENUM
FileProcessor::processFile(TSK_FS_FILE *fs_file, const char *path)
{
    if (isDotDir(fs_file, path))
        return TSK_OK;
    else if (isDir(fs_file))
        return TSK_OK;

    if (fs_file->meta)
    {
        time_t atime = fs_file->meta->atime;
        time_t mtime = fs_file->meta->mtime;
        time_t crtime = fs_file->meta->crtime;

        std::vector<AnomalyCollection*>::iterator it;
        for (it=m_collections->begin(); it != m_collections->end(); it++)
        {
            if ((atime > (*it)->getPair()->getPreviousAnomaly()->getNextEvent()->getDateCreated() &&
                atime < (*it)->getPair()->getNextAnomaly()->getPreviousEvent()->getDateCreated()) ||
                (atime > (*it)->getPair()->getPreviousAnomaly()->getNextEvent()->getDateWritten() &&
                atime < (*it)->getPair()->getNextAnomaly()->getPreviousEvent()->getDateWritten()) ||

                (mtime > (*it)->getPair()->getPreviousAnomaly()->getNextEvent()->getDateCreated() &&
                mtime < (*it)->getPair()->getNextAnomaly()->getPreviousEvent()->getDateCreated()) ||
                (mtime > (*it)->getPair()->getPreviousAnomaly()->getNextEvent()->getDateWritten() &&
                mtime < (*it)->getPair()->getNextAnomaly()->getPreviousEvent()->getDateWritten()) ||

                (crtime > (*it)->getPair()->getPreviousAnomaly()->getNextEvent()->getDateCreated() &&
                crtime < (*it)->getPair()->getNextAnomaly()->getPreviousEvent()->getDateCreated()) ||
                (crtime > (*it)->getPair()->getPreviousAnomaly()->getNextEvent()->getDateWritten() &&
                crtime < (*it)->getPair()->getNextAnomaly()->getPreviousEvent()->getDateWritten()) )
            {
                file_info *file = new file_info;
                file->path = std::string(path);
                file->name = std::string(fs_file->name->name);
                (*it)->addFile(file);
            }
        }
    }

    return TSK_OK;
}

bool
FileProcessor::findAndProcessFiles()
{
    return findFilesInImg();
}
