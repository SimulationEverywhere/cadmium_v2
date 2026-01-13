/**
 * Interrupt Component Handler for the real time clock.
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2025-present Sasisekhar Mangalam Govind
 * ARSLab - Carleton University
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