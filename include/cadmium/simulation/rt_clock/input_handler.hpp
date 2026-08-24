#ifndef INPUT_HANDLER_HPP
#define INPUT_HANDLER_HPP

namespace cadmium {
    

    class InputHandler {
        
        public:
        /**
         * The input handler abstract class. Override this class
         * to enable asynchronous inputs in your model
         */
        InputHandler(){};

        /**
         * This function must be overriden to return true when an input arrives.
         * This function is called within the clock to check for the arrival of an input.
         * 
         * @return true if an input has arrived
        */
        virtual bool ISRcb() = 0;
        
        /**
         * This method must be overriden to obtain the value of the input, and convert it
         * to the input port's message type, and inject it into the correct port.
         * 
         * @return 
         */
        virtual void decodeISR() = 0;
    };
}

#endif