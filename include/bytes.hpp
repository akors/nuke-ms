// bytes.hpp

/*
 *   nuke-ms - Nuclear Messaging System
 *   Copyright (C) 2008 - 2012  Alexander Korsunsky
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

/** @file bytes.hpp
 * @ingroup common
 * @brief Datatypes and functions for handling bytewise data.
 *
 * This file defines sizes and implementations of binary (bytewise) datatypes
 * used in nuke-ms uniformly across platforms and modules.
 *
 * When encoding data that is to be transmitted over the network, it is
 * neccessary that the encoding is the same on any platform. To ensure this
 * compatibility, any data that will be transmitted shall use the datatype
 * definitions in this file instead of native datatypes.
 * The definitions for bytewise datatypes can be found in the
 * @ref nuke_ms::byte_traits class.
 *
 * To ensure the correct encoding of integer values across platforms with
 * different MSB/LSB encodings, all integers shall be converted using the
 * to_netbo() and to_hostbo() functions before sending or using the integer
 * value.
 *
 * Additional routines for reading and writing raw sequences of bytes to/from
 * POD variables are provided by the functions readbytes() and writebytes().
*/


#ifndef BYTES_HPP_INCLUDED
#define BYTES_HPP_INCLUDED

#include <memory>
#include <algorithm>
#include <string>
#include <vector>
#include <cstdint>

/** General namespace for the Nuclear Messaging System project */
namespace nuke_ms
{


/** @addtogroup common nuke-ms common routines and types
 * @{
 */


/** Traits class for byte types used by nuke-ms */
struct byte_traits
{
    /** The type of the smallest adressable data unit in memory */
    typedef std::uint_least8_t byte_t;

    /** The type of an unsigned integer with the width of two bytes*/
    typedef std::uint_least16_t uint2b_t;

    /** The type of a signed integer with the width of two bytes*/
    typedef std::int_least16_t int2b_t;

    /** The type of an unsigned integer with the width of four bytes*/
    typedef std::uint_least32_t uint4b_t;

    /** The type of a signed integer with the width of four bytes*/
    typedef std::int_least32_t int4b_t;

    /** The type for a sequence of bytes */
    typedef std::vector<byte_t> byte_sequence;

    /** Strings for text that stays on this machine */
    typedef std::string native_string;

    /** Strings that should be transmitted over the network */
    typedef std::basic_string<char, std::char_traits<char> > msg_string;
};

/** Reverse the bytes of a POD variable.
* @tparam T A POD variable.
* @param x The value whos bytes you want to get reversed
* @return The value with returned bytes.
*/
template <typename T>
inline T reversebytes(T x)
{
    std::reverse(
        reinterpret_cast<byte_traits::byte_t*>(&x),
        reinterpret_cast<byte_traits::byte_t*>(&x) + sizeof(x)
    );

    return x;
}

// Specializations for certain integer types
/// @cond TEMPLATE_SPECIALIZATIONS

// specialization for a two byte value,
// hoping this will be faster than a std::reverse<>()
template<>
inline byte_traits::uint2b_t reversebytes(byte_traits::uint2b_t x)
{
    return (x << 8) | (x >> 8);
}

// specialization for a two byte value,
// hoping this will be faster than a std::reverse<>()
template<>
inline byte_traits::uint4b_t reversebytes(byte_traits::uint4b_t x)
{
    return (x << 24) | ((x << 8) & 0xff0000) | ((x >> 8) & 0xff00) | (x >> 24);
}

// for signed types call unsigned version
template<>
inline byte_traits::int2b_t reversebytes(byte_traits::int2b_t x)
{ return reversebytes<byte_traits::uint2b_t>(x); }

// for signed types call unsigned version
template<>
inline byte_traits::int4b_t reversebytes(byte_traits::int4b_t x)
{ return reversebytes<byte_traits::uint4b_t>(x); }

/// @endcond



/** Write a value into a byte sequence container of any kind.
* @tparam T The type of the value you want to write. Most useful when POD.
* @tparam ByteSequenceIterator Type of the iterator to the byte sequence. Must
* meet the requirement of OutputIterator. The dereference of this iterator must
* be able to be assigned to a byte_traits::byte_t value.
*
* @param it Iterator to the byte sequence
* @param value Value to be written to the byte sequence
* @return Returns it + sizeof(T)
*/
template <typename T, typename ByteSequenceIterator> inline
ByteSequenceIterator
writebytes(ByteSequenceIterator it, T value)
{
    return std::copy(
        reinterpret_cast<byte_traits::byte_t*>(&value),
        reinterpret_cast<byte_traits::byte_t*>(&value) + sizeof(value),
        it
    );
}

/** Read bytes from a byte sequence into a POD variable
* @tparam T The type of the value. POD is required.
* @tparam ByteSequenceIterator Iterator to the byte sequence. Must meet
* InputIterator requirement. The dereference of this iterator must be
* POD and be one byte of size.
*
* @param val_ptr Pointer to the value that will be written to.
* @param it Iterator to the sequence that contains the bytes for the value
* @return Returns it + sizeof(T)
*/
template <typename T, typename ByteSequenceIterator> inline
ByteSequenceIterator readbytes(T* val_ptr, ByteSequenceIterator it)
{
    std::copy(
        it,
        it + sizeof(T),
        reinterpret_cast<byte_traits::byte_t*>(val_ptr)
    );

    return it + sizeof(T);
}

/// @cond TEMPLATE_SPECIALIZATIONS

template <typename ByteSequenceIterator> inline
ByteSequenceIterator readbytes(
    byte_traits::byte_t* val_ptr, ByteSequenceIterator it)
{ *val_ptr = *it; return ++it; }

/// @endcond


/** Create an std::shared_ptr for an array.
* @tparam T Type of the array element
* @tparam N Size of the array
*
* This creates a shared pointer to an array. It behaves like a normal
* shared_ptr, with the exception that the array is correctly deleted with
* delete[].
*
* @return The shared array
*/
template <typename T, std::size_t N> inline
std::shared_ptr<T> make_shared_array()
{
    return std::shared_ptr<T>(new T[N], [](T* p){ delete[] p;} );
}


#ifdef NUKE_MS_BIG_ENDIAN

/** Convert integer to network byte order
*
* Beware! As opposed to common understanding of the term "network byte order",
* the network byte order of nuke-ms is Little Endian, that means least
* significant byte first!
*
* This function does nothing (returns the value passed) on a Little Endian
* system and inverts the byte order on a Big Endian system.
*
* @tparam T The type of the integer
* @param x Integer in host byte order to be converted
* @returns Converted integer in network byte order
*/
template <typename T>
inline T to_netbo(T x) { return reversebytes(x); }

/** Convert integer to host byte order
*
* Beware! As opposed to common understanding of the term "network byte order",
* the network byte order of nuke-ms is Little Endian, that means least
* significant byte first!
*
* This function does nothing (returns the value passed) on a Little Endian
* system and inverts the byte order on a Big Endian system.
*
* @tparam T The type of the integer
* @param x Integer in network byte order to be converted
* @returns Converted integer in host byte order
*/
template <typename T>
inline T to_hostbo(T x) { return reversebytes(x); }


#else

/** Convert integer to network byte order
*
* Beware! As opposed to common understanding of the term "network byte order",
* the network byte order of nuke-ms is Little Endian, that means least
* significant byte first!
*
* This function does nothing (returns the value passed) on a Little Endian
* system and inverts the byte order on a Big Endian system.
*
* @tparam T The type of the integer
* @param x Integer in host byte order to be converted
* @returns Converted integer in network byte order
*/
template <typename T>
inline T to_netbo(T x) { return x; }

/** Convert integer to host byte order
*
* Beware! As opposed to common understanding of the term "network byte order",
* the network byte order of nuke-ms is Little Endian, that means least
* significant byte first!
*
* This function does nothing (returns the value passed) on a Little Endian
* system and inverts the byte order on a Big Endian system.
*
* @tparam T The type of the integer
* @param x Integer in network byte order to be converted
* @returns Converted integer in host byte order
*/
template <typename T>
inline T to_hostbo(T x) { return x; }

#endif

namespace packedstruct_detail {

template <
    std::size_t Offset,
    typename VarType, typename NameTag,
    typename... Rest
>
struct PackedStructMember
{
    static constexpr std::size_t offset = Offset;
    typedef VarType vartype;
    typedef NameTag nametag;

    typedef PackedStructMember<offset + sizeof(vartype), Rest...> next;
    static constexpr std::size_t list_size = sizeof(vartype) + next::list_size;

    static vartype& access(char *base)
    {
        return *reinterpret_cast<vartype*>(base+offset);
    }

#ifdef PACKEDSTRUCT_NONTRIVIAL
    static void construct(void* member_ptr)
    {
        // use placement new to initialize
        new (member_ptr) vartype{};

        // defer to next element initialization
        next::construct(static_cast<vartype*>(member_ptr) + 1);
    }

    template <typename ArgumentType, typename... ArgTypeRest>
    static void construct(
        void* member_ptr,
        ArgumentType&& arg,
        ArgTypeRest... args_rest
    )
    {
        // use placement new to initialize, forward arguments
        new (member_ptr) vartype(std::forward<ArgumentType>(arg));

        next::construct(
            static_cast<vartype*>(member_ptr) + 1,
            std::forward<ArgTypeRest>(args_rest)...
        );
    }

    static void destruct(void* member_ptr)
    {
        // call destructor explicitly
        static_cast<vartype*>(member_ptr)->~vartype();

        // call next node with pointer after the current element
        next::destruct(static_cast<vartype*>(member_ptr) + 1);
    }
#endif
};

template <std::size_t Offset, typename VarType, typename NameTag>
struct PackedStructMember<Offset, VarType, NameTag>
{
    static constexpr std::size_t offset = Offset;
    typedef VarType vartype;
    typedef NameTag nametag;

    // we are at the last member, the size of the list is just the size of our
    // variable type
    static constexpr std::size_t list_size = sizeof(vartype);

    static vartype& access(char *base)
    {
        return *reinterpret_cast<vartype*>(base+offset);
    }
#ifdef PACKEDSTRUCT_NONTRIVIAL
    static void construct(void* member_ptr)
    {
        // use placement new to initialize
        new (member_ptr) vartype{};
    }

    template <typename ArgumentType>
    static void construct(void* member_ptr, ArgumentType&& arg)
    {
        // use placement new to initialize, forward arguments
        new (member_ptr) vartype(std::forward<ArgumentType>(arg));
    }

    static void destruct(void* member_ptr)
    {
        // call destructor explicitly
        static_cast<vartype*>(member_ptr)->~vartype();
    }
#endif
};

// Search for a member by name tag.
template <typename MemberList, typename NameTag>
struct FindMemberByNameTag
{
    // this means we don't have it, ask the next one
    typedef typename FindMemberByNameTag<typename MemberList::next, NameTag>::member
        member;
};

// specialization, if the searched name tag is our name tag
template <typename MemberList>
struct FindMemberByNameTag<MemberList, typename MemberList::nametag>
{
    typedef MemberList member;
};



template <typename... Members>
class PackedStruct {
    static_assert(sizeof...(Members) % 2 == 0,
        "Members must be added in pairs of Variable-Type / Name-Tag"
    );

    typedef PackedStructMember<0, Members...> MemberList;

    char data[MemberList::list_size];

public:
#ifdef PACKEDSTRUCT_NONTRIVIAL
    PackedStruct()
    {
        MemberList::construct(data);
    }

    template <typename... ArgTypes>
    explicit PackedStruct(ArgTypes&&... args)
    {
        static_assert(
            sizeof...(ArgTypes) == sizeof...(Members) / 2,
            "Number of constructor arguments does not match number of struct members."
        );

        MemberList::construct(data, std::forward<ArgTypes>(args)...);
    }


    ~PackedStruct()
    {
        MemberList::destruct(data);
    }
#endif

    template <typename NameTag>
    typename FindMemberByNameTag<MemberList, NameTag>::member::vartype& get()
    {
        return FindMemberByNameTag<MemberList, NameTag>::member::access(data);
    }
};

} // namespace packedstruct_detail

using packedstruct_detail::PackedStruct;

/**@}*/ // addtogroup common


} // namespace nuke_ms





#endif // ifndef MSGLAYERS_HPP_INCLUDED
