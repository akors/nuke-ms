// protocol.hpp

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


/** @defgroup proto Communication Protocl */


#ifndef PROTOCOL_HPP_
#define PROTOCOL_HPP_



#include "control.hpp"

class NMSProtocol : public AbstractProtocol
{
    bool connected;
    
 
    /** Connect to a remote site.
     * @param id The string representation of the address of the remote site
     */
    virtual void connect_to(const std::wstring& id);
    
    /* Send message to connected remote site.
     * @param msg The message you want to send
     */
    virtual void send(const std::wstring& id);
    
    /** Disconnect from the remote site.
     * 
     */
    virtual void disconnect();
    
    
    virtual bool is_connected();   
    
    
public:

    NMSProtocol()
        : connected(true)
    {
        
    }
};



#endif /*PROTOCOL_HPP_*/
