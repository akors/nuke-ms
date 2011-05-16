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

#include "msglayer.hpp"
#include "testutils.hpp"
#include "byteprinter.hpp"


DECLARE_TEST("class StringWrapLayer");


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
    byte_traits::byte_sequence bytewise;
    bytewise.resize(stringwrap_down.size());
    stringwrap_down.fillSerialized(bytewise.begin());

    std::cout<<"Serialized StringwrapLayer (size "<<bytewise.size()<<"):\n"<<
        hexprint(bytewise.begin(), bytewise.end())<<'\n';

    // create a string from this data. It still has to be correct, even after being serialized
    {
        // create raw byte array
        byte_traits::byte_t* str_raw =new byte_traits::byte_t[bytewise.size()];

        // copy data
        std::copy(bytewise.begin(), bytewise.end(), str_raw);

        // create string
        byte_traits::msg_string serialized_string(
            reinterpret_cast<byte_traits::msg_string::value_type*>(str_raw),
            bytewise.size()
        );
        std::cout<<"serialized_string: \""<<serialized_string<<"\"\n";

        TEST_ASSERT(orig_string == serialized_string);

        delete[] str_raw;
    }

    SerializedData ser_data(DataOwnership(), bytewise.begin(), bytewise.size());

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


