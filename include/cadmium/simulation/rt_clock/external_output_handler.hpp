#ifndef EXTERNAL_OUTPUT_HANDLER_HPP
#define EXTERNAL_OUTPUT_HANDLER_HPP

#include <string>
#include <utility>
#include <vector>

namespace cadmium {
    

    class outputHandler {
        
        public:
        /**
         * The output handler abstract class. Override this class
         * to enable asynchronous outputs in your model
         */
        outputHandler(){};
        
        /**
         * This method must be overriden to obtain the value from output ports of your top model,
         * parse it, and then send it outsided of the simulation.  
         * 
         * @return nothing. TODO! maybe return error codes to help with simulation?
         */
        virtual void parseOutput() = 0;
    };
}

#endif //EXTERNAL_OUTPUT_HANDLER_HPP