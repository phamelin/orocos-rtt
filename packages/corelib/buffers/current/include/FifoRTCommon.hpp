/***************************************************************************
  tag: Peter Soetens  Thu Oct 10 16:16:57 CEST 2002  FifoRTCommon.hpp 

                        FifoRTCommon.hpp -  description
                           -------------------
    begin                : Thu October 10 2002
    copyright            : (C) 2002 Peter Soetens
    email                : peter.soetens@mech.kuleuven.ac.be
 
 ***************************************************************************
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Lesser General Public            *
 *   License as published by the Free Software Foundation; either          *
 *   version 2.1 of the License, or (at your option) any later version.    *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place,                                    *
 *   Suite 330, Boston, MA  02111-1307  USA                                *
 *                                                                         *
 ***************************************************************************/ 
 

#ifndef FIFORTCOMMON_HPP
#define FIFORTCOMMON_HPP

#include "Event.hpp"

namespace ORO_CoreLib
{

    /**
     * Data structure used for signaling when a fifo has new data.
     * @see FifoRTIn
     * @see FifoRTOut
     */
    typedef struct
    {
        Event<void()>* dataReady;
        bool inUse;
    } fifo_info_t;

    const int MAX_NR_FIFOS = 64;

    /**
     * Returns the number of a free Fifo
     *
     * @return the number of a free Fifo
     * @return -1 when there are no free Fifo's left
     */
    int getFreeFifoNr();

}

#endif

