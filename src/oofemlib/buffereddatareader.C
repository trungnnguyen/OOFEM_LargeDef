/*
 *
 *                 #####    #####   ######  ######  ###   ###
 *               ##   ##  ##   ##  ##      ##      ## ### ##
 *              ##   ##  ##   ##  ####    ####    ##  #  ##
 *             ##   ##  ##   ##  ##      ##      ##     ##
 *            ##   ##  ##   ##  ##      ##      ##     ##
 *            #####    #####   ##      ######  ##     ##
 *
 *
 *             OOFEM : Object Oriented Finite Element Code
 *
 *               Copyright (C) 1993 - 2013   Borek Patzak
 *
 *
 *
 *       Czech Technical University, Faculty of Civil Engineering,
 *   Department of Structural Mechanics, 166 29 Prague, Czech Republic
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "buffereddatareader.h"
#include "error.h"

#include <cstdio>
#include <cctype>

namespace oofem {
BufferedDataReader :: BufferedDataReader()
{ }

BufferedDataReader :: ~BufferedDataReader()
{ }

InputRecord *
BufferedDataReader :: giveInputRecord(InputRecordType typeId, int recordId)
{
    std::string line;
    this->giveLineFromInput(line);
    ir.setRecordString(line);
    return & ir;
}

void
BufferedDataReader :: finish()
{
    buffer.clear();
}

void
BufferedDataReader :: rewind()
{
    pos = buffer.begin();
}

void
BufferedDataReader :: seek(int position)
{
    int i = 0;

    pos = buffer.begin();

    while ( pos != buffer.end() ) {
        if ( (*pos)[0] != '#' ) {
            if ( ++i == position ) {
                return;
            }
        }

        ++pos;
    }

    OOFEM_ERROR("BufferedDataReader: seek: already at the end");
}


void
BufferedDataReader :: printYourself()
{
    dynaList< std :: string > :: iterator rec;

    printf( "\nBuffer with %d records\n\n", buffer.size() );

    rec = buffer.begin();
    while ( rec != buffer.end() ) {
        printf( "%s\n", ( * rec ).c_str() );
        ++rec;
    }

    printf("\n");
}


std::string
BufferedDataReader :: giveLine()
{
    return *(this->pos);
}

void
BufferedDataReader :: writeToFile(char *fileName)
{
    dynaList< std :: string > :: iterator rec;
    FILE *dataStream;

    if ( ( dataStream = fopen(fileName, "w") ) == NULL ) {
        OOFEM_ERROR2("BufferedDataReader::writeToFile : Can't open data stream %s", fileName);
    }

    rec = buffer.begin();
    while ( rec != buffer.end() ) {
        fprintf( dataStream, "%s\n", ( * rec ).c_str() );
        ++rec;
    }

    fclose(dataStream);
}


void
BufferedDataReader :: appendInputString(std :: string &str)
{
    // Append zero char to the end of the string to prevent rubish to occur
    // at the of the line read in giveRawLineFromInput if previous string was longer !!!
    str += '\0';
    buffer.pushBack(str);
}

void
BufferedDataReader :: appendInputString(const char *line)
{
    std :: string str(line);

    // Append zero char to the end of the string to prevent rubish to occur
    // at the of the line read in giveRawLineFromInput if previous string was longer !!!
    str += '\0';
    buffer.pushBack(str);
}


void
BufferedDataReader :: giveLineFromInput(std::string &line)
{
    // reads one line from inputStream
    // if " detected, start/stop changing to lower case characters
    bool flag = false; //0-tolower, 1-remain with capitals
    
    this->giveRawLineFromInput(line);
    
    for ( std::size_t i = 0; i < line.size(); i++ ) {
        if ( line[i] == '"' ) { //do not change to lowercase inside quotation marks
            flag = !flag; // switch flag
        }
        
        if ( !flag ) {
            line[i] = tolower(line[i]); // convert line to lowercase
        }
    }
}


void
BufferedDataReader :: giveRawLineFromInput(std::string &line)
{
    do {
        if ( pos == buffer.end() ) {
            OOFEM_ERROR("BufferedDataReader: giveRawLineFromInput: already at the end");
        }

        line = *pos;
        ++pos;
    } while ( line[0] == '#' ); // skip comments

}
} // end namespace oofem
