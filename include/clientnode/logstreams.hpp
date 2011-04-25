// logstreams.hpp

/*
 *   nuke-ms - Nuclear Messaging System
 *   Copyright (C) 2010  Alexander Korsunsky
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, version 3 of the License.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/** @file clientnode/logstreams.hpp
* @brief Logging output streams used ClientNode module.
* @ingroup clientnode
*
* @author Alexander Korsunsky
*/

#ifndef LOGSTREAMS_HPP
#define LOGSTREAMS_HPP

#include <iostream>

namespace nuke_ms
{


/** @addtogroup clientnode Communication Protocol
 * @{
*/


namespace clientnode
{

/** Wrapper class for logging streams. */
struct LoggingStreams
{
    /** Stream info messages will be written to */
    std::ostream& infostream;

    /** Stream warning messages will be written to */
    std::ostream& warnstream;

    /** Stream error messages will be written to */
    std::ostream& errorstream;

    /** Default constructor, initialize to std::clog and std::cerr */
    LoggingStreams() :
        infostream(std::clog), warnstream(std::cerr), errorstream(std::cerr)
    {}
};

} // namespace clientnode

/**@}*/ // addtogroup clientnode

} // namespace nuke_ms


#endif // ifndef LOGSTREAMS_HPP
