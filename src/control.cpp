// control.cpp

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


#include "control.hpp"


void Controllable::setAppControl(AppControl* _app_control)
{
    
    if ( !app_ctrl_set)
    {
        // casting away constness to set the value once and for all
        *const_cast<AppControl**>(&app_control) = _app_control;
            app_ctrl_set = true;
    }
    else        
        // I said only callable ONCE!
        throw std::logic_error("Function is allowed to be called only once.");

}

