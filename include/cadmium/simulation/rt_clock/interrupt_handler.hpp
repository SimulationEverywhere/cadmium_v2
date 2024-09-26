/**
 * Interrupt Component Handler for the real time clock.
 * Copyright (C) 2024  Sasisekhar Mangalam Govind
 * ARSLab - Carleton University
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef INTERRUPT_HANDLER_HPP
#define INTERRUPT_HANDLER_HPP

namespace cadmium {
    
    template<typename decodeType>
    class InterruptHandler {
        
        public:
        InterruptHandler(){};

        virtual bool ISRcb() = 0;
        
        virtual decodeType decodeISR() = 0;
    };
}

#endif