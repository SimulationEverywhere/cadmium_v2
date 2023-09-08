#ifndef BLINKY_SYSTEM_HPP
#define BLINKY_SYSTEM_HPP

#include <cadmium/modeling/devs/coupled.hpp>
#ifdef EMBED
    #include "digitalInput.hpp"
    #include "digitalOutput.hpp"
#else
    #include <cadmium/lib/iestream.hpp>
#endif
#include "blinky.hpp"

namespace cadmium::blinkySystem {
    class blinkySystem : public Coupled {
        public:
        blinkySystem(const std::string& id): Coupled(id){
            auto blinky = addComponent<Blinky>("blinky");
        #ifdef EMBED
            auto digitalInput = addComponent<DigitalInput>("digitalInput",GPIO_PORT_P5,GPIO_PIN1);
            auto digitalOutput = addComponent<DigitalOutput>("digitalOutput",GPIO_PORT_P2,GPIO_PIN6);
            addCoupling(digitalInput->out,blinky->in);
            addCoupling(blinky->out,digitalOutput->in);
        #else
            auto textInput = addComponent<cadmium::lib::IEStream<bool>>("textInput","input.txt");
            addCoupling(textInput->out,blinky->in);
        #endif
        }
    };
} // namespace cadmium::blinkySystem

#endif // BLINKY_SYSTEM_HPP
