// test_stringwraplayer.cpp

/*
 *   nuke-ms - Nuclear Messaging System
 *   Copyright (C) 2011  Alexander Korsunsky
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




#include <iostream>
#include <utility>
#include <vector>
#include <list>
#include <deque>

#include "msglayer.hpp"
#include "testutils.hpp"
#include "byteprinter.hpp"


DECLARE_TEST("class StringWrapLayer");


struct ConvertibleToInt {
    int value;
    ConvertibleToInt() {}
    ConvertibleToInt(int _value) : value(_value) {}
    operator int () { return value; }
};

using namespace nuke_ms;

int main()
{

    byte_traits::msg_string orig_string("This is a narrow char string");
    std::cout<<"Original string (length "<<orig_string.length()<<"): \""<<
        printbytes(orig_string.begin(), orig_string.end())<<"\"\n";

    // create stringwrap from string
    StringwrapLayer stringwrap_down(orig_string);

    TEST_ASSERT(stringwrap_down.getString() == orig_string);


    // check string length
    std::cout<<"stringwrap_down.size() == "<<stringwrap_down.size()<<"\n";
    TEST_ASSERT(stringwrap_down.size() == orig_string.length() *
        sizeof(byte_traits::msg_string::value_type));
    std::cout<<std::endl;


    // get serialized version
    byte_traits::byte_sequence bytewise(stringwrap_down.size());
    stringwrap_down.fillSerialized(bytewise.begin());

    std::cout<<"Serialized StringwrapLayer (size "<<bytewise.size()<<"):\n"<<
        hexprint(bytewise.begin(), bytewise.end())<<'\n';

    // create a string from this data.
    // It still has to be correct, even after being serialized
    {
        // create raw byte array
        byte_traits::byte_t* str_raw = new byte_traits::byte_t[bytewise.size()];

        // copy data
        std::copy(bytewise.begin(), bytewise.end(), str_raw);

        // create string
        byte_traits::msg_string serialized_string(
            reinterpret_cast<byte_traits::msg_string::value_type*>(str_raw),
            bytewise.size() / sizeof(byte_traits::msg_string::value_type)
        );
        std::cout<<"serialized_string: \""<<serialized_string<<"\"\n";

        TEST_ASSERT(orig_string == serialized_string);

        delete[] str_raw;
    }

    // test fillSerialized function with vector of char iterators
    std::vector<char> char_vec(stringwrap_down.size());
    stringwrap_down.fillSerialized(char_vec.begin());
    TEST_ASSERT(std::equal(bytewise.begin(), bytewise.end(), char_vec.begin()));

    // test fillSerialized function with list of int iterators
    std::list<int> int_list(stringwrap_down.size());
    stringwrap_down.fillSerialized(int_list.begin());
    TEST_ASSERT(std::equal(bytewise.begin(), bytewise.end(), int_list.begin()));

    // test fillSerialized function with deque of ConvertibleToInt iterators
    std::deque<ConvertibleToInt> convint_list(stringwrap_down.size());
    stringwrap_down.fillSerialized(convint_list.begin());
    TEST_ASSERT(std::equal(bytewise.begin(), bytewise.end(), convint_list.begin()));


    SerializedData ser_data({}, bytewise.begin(), bytewise.size());

    try
    {
        StringwrapLayer stringwrap_up(ser_data);

        std::cout<<"\nResulting String: \""<<stringwrap_up.getString()<<"\"\n";
        TEST_ASSERT(stringwrap_up.getString() == orig_string);
    }
    catch(...)
    {
        bool exception_was_thrown = true;
        TEST_ASSERT(!exception_was_thrown);
    }

    return CONCLUDE_TEST();
}


