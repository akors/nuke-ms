// test_neartypes.cpp

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
 *   GNU Genera l Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "neartypes.hpp"
#include "msglayer.hpp"

#include "testutils.hpp"

DECLARE_TEST("class NearUserMessage")

using namespace nuke_ms;

int main()
{
    const char message_string_array[] = "With love";
    std::string message_string(message_string_array);

    {

    // check constructors for "move-ness", assert all messages have equal string value
    StringwrapLayer sw{message_string};
    NearUserMessage num1{message_string};
    NearUserMessage num2{StringwrapLayer{message_string}};
    NearUserMessage num3{sw};

    TEST_ASSERT(
        num1._stringwrap._message_string == num2._stringwrap._message_string
        && num1._stringwrap._message_string == num3._stringwrap._message_string
        && num2._stringwrap._message_string == num3._stringwrap._message_string
    );
    }

    // serialize down to the network
    long long int f = 0x656d206d6f7266ll, t = 0x756f79206f74ll;
    UniqueUserID recipient(reinterpret_cast<byte_traits::byte_t*>(&t));
    UniqueUserID sender(reinterpret_cast<byte_traits::byte_t*>(&f));

    NearUserMessage down{
        message_string,
        recipient,
        sender,
        NearUserMessage::msg_id_t{0xF0}
    };

    std::vector<byte_traits::byte_t> bytes(down.size());
    down.fillSerialized(bytes.begin());


    SerializedData serdat{{}, bytes.begin(), bytes.size()};

    bool no_exception_thrown = false;
    try
    {
        NearUserMessage up{serdat};

        TEST_ASSERT(up._stringwrap._message_string == message_string);
        TEST_ASSERT(up._recipient == recipient);
        TEST_ASSERT(up._sender == sender);
        TEST_ASSERT(up._msg_id == NearUserMessage::msg_id_t{0xF0});

        std::cout<<"Message string:\n"<<
            up._stringwrap._message_string<<", "<<
            reinterpret_cast<char*>(&up._sender.id)<<' '<<
            reinterpret_cast<char*>(&up._recipient.id)<<std::endl;

        no_exception_thrown = true;
    }
    catch(const std::exception& e)
    {
        std::cerr<<"Caught exception "<<e.what()<<'\n';
        TEST_ASSERT(no_exception_thrown);
    }

    return CONCLUDE_TEST();
}
