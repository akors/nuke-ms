// testownership.cpp

/*
 *   nuke-ms - Nuclear Messaging System
 *   Copyright (C) 2009, 2011  Alexander Korsunsky
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

#include <iostream>
#include <boost/shared_ptr.hpp>

#include "msglayer.hpp"
#include "testutils.hpp"

DECLARE_TEST("class MemoryOwnership");

struct A
{
    A()
    {
        ++refcount;
        std::cout<<"A was born. this = "<<reinterpret_cast<void*>(this)<<'\n';
    }

    ~A()
    {
        --refcount;
        std::cout<<"A has died. this = "<<reinterpret_cast<void*>(this)<<'\n';
    }

    static inline std::size_t getRefCount() { return refcount; }

private:
    static std::size_t refcount;
};

std::size_t A::refcount = 0;

int main()
{
    nuke_ms::MemoryOwnership<boost::shared_ptr<A> >
    own_a0(boost::shared_ptr<A>(new A));

    TEST_ASSERT(A::getRefCount() == 1);

    {
        boost::shared_ptr<A> a_ptr(new A);
        nuke_ms::MemoryOwnership<boost::shared_ptr<A> > own_a1(a_ptr);

        std::cout<<"Holding first ownership.\n";
        TEST_ASSERT(A::getRefCount() == 2);
        TEST_ASSERT(a_ptr.use_count() == 2);

        {
            nuke_ms::MemoryOwnership<boost::shared_ptr<A> > own_a2 = own_a1;
            TEST_ASSERT(a_ptr.use_count() == 3);

            // no new A should be constructed
            TEST_ASSERT(A::getRefCount() == 2);

            {
                nuke_ms::MemoryOwnership<boost::shared_ptr<A> > own_a3 = own_a1;
                TEST_ASSERT(a_ptr.use_count() == 4);

                std::cout<<"giving away third ownership\n";
                // own_a0 should NOT die when going out of scope
            }
            TEST_ASSERT(a_ptr.use_count() == 3);
            TEST_ASSERT(A::getRefCount() == 2);

            std::cout<<"giving away second ownership\n";
        }
        TEST_ASSERT(a_ptr.use_count() == 2);

        own_a0 = own_a1; std::cout<<"Assigning ownership.\n";
        // a0 should have died now
        TEST_ASSERT(A::getRefCount() == 1);
        TEST_ASSERT(a_ptr.use_count() == 3);
    }

    TEST_ASSERT(A::getRefCount() == 1);
    std::cout<<"own_a1 and a_ptr out of scope.\n";


    return CONCLUDE_TEST();
}
