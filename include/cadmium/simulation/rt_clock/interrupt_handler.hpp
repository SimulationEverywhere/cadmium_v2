/**
 * Interrupt Component Handler for the real time clock.
 * Copyright (C) 2025  Sasisekhar Mangalam Govind
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
    
    template<typename variantType>
    class InterruptHandler {
        
        public:
        /**
         * The interrupt handler abstract class. Override this class
         * to enable asynchronous inputs in yout model
         */
        InterruptHandler(){};

        /**
         * This function must be overriden to return true when an input arrives.
         * This function is called within the clock to check for the arrival of an input.
         * 
         * @return true if an input has arrived
        */
        virtual bool ISRcb() = 0;
        
        /**
         * This method must be overriden to obtain the value of the input, and convert it
         * to a format understandable by your model.
         * 
         * @return std::pair<data to be injected(must be std::variant<>), string id of the input port>
         */
        virtual std::pair<variantType, std::string> decodeISR() = 0;
    };
}

#endif