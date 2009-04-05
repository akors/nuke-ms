// testownership.cpp

/*
 *   nuke-ms - Nuclear Messaging System
 *   Copyright (C) 2009  Alexander Korsunsky
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

struct A
{
    A()
    { std::cout<<"A was born\n"; }

    ~A()
    { std::cout<<"A has died\n"; }
};

int main()
{
    nuke_ms::MemoryOwnership<boost::shared_ptr<A> > own_a0;

    {
        boost::shared_ptr<A> a_ptr(new A);
        nuke_ms::MemoryOwnership<boost::shared_ptr<A> > own_a1(a_ptr);
        std::cout<<"Holding first ownership.\n";

        {
            nuke_ms::MemoryOwnership<boost::shared_ptr<A> > own_a2 = own_a1;

            {
                nuke_ms::MemoryOwnership<boost::shared_ptr<A> > own_a3 = own_a1;
                std::cout<<"giving away third ownership\n";
            }
            std::cout<<"giving away second ownership\n";
        }

        own_a0 = own_a1; std::cout<<"Assigning ownership.\n";
    }

    std::cout<<"own_a1 and a_ptr out of scope.\n";


    return 0;
}