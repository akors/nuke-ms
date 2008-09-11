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

#include <algorithm>
#include <string>
#include <vector>
#include <boost/cstdint.hpp>

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
    typedef std::vector<byte_t> byte_sequence;

};


template <typename T>
inline T reversebytes(T x)
{
	std::reverse(
		reinterpret_cast<byte_traits::byte_t*>(&x),
		reinterpret_cast<byte_traits::byte_t*>(&x) + sizeof(x)
	);

    return x;
}


template <typename T> inline
byte_traits::byte_sequence::iterator
writebytes(byte_traits::byte_sequence::iterator it, T value)
{
	return std::copy(
		reinterpret_cast<byte_traits::byte_t*>(&value),
		reinterpret_cast<byte_traits::byte_t*>(&value) + sizeof(value),
		it);

}

#ifndef NMS_BIG_ENDIAN

template <typename T>
inline T htonx(T x) { return reversebytes(x); }

template <typename T>
inline T nms_ntohx(T x) { return reversebytes(x); }


#else

template <typename T>
inline T htonx(T x) { return x; }

template <typename T>
inline T nms_ntohx(T x) { return x; }

#endif

} // nms





#endif // ifndef MSGLAYERS_HPP_INCLUDED
