#ifndef _CADMIUM_SIMULATION_SIMULATOR_HPP_
#define _CADMIUM_SIMULATION_SIMULATOR_HPP_

#include <cadmium/core/modeling/atomic.hpp>
#include <memory>
#include <utility>
#include "abstract_simulator.hpp"

namespace cadmium {
    class Simulator: public AbstractSimulator {
     private:
        std::shared_ptr<AbstractAtomic> model;
     public:
        Simulator(std::shared_ptr<AbstractAtomic> model, double time): AbstractSimulator(time), model(std::move(model)) {
			if (this->model == nullptr) {
				throw std::bad_exception(); // TODO custom exceptions
			}
			timeNext = timeLast + this->model->timeAdvance();
        }
        ~Simulator() override = default;

        std::shared_ptr<Component> getComponent() override {
            return model;
        }

        void collection(double time) override {
            if (time >= timeNext) {
                model->output();
                // call model->logger->log_output()
            }
        }

        void transition(double time) override {
            if (model->inEmpty()) {
                if (time < timeNext) {
                    return;
                }
                // call model->logger->log_output()
                model->internalTransition();
            } else {
                auto e = time - timeLast;
                (time < timeNext) ? model->externalTransition(e) : model->confluentTransition(e);
            }
            // call model->logger->log_state()
            timeLast = time;
            timeNext = time + model->timeAdvance();
        }
    };
}

/*
"parent.parent.model[port]"


model_id; model_name ; model_type
                       "Multiplexer.mux"



time ; model_id ; model_name ; port_name ; data
0    ; 1        ; "mux"      ;            ; "that"
double  string     string    true    "[string]"
*/
#endif //_CADMIUM_SIMULATION_SIMULATOR_HPP_
