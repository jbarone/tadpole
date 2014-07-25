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

#include "EvtxLogParser.h"
#include <iostream>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include "Crc32.h"
#include "exceptions/Exception.h"

#define EPOCH_DIFF 0x019DB1DED53E8000LL /* 116444736000000000 nsecs */
#define RATE_DIFF 10000000 /* 100 nsecs */

#define HEADER_SIZE 128

#define HEADER_MAGIC "ElfFile\x00"
#define CHUNK_MAGIC "ElfChnk\x00"
#define EVENT_MAGIC "**\x00\x00"

typedef struct EvtxHeader
{
    char magic[8];
    int64_t oldest_chunk_number;
    int64_t current_chunk_number;
    int64_t next_record_number;
    uint32_t header_part_len;
    uint16_t version_minor;
    uint16_t version_major;
    uint16_t header_len;
    uint16_t chunk_count;
    char unknown[76];
    uint32_t flags;
    uint32_t check_sum;
}
EvtxHeader_t;

typedef struct EvtxChunkHeader
{
    char magic[8];
    int64_t oldest_log_record_number;
    int64_t last_log_record_number;
    int64_t oldest_file_record_number;
    int64_t last_file_record_number;
    uint32_t header_len;
    uint32_t offset_last;
    uint32_t offset_next;
    uint32_t data_check_sum;
    char unknown[68];
    uint32_t header_check_sum;
    uint32_t string_table[64];
    uint32_t template_table[32];
}
EvtxChunkHeader_t;

typedef struct EvtxEventRecord
{
    char magic[4];
    uint32_t length;
    int64_t record_id;
    int64_t time_created;
    //char *bin_xml_stream;
    //uint32_t length2;
}
EvtxEventRecord_t;

/* Convert a Windows filetime_t into a UNIX time_t */
time_t fileTimeToUnixTime(int64_t ftime) {
        int64_t tconv = (ftime - EPOCH_DIFF) / RATE_DIFF;
        return (time_t)tconv;
}

bool checkHeader(EvtxHeader_t *header)
{
    if (strncmp(header->magic, HEADER_MAGIC, 8))
        return false;

    Crc32 crc32;
    crc32.addData((uint8_t*)header, 0x78);

    return crc32.getCrc32() == header->check_sum;
}

void printHeader(EvtxHeader_t *header)
{
    std::cerr << "HEADER\nMagic: ";
    for(int i; i < 8; i++)
        std::cerr << header->magic[i];
    std::cerr << std::endl;
    std::cerr << "Oldest Chunk: " << header->oldest_chunk_number << std::endl;
    std::cerr << "Current Chunk: " << header->current_chunk_number << std::endl;
    std::cerr << "Next Record: " << header->next_record_number << std::endl;
    std::cerr << "Header Part: " << header->header_part_len << std::endl;
    std::cerr << "Version Major: " << header->version_major << std::endl;
    std::cerr << "Version Minor: " << header->version_minor << std::endl;
    std::cerr << "Header Length: " << header->header_len << std::endl;
    std::cerr << "Chunk Count: " << header->chunk_count << std::endl;
    std::cerr << "Flags: " << header->flags << std::endl;
    std::cerr << "Checksum: " << header->check_sum << std::endl;
}

void printChunkHeader(EvtxChunkHeader_t *chunk_head)
{
    std::cerr << "\nCHUNK\nMagic: ";
    for(int i; i < 8; i++)
        std::cerr << chunk_head->magic[i];
    std::cerr << std::endl;
    std::cerr << "First Log Record Number: " 
        << chunk_head->oldest_log_record_number << std::endl;
    std::cerr << "Last Log Record Number: " 
        << chunk_head->last_log_record_number << std::endl;
    std::cerr << "First File Record Number: " 
        << chunk_head->oldest_file_record_number << std::endl;
    std::cerr << "Last File Record Number: " 
        << chunk_head->last_file_record_number << std::endl;
    std::cerr << "Header Size: " 
        << chunk_head->header_len << std::endl;
    std::cerr << "Last Offset: " 
        << chunk_head->offset_last << std::endl;
    std::cerr << "Next Offset: " 
        << chunk_head->offset_next << std::endl;
}

bool checkChunkHeader(EvtxChunkHeader_t *chunk_head)
{
    if (strncmp(chunk_head->magic, CHUNK_MAGIC, 8))
        return false;

    Crc32 crc32;
    crc32.addData((uint8_t*)chunk_head, 0x78);
    crc32.addData(((uint8_t*)chunk_head) + 0x80, 0x180);

    return crc32.getCrc32() == chunk_head->header_check_sum;
}

bool checkChunkData(uint8_t *data, uint32_t length, uint32_t crc)
{
    Crc32 crc32;
    crc32.addData(data, length);

    return crc == crc32.getCrc32();
}

bool checkEvent(EvtxEventRecord_t *event)
{
    return strncmp(event->magic, EVENT_MAGIC, 4) == 0;
}

std::vector<LogEvent*>
EvtxLogParser::parseLogFile(TSK_FS_FILE *file, const char *path)
{
    if (tsk_verbose)
        std::cerr << "\nattempting to parse (" << file->name->name << ")\n";
    std::vector<LogEvent*> events;

    EvtxHeader_t header;
    int size = tsk_fs_file_read(file, 0, (char*)&header, sizeof(header),
            TSK_FS_FILE_READ_FLAG_NONE);
    if (!checkHeader(&header))
    {
        throw ReadException("could not find header record");
    }
    if (size != HEADER_SIZE)
    {
        throw ReadException("Header record was too short");
    }

    if (tsk_verbose)
        printHeader(&header);

    //read chunks
    EvtxChunkHeader_t chunk_head;
    int32_t chunk_offset = header.header_len;
    for (int chunk = 0; chunk < header.chunk_count; chunk++)
    {
        size = tsk_fs_file_read(file, chunk_offset, (char*)&chunk_head,
                sizeof(chunk_head), TSK_FS_FILE_READ_FLAG_NONE);
        if (!checkChunkHeader(&chunk_head))
        {
            throw ReadException("chunk header not valid");
        }

        if (tsk_verbose)
            printChunkHeader(&chunk_head);

        int len = chunk_head.offset_next - 0x200;
        char data[len];
        size = tsk_fs_file_read(file, chunk_offset + 0x200, data,
                len, TSK_FS_FILE_READ_FLAG_NONE);

        if (!checkChunkData((uint8_t*)data, len, chunk_head.data_check_sum))
        {
            throw ReadException("chunk data not valid");
        }

        //read events
        int event_offset = 0x200;
        EvtxEventRecord_t event;
        while (event_offset < chunk_head.offset_next)
        {
            size = tsk_fs_file_read(file, chunk_offset + event_offset, 
                    (char*)&event, sizeof(event), TSK_FS_FILE_READ_FLAG_NONE);

            if (!checkEvent(&event))
            {
                throw Exception("event not valid");
            }

            time_t time = fileTimeToUnixTime(event.time_created);
            events.push_back(new LogEvent(
                        event.record_id,
                        time,
                        time));

            //next offset
            event_offset += event.length;
        }

        chunk_offset += 0x10000;
    }

    return events;
}

std::string
EvtxLogParser::getExtension()
{
    return std::string("evtx");
}
