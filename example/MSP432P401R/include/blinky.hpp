#ifndef BLINKY_HPP
#define BLINKY_HPP

#include <cadmium/modeling/devs/atomic.hpp>
#ifndef NO_LOGGING
    #include <iostream>
#endif

namespace cadmium::blinkySystem {
    struct BlinkyState {
        double sigma;
        bool lightOn;
        bool fastToggle;
        /**
         * Constructor
         */
        BlinkyState(): sigma(0), lightOn(false), fastToggle(false)  {}
    };
#if !defined NO_LOGGING || !defined EMBED 
    /**
     * Insertion operator for BlinkyState objects
     * @param out output stream.
     * @param s state to be represented in the output stream.
     * @return output stream
     */
    std::ostream& operator<<(std::ostream &out, const BlinkyState& state) {
        out << "Status:, " << state.lightOn << ", sigma: " << state.sigma;
        return out;
    }
#endif
    class Blinky : public Atomic<BlinkyState> {
      public:
        Port<bool> out;
        Port<bool> in;
        double slowToggleTime;
        double fastToggleTime;

        // constructor
        Blinky(const std::string& id): Atomic<BlinkyState>(id, BlinkyState()) {
            out = addOutPort<bool>("out");
            in  = addInPort<bool>("in");
            slowToggleTime = 3.0;
            fastToggleTime = 0.75;
            state.sigma = slowToggleTime;
        }

        // internal transition
        void internalTransition(BlinkyState& state) const override {
            state.lightOn = !state.lightOn;
        }

        // external transition
        void externalTransition(BlinkyState& state, double e) const override {

            if(!in->empty()){
                for( const auto x : in->getBag()){
                    if (x==0)
                        state.fastToggle = !state.fastToggle;
                }

                if(state.fastToggle)
                    state.sigma = fastToggleTime;
                else
                    state.sigma = slowToggleTime;
            }
        }

        // output
        void output(const BlinkyState& state) const override {
            out->addMessage(state.lightOn);
        }

        // time advance
        [[nodiscard]] double timeAdvance(const BlinkyState& state) const override {
            return state.sigma;
        }
    };
} // namespace cadmium::blinkySystem

#endif // BLINKY_HPP
