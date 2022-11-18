#ifndef CADMIUM_EXAMPLE_DEVSTONE_GENERATOR_HPP_
#define CADMIUM_EXAMPLE_DEVSTONE_GENERATOR_HPP_

#include <cadmium/core/modeling/atomic.hpp>
#include <iostream>

namespace cadmium::example::devstone {
    //! DEVStone generator DEVS model
    class DEVStoneGenerator: public Atomic<double> {
     private:
     public:
        Port<int> out;  //!< Output Port.

        /**
         * Constructor function of the atomic DEVStone model.
         * @param id ID of the atomic DEVStone model.
         */
        DEVStoneGenerator();

        /**
         * It runs the Dhrystone benchmark during DEVStoneAtomic::intDelay milliseconds and passivates.
         * @param s reference to the model state.
         */
        void internalTransition(double& s) const override;

        /**
         * It runs the Dhrystone benchmark during DEVStoneAtomic::extDelay milliseconds and activates.
         * @param s reference to the current model state.
         * @param e time elapsed since the last state transition function was triggered.
         * @param x reference to the atomic model input port set.
         */
        void externalTransition(double& s, double e) const override;

        /**
         * It outputs a new event via the DEVStoneAtomic::out Port.
         * @param s reference to the current model state.
         * @param y reference to the model output port set.
         */
        void output(const double& s) const override;

        /**
         * It returns DEVStoneState::sigma.
         * @param s reference to the current model state.
         * @return the value of the state sigma.
         */
        [[nodiscard]] double timeAdvance(const double& s) const override;
    };
}  //namespace cadmium::example::devstone

#endif //CADMIUM_EXAMPLE_DEVSTONE_GENERATOR_HPP_
