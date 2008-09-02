// bytes.hpp

/*
 *   NMS - Nuclear Messaging System
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


#ifndef BYTES_HPP_INCLUDED
#define BYTES_HPP_INCLUDED

#include <vector>
#include <boost/cstdint.hpp>
#include <climits>
#include <limits>

namespace nms
{


struct byte_traits
{
    /** The type of the smallest adressable data unit in memory */
    typedef boost::int8_t byte_t;

    /** The type of an unsigned integer with the width of two bytes*/
    typedef boost::uint16_t uint2b_t;

    /** The type of a signed integer with the width of two bytes*/
    typedef boost::int16_t int2b_t;


    /** The type for a sequence of bye*/
    typedef std::vector<byte_t> bytes_sequence;
};


template <typename T>
T reversebytes(T x)
{
    T result;

    byte_traits::byte_t* outptr =
            reinterpret_cast<byte_traits::byte_t*>(&result) + sizeof(result);

    const byte_traits::byte_t* inptr =
            reinterpret_cast<byte_traits::byte_t*>(&x);

    while (outptr != reinterpret_cast<byte_traits::byte_t*>(&result))
        *(--outptr) = *(inptr++);

    return result;
}

#ifdef NMS_LITTLE_ENDIAN

#   define nms_htonx(x) reversebytes(x)
#   define nms_ntohx(x) reversebytes(x)

#else

#   define nms_htonx(x) (x)
#   define nms_ntohx(x) (x)

#endif

}; // nms





#endif // ifndef MSGLAYERS_HPP_INCLUDED
