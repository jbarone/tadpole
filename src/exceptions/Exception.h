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

#ifndef TADPOLE_EXCEPTION_H
#define TADPOLE_EXCEPTION_H

#include <exception>
#include <string>
#include <typeinfo>

class Exception : public std::exception
{
    private:
        std::string m_message;
    protected:
        virtual const std::string getName() const throw() 
            { return std::string("Exception"); }
    public:
        ~Exception() throw() { }
        Exception(std::string message) throw() : m_message(message) { }
        virtual const char* what() const throw() 
        { 
            return (getName() + std::string(": ") + m_message).c_str(); 
        }
};

class ReadException : public Exception
{
    protected:
        virtual const std::string getName() const throw() 
            { return std::string("ReadException"); }
    public:
        ~ReadException() throw() { }
        ReadException(std::string message) throw() : Exception(message) { }
};

class UnknownRecordTypeException : public Exception
{
    protected:
        virtual const std::string getName() const throw() 
            { return std::string("UnknownRecordTypeException"); }
    public:
        ~UnknownRecordTypeException() throw() { }
        UnknownRecordTypeException(std::string message) throw() : 
            Exception(message) { }
};

#endif
