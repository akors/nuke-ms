// test_packedstruct-trivial.cpp

/*
 *   nuke-ms - Nuclear Messaging System
 *   Copyright (C) 2012  Alexander Korsunsky
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

#define PACKEDSTRUCT_NONTRIVIAL

#include "testutils.hpp"
#include "bytes.hpp"

DECLARE_TEST("class template PackedStruct");


int main()
{
    struct age {};
    struct weight {};
    struct cupsize {};

    typedef nuke_ms::PackedStruct<
        unsigned short, age,
        double, weight,
        char, cupsize
    > PackedTrivial;

    std::cout<<"Struct has size: "<<sizeof(PackedTrivial)<<'\n';

    // check struct sizes
    TEST_ASSERT(sizeof(PackedTrivial) ==
        sizeof(unsigned short) + sizeof(double) + sizeof(char)
    );

    // write to uninitialized object, check if written correctly
    PackedTrivial trivial_uninit;

    trivial_uninit.get<age>() = 19;
    trivial_uninit.get<weight>() = 61.3;
    trivial_uninit.get<cupsize>() = 'C';

    std::cout<<"Member-assigned object:\n"
        <<"Age: "<<trivial_uninit.get<age>()<<'\n'
        <<"Weight: "<<trivial_uninit.get<weight>()<<'\n'
        <<"Cupsize: "<<trivial_uninit.get<cupsize>()<<'\n'
        <<'\n';

    TEST_ASSERT(trivial_uninit.get<age>() == 19);
    TEST_ASSERT(trivial_uninit.get<weight>()  == 61.3);
    TEST_ASSERT(trivial_uninit.get<cupsize>() == 'C');

    // now check the memory manually
    TEST_ASSERT(*reinterpret_cast<unsigned short*>(&trivial_uninit) == 19);
    TEST_ASSERT(
        *reinterpret_cast<double*>(
            reinterpret_cast<char*>(&trivial_uninit) + sizeof(unsigned short)
        ) == 61.3
    );
    TEST_ASSERT(
        *reinterpret_cast<char*>(
            reinterpret_cast<char*>(&trivial_uninit) + sizeof(unsigned short) + sizeof(double)
        ) == 'C'
    );

    // copy objects with memcopy, assert that they are still the same
    PackedTrivial other_trivial_uninit;
    memcpy(&other_trivial_uninit, &trivial_uninit, sizeof(PackedTrivial));
    TEST_ASSERT(trivial_uninit == other_trivial_uninit);


    // check struct initialization
    PackedTrivial trivial_init{21, 57.4, 'B'};

    std::cout<<"Constructor-initialized object:\n"
        <<"Age: "<<trivial_init.get<age>()<<'\n'
        <<"Weight: "<<trivial_init.get<weight>()<<'\n'
        <<"Cupsize: "<<trivial_init.get<cupsize>()<<'\n'
        <<'\n';

    TEST_ASSERT(trivial_init.get<age>() == 21);
    TEST_ASSERT(trivial_init.get<weight>()  == 57.4);
    TEST_ASSERT(trivial_init.get<cupsize>() == 'B');


    struct MyType {
        MyType() = delete;
        MyType(int x, double z) : _x{x}, _z{z}
        {
            std::cout<<"MyType constructed (this = "<<static_cast<void*>(this)<<") \n";
        }

        int _x;
        double _z;
    };

    struct size {};
    struct myval {};

    typedef nuke_ms::PackedStruct
    <
        std::size_t, size,
        MyType,      myval
    > PackedNonTrivial;


#   ifdef NUKE_MS_TEST_SHOULDBREAK
    // this should not compile, since the MyType default constructor is deleted
    PackedNonTrivial nontrivial;
#   endif

    // note that this is not packed anymore, since MyType is not a packed data
    // structure. However, sizeof() and construction/access must still work.
    PackedNonTrivial nontrivial{33, MyType{12, 99.999}};

    std::cout<<"Constructor-initialized non-trivial object:\n"
        <<"size: "<<nontrivial.get<size>()<<'\n'
        <<"myval._x: "<<nontrivial.get<myval>()._x<<'\n'
        <<"myval._z: "<<nontrivial.get<myval>()._z<<'\n'
        <<'\n';

    TEST_ASSERT(nontrivial.get<size>() == 33);
    TEST_ASSERT(nontrivial.get<myval>()._x  == 12);
    TEST_ASSERT(nontrivial.get<myval>()._z == 99.999);


    return CONCLUDE_TEST();
}
