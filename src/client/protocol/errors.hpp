// errors.hpp

/*
 *   nuke-ms - Nuclear Messaging System
 *   Copyright (C) 2008  Alexander Korsunsky
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef ERRORS_HPP
#define ERRORS_HPP

#include <stdexcept>

namespace nuke_ms
{
namespace protocol
{


/** Class for errors that can be issued by the Communication Protocol.
* @ingroup proto
*/
class ProtocolError : public std::runtime_error
{
    /** The error message */
    const char* msg;
public:
    /** Default Constructor */
    ProtocolError() throw()
        : std::runtime_error("Unknown Communication Protocol Error")
    { }

    /** Constructor.
    * @param str The error message
    */
    ProtocolError(const char* str) throw()
        : std::runtime_error(str)
    {}

    /** Return error message as char array.
    * @return A null- terminated character array containg the error message
    */
    virtual const char* what() const throw()
    { return std::runtime_error::what(); }

    virtual ~ProtocolError() throw()
    { }
};


} // namespace protocol
} // namespace nuke_ms

#endif // ifndef ERRORS_HPP
