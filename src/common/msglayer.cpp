// msglayer.cpp

/*
 *   nuke-ms - Nuclear Messaging System
 *   Copyright (C) 2008, 2010  Alexander Korsunsky
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <algorithm>

#include "bytes.hpp"
#include "msglayer.hpp"

using namespace nuke_ms;



////////////////////////////// StringwrapLayer /////////////////////////////////

StringwrapLayer::StringwrapLayer(const SerializedData& msg)
{
    std::size_t datasize = msg.size();
    const_data_it data_it = msg.getDataIterator();

    // bail out if the string is not aligned
    if (datasize % sizeof(StringType::value_type) !=0)
        throw MsgLayerError("Unaligned packet");

    // set message_string to the proper size
    _message_string.resize((datasize)/sizeof(StringType::value_type));

    // iterator to the message_string
    StringType::iterator out_iter = _message_string.begin();

    std::remove_reference<decltype(*out_iter)>::type tmpval;

    // iterate through all bytes in the sequence
    for ( const_data_it it = data_it; it < data_it + datasize; )
    {
        // read bytes into a character, convert byte endianness
        it = readbytes(&tmpval, it);
        *out_iter++ = to_hostbo(tmpval);
    }

}

