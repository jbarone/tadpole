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

#include "EvtLogParser.h"
#include <iostream>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include "exceptions/Exception.h"

#define HEADER_SIZE     0x30
#define CURSOR_SIZE     0x28
#define LOG_FIXED_SIZE  0x38

#define HEADER_MAGIC    "\x4C\x66\x4C\x65"
#define HEADER_VERSION  "\x01\x00\x00\x00\x01\x00\x00\x00"
#define CURSOR_MAGIC    \
    "\x11\x11\x11\x11\x22\x22\x22\x22\x33\x33\x33\x33\x44\x44\x44\x44"

enum {
    EVT_HEADER_DIRTY   = 0x1,
    EVT_HEADER_WRAPPED = 0x2,
    EVT_HEADER_LOGFULL = 0x4,
    EVT_HEADER_PRIMARY = 0x8
};

enum RecordType {
    EVT_RECORD_HEADER,
    EVT_RECORD_CURSOR,
    EVT_RECORD_WRAPPED,
    EVT_RECORD_LOG,
    EVT_RECORD_UNKOWN
};

typedef struct EvtHeader
{
    int32_t record_length;
    char magic[4];
    int32_t unknown1;
    int32_t unknown2;
    int32_t first_offset;
    int32_t write_offset;
    int32_t next_record_number;
    int32_t first_record_number;
    int32_t filesize;
    int32_t flags;
    int32_t retention_period;
    int32_t record_length_repeat;
} 
EvtHeader_t;

typedef struct EvtCursor
{
    int32_t record_length;
    char magic[16];
    int32_t first_offset;
    int32_t write_offset;
    uint32_t next_record_number;
    uint32_t first_record_number;
    int32_t record_length_repeat;
} 
EvtCursor_t;

typedef struct EvtLogRecord
{
    int32_t record_length;
    char magic[4];
    int32_t message_number;
    int32_t date_created;
    int32_t date_written;
    int32_t event_id;
    int16_t event_type;
    int16_t string_count;
    int16_t category;
    int16_t unknown1;
    int32_t unknown2;
    int32_t string_offset;
    int32_t sid_length;
    int32_t sid_pointer;
    int32_t data_length;
    int32_t data_pointer;
    //char* var_string;
    //char* computer_name;
    //char* buffer;
} 
EvtLogRecord_t;

void printHeader(EvtHeader_t header)
{
    std::cerr << "rec size: 0x" << std::hex 
        << header.record_length << std::endl;
    std::cerr << "magic: " << header.magic[0] << header.magic[1] 
        << header.magic[2] << header.magic[3] << std::endl;
    std::cerr << "first offset: 0x" << std::hex 
        << header.first_offset << std::endl;
    std::cerr << "write offset: 0x" << std::hex 
        << header.first_offset << std::endl;
    std::cerr << "next: 0x" << std::hex 
        << header.next_record_number << std::endl;
    std::cerr << "first: 0x" << std::hex 
        << header.first_record_number << std::endl;
    std::cerr << "filesize: 0x" << std::hex << header.filesize << std::endl;
    std::cerr << "flags: 0x" << std::hex << header.flags << std::endl;
    std::cerr << "\tdirty: " << (header.flags & EVT_HEADER_DIRTY) 
        << std::endl;
    std::cerr << "\twrapped: " << (header.flags & EVT_HEADER_WRAPPED) 
        << std::endl;
    std::cerr << "\tfull: " << (header.flags & EVT_HEADER_LOGFULL) 
        << std::endl;
    std::cerr << "\tprimary: " << (header.flags & EVT_HEADER_PRIMARY) 
        << std::endl;
    std::cerr << "retention: 0x" << std::hex 
        << header.retention_period << std::endl;
    std::cerr << "rec size: 0x" << std::hex 
        << header.record_length_repeat << std::endl;
}

void printCursor(EvtCursor_t cursor)
{
    std::cerr << "size: 0x" << std::hex << cursor.record_length << std::endl;
    std::cerr << "magic: 0x";
    for(int i = 0; i < 16; i++)
        std::cerr << std::hex << (int)cursor.magic[i];
    std::cerr << std::endl;
    std::cerr << "first offset: 0x" <<std::hex 
        << cursor.first_offset << std::endl;
    std::cerr << "write offset: 0x" <<std::hex 
        << cursor.first_offset << std::endl;
    std::cerr << "next: 0x" <<std::hex 
        << cursor.next_record_number << std::endl;
    std::cerr << "first: 0x" <<std::hex 
        << cursor.first_record_number << std::endl;
    std::cerr << "rec size: 0x" <<std::hex 
        << cursor.record_length_repeat << std::endl;
}

RecordType
getRecordType(TSK_FS_FILE *file, int offset)
{
    RecordType retval = EVT_RECORD_UNKOWN;

    int32_t size;
    int read = tsk_fs_file_read(file, offset, (char*)&size, sizeof(size),
            TSK_FS_FILE_READ_FLAG_NONE);
    if (read == 4)
    {
        if (size == HEADER_SIZE)
        {
            //read and verify magic
            char hmagic[12];
            tsk_fs_file_read(file, offset + 4, hmagic, 12,
                    TSK_FS_FILE_READ_FLAG_NONE);
            if (strncmp(hmagic, HEADER_MAGIC HEADER_VERSION, 12) == 0)
                retval = EVT_RECORD_HEADER;
        }
        else if (size == CURSOR_SIZE)
        {
            //read and verify magic
            char cmagic[16];
            tsk_fs_file_read(file, offset + 4, cmagic, 16,
                    TSK_FS_FILE_READ_FLAG_NONE);
            if (strncmp(cmagic, CURSOR_MAGIC, 16) == 0)
                retval = EVT_RECORD_CURSOR;
        }
        else if (size >= LOG_FIXED_SIZE)
        {
            //read and verify magic and weather or not wrapped
            char lmagic[4];
            tsk_fs_file_read(file, offset + 4, lmagic, 4,
                    TSK_FS_FILE_READ_FLAG_NONE);
            if (strncmp(lmagic, HEADER_MAGIC, 4) == 0)
            {
                if (offset + size >= file->meta->size)
                    retval = EVT_RECORD_WRAPPED;
                else
                    retval = EVT_RECORD_LOG;
            }
        }
    }

    return retval;
}

int
findLastIndexOfCursor(TSK_FS_FILE *file)
{
    char cmagic[16];
    for (int i = (file->meta->size - 16); i >= 0; i--)
    {
        tsk_fs_file_read(file, i, cmagic, 16, TSK_FS_FILE_READ_FLAG_NONE);
        if (strncmp(cmagic, CURSOR_MAGIC, 16) == 0)
            return i - 4;
    }

    return -1;
}

EvtLogRecord_t*
getLogRecord(TSK_FS_FILE *file, int offset, int *newoffset)
{
    EvtLogRecord_t *log = new EvtLogRecord_t;
    RecordType type = getRecordType(file, offset);
    if (type == EVT_RECORD_LOG)
    {
        tsk_fs_file_read(file, offset, (char*)log, LOG_FIXED_SIZE,
                TSK_FS_FILE_READ_FLAG_NONE);
        *newoffset = offset + log->record_length;
    }
    else if (type == EVT_RECORD_WRAPPED)
    {
        //read wrapped record
        int32_t rec_size;
        tsk_fs_file_read(file, offset, (char*)&rec_size, sizeof(int32_t), 
                TSK_FS_FILE_READ_FLAG_NONE);
        int size = tsk_fs_file_read(file, offset, (char*)log, LOG_FIXED_SIZE,
                TSK_FS_FILE_READ_FLAG_NONE);
        if (size < LOG_FIXED_SIZE)
        {
            offset = HEADER_SIZE;
            tsk_fs_file_read(file, offset, (char*)log, LOG_FIXED_SIZE,
                    TSK_FS_FILE_READ_FLAG_NONE);
            *newoffset = offset + log->record_length;
        }
        else if (size == LOG_FIXED_SIZE)
        {
            *newoffset = HEADER_SIZE + 
                (rec_size - ((file->meta->size) - offset));
        }
        else if (size < rec_size)
        {
            tsk_fs_file_read(file, HEADER_SIZE, (char*)(log+size), 
                    LOG_FIXED_SIZE - size, TSK_FS_FILE_READ_FLAG_NONE);
            *newoffset = HEADER_SIZE + (rec_size - size);
        }
        else
            throw ReadException("Error reading wrapped record");
    }
    else if (type == EVT_RECORD_CURSOR)
    {
        delete log;
        return NULL;
    }
    else
    {
        throw UnknownRecordTypeException("Log record not a known type");
    }

    return log;
}

std::vector<LogEvent*>
EvtLogParser::parseLogFile(TSK_FS_FILE *file, const char *path)
{
    if (tsk_verbose)
        std::cerr << "\nattempting to parse (" << file->name->name << ")\n";
    std::vector<LogEvent*> events;

    // Make sure header exists
    RecordType type = getRecordType(file, 0);
    if (type != EVT_RECORD_HEADER)
    {
        throw ReadException("could not find header record");
    }

    EvtHeader_t header;
    int size = tsk_fs_file_read(file, 0, (char*)&header, sizeof(header), 
            TSK_FS_FILE_READ_FLAG_NONE);

    if (size != HEADER_SIZE)
    {
        throw ReadException("Header record was too short");
    }

    if(tsk_verbose)
    {
        printHeader(header);
        std::cerr << std::endl;
    }

    // Get cursor
    EvtCursor_t cursor;
    type = getRecordType(file, header.write_offset);
    if (type != EVT_RECORD_CURSOR)
    {
        // Header not point to cursor, need to search for it.
        if (tsk_verbose)
        {
            std::cerr << "WARNING: Header does not point to cursor record.\n";
            std::cerr << "WARNING: Searching for cursor manually...\n";
        }

        int offset = findLastIndexOfCursor(file);
        if(offset >= 0 && 
                ((type = getRecordType(file, offset)) == EVT_RECORD_CURSOR))
        {
            size = tsk_fs_file_read(file, header.write_offset, (char*)&cursor,
                    sizeof(cursor), TSK_FS_FILE_READ_FLAG_NONE);
        }
        else
        {
            throw ReadException("Could not find cursor record");
        }
    }
    else
    {
        // Found cursor. Read it in.
        size = tsk_fs_file_read(file, header.write_offset, (char*)&cursor,
                sizeof(cursor), TSK_FS_FILE_READ_FLAG_NONE);
    }

    if (size != CURSOR_SIZE)
    {
        throw ReadException("Cursor record was too short");
    }

    if (tsk_verbose)
        printCursor(cursor);

    int offset = header.first_offset;
    for (int i = cursor.first_record_number; i < cursor.next_record_number; i++)
    {
        int newoff;
        EvtLogRecord_t *rec = getLogRecord(file, offset, &newoff);
        if (rec == NULL) break;
        events.push_back(new LogEvent(
                    rec->message_number, 
                    rec->date_created, 
                    rec->date_written));
        offset = newoff;
        delete rec;
    }

    return events;
}

std::string
EvtLogParser::getExtension()
{
    return std::string("evt");
}
