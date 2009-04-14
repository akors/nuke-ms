// msglayer.cpp

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

#include <algorithm>

#include "bytes.hpp"
#include "msglayer.hpp"

using namespace nuke_ms;


UnknownMessageLayer::UnknownMessageLayer(
    DataOwnership _memblock,
    BasicMessageLayer::const_data_iterator _data_it,
    std::size_t _data_size,
    bool new_memory_block
)
    : memblock(), data_it(_data_it), data_size(_data_size)
{
    // if we don't want a new memory block, everything is fine
    if (!new_memory_block)
    {
        memblock = _memblock;
        data_it = _data_it;
    }
    // if we do want one, we have to allocate a new buffer and copy the data
    else
    {
        // allocate buffer with appropriate size
        BasicMessageLayer::dataptr_type data(
            new byte_traits::byte_sequence(_data_size)
        );

        // copy buffer
        std::copy(
            _data_it,
            _data_it + _data_size,
            data->begin()
        );

        // assign ownership
        memblock = DataOwnership(data);

        // asign data iterator
        data_it = data->begin();
    }
}


std::size_t UnknownMessageLayer::getSerializedSize() const throw()
{
    // return the size of the memory block
    return data_size;
}


void UnknownMessageLayer::fillSerialized(data_iterator buffer) const throw()
{
    // copy the maintained data into the specified buffer
    std::copy(
        data_it,
        data_it + data_size,
        buffer
    );
}

