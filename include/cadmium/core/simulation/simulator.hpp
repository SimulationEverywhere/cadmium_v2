/**
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2021  Román Cárdenas Rodríguez
 * ARSLab - Carleton University
 * GreenLSI - Polytechnic University of Madrid
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

#ifndef _CADMIUM_CORE_SIMULATION_SIMULATOR_HPP_
#define _CADMIUM_CORE_SIMULATION_SIMULATOR_HPP_

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
#endif //_CADMIUM_CORE_SIMULATION_SIMULATOR_HPP_
