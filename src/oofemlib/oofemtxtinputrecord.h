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

#ifndef oofemtxtinputrecord_h
#define oofemtxtinputrecord_h

#include "inputrecord.h"
#include "tokenizer.h"

#include <vector>

namespace oofem {

/**
 * Class representing the Input Record for OOFEM txt input file format.
 * The input record is represented as string consisting of several fields.
 */
class OOFEMTXTInputRecord : public InputRecord
{
protected:
    /**
     * Tokenizer is used to parse records.
     * This also enables to perform additional check for input records, since
     * unread fields can be detected
     */
    Tokenizer tokenizer;
    std::vector< bool > readFlag;

    /// Record representation.
    std::string record;

    int lineNumber;

public:
    /// Constructor. Creates an empty input record.
    OOFEMTXTInputRecord();
    /// Constructor. Creates the input record corresponding to given string.
    OOFEMTXTInputRecord(const char *source);
    /// Copy constructor.
    OOFEMTXTInputRecord(const OOFEMTXTInputRecord &);
    /// Destructor.
    virtual ~OOFEMTXTInputRecord() { }
    /// Assignment operator.
    OOFEMTXTInputRecord & operator=(const OOFEMTXTInputRecord &);

    virtual InputRecord *GiveCopy() { return new OOFEMTXTInputRecord(* this); }

public:
    /// Sets the record string.
    void setRecordString(const std::string &newStr);
    /// Returns record string.
    std::string giveRecordAsString() { return this->record; }

    void finish(bool wrn = true);

public:
    virtual IRResultType giveRecordKeywordField(std::string &answer, int &value);
    virtual IRResultType giveRecordKeywordField(std::string &answer);
    virtual IRResultType giveField(int &answer, InputFieldType id);
    virtual IRResultType giveField(double &answer, InputFieldType id);
    virtual IRResultType giveField(bool &answer, InputFieldType id);
    virtual IRResultType giveField(std::string &answer, InputFieldType id);
    virtual IRResultType giveField(FloatArray &answer, InputFieldType id);
    virtual IRResultType giveField(IntArray &answer, InputFieldType id);
    virtual IRResultType giveField(FloatMatrix &answer, InputFieldType id);
    virtual IRResultType giveField(std::vector< std::string > &answer, InputFieldType id);
    virtual IRResultType giveField(Dictionary &answer, InputFieldType id);
    virtual IRResultType giveField(std::list< Range > &answer, InputFieldType id);

    virtual bool hasField(InputFieldType id);
    virtual void printYourself();

    virtual void report_error(const char *_class, const char *proc, InputFieldType id,
                              IRResultType result, const char *file, int line);
    void setLineNumber(int lineNumber) { this->lineNumber = lineNumber; }

protected:
    int giveKeywordIndx(const char *kwd);
    int scanInteger(const char *source, int &value);
    int scanDouble(const char *source, double &value);
    void setReadFlag(int itok) { readFlag [ itok - 1 ] = true; }

    /**
     * Returns position of substring id in source.
     * Id must be separated from rest by blank or by tabulator.
     * @return Value pointer at the end of occurrence id in source. If string not found, returns NULL.
     */
    const char *__getPosAfter(const char *, const char *);
    const char *__scanInteger(const char *source, int *value);
    const char *__scanDouble(const char *source, double *value);
    const char *__skipNextWord(const char *src);

    char *__readSimpleString(const char *source, char *simpleString, int maxchar, const char **remain);
    const char *__readKeyAndVal(const char *source, char *key, int *val, int maxchar, const char **remain);
    const char *__readKeyAndVal(const char *source, char *key, double *val, int maxchar, const char **remain);

    /**
     * Reads single range record from input record represented by *helpSource  string.
     * @param helpSource Pointer to current string position, on return helpSource points
     * to next character after reading range record.
     * @param li Starting range index.
     * @param hi End range index.
     * @return Nonzero on success.
     */
    int readRange(const char **helpSource, int &li, int &hi);
    /**
     * Reads single matrix record from input record represented by *helpSource  string.
     * @param helpSource Pointer to current string position, on return helpSource points
     * to next character after reading range record.
     * @param r Matrix rows.
     * @param c Matrix columns.
     * @param ans Float matrix.
     * @return Nonzero on success.
     */
    int readMatrix(const char *helpSource, int r, int c, FloatMatrix &ans);
};
} // end namespace oofem
#endif // oofemtxtinputrecord_h
