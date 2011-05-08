// boostasio_impl.cpp

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

#include <boost/version.hpp>

#if (BOOST_VERSION == 104400)
#	include <boost/asio/impl/src.cpp>
#elif (BOOST_VERSION >= 104500)
#	include <boost/asio/impl/src.hpp>
#endif // if (BOOST_VERSION == 104400)
