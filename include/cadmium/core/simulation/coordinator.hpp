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

#ifndef _CADMIUM_CORE_SIMULATION_COORDINATOR_HPP_
#define _CADMIUM_CORE_SIMULATION_COORDINATOR_HPP_

#include <memory>
#include <utility>
#include <vector>
#include "abs_simulator.hpp"
#include "simulator.hpp"
#include "../modeling/atomic.hpp"
#include "../modeling/coupled.hpp"

namespace cadmium {
    class Coordinator: public AbstractSimulator {
     private:
        std::shared_ptr<Coupled> model;
        std::vector<std::shared_ptr<AbstractSimulator>> simulators;
     public:
        Coordinator(std::shared_ptr<Coupled> model, double time): AbstractSimulator(time), model(std::move(model)) {
			if (this->model == nullptr) {
				throw std::bad_exception();  // TODO custom exceptions
			}
			for (auto& component: this->model->components) {
				std::shared_ptr<AbstractSimulator> simulator;
				auto coupled = std::dynamic_pointer_cast<Coupled>(component);
				if (coupled != nullptr) {
					simulator = std::make_shared<Coordinator>(coupled, time);
				} else {
					auto atomic = std::dynamic_pointer_cast<AbstractAtomic>(component);
					if (atomic == nullptr) {
						throw std::bad_exception();  // TODO custom exceptions
					}
					simulator = std::make_shared<Simulator>(atomic, time);
				}
				simulators.push_back(simulator);
				timeNext = std::min(timeNext, simulator->timeNext);
			}
		}
		template <typename T>
		explicit Coordinator(T model) : Coordinator(std::make_shared<T>(std::move(model)), 0) {}

		void start() override {
			this->setModelId(0);
			if (logger != nullptr) {
				logger->start();
			}
		}

		void stop() override {
			if (logger != nullptr) {
				logger->stop();
			}
		}

		long setModelId(long next) override {
			next = AbstractSimulator::setModelId(next);
			for (auto& simulator: simulators) {
				next = simulator->setModelId(next);
			}
			return next;
		}

		void setLogger(const std::shared_ptr<Logger>& l) override {
			AbstractSimulator::setLogger(l);
			std::for_each(simulators.begin(), simulators.end(), [l](auto& s) { s->setLogger(l); });
		}

        std::shared_ptr<Component> getComponent() override {
            return model;
        }

        void collection(double time) override {
            if (time >= timeNext) {
                std::for_each(simulators.begin(), simulators.end(), [time](auto& s) { s->collection(time); });
                std::for_each(model->IC.begin(), model->IC.end(), [](auto& s) {std::get<1>(s)->propagate(std::get<0>(s)); });
                std::for_each(model->EOC.begin(), model->EOC.end(), [](auto& s) {std::get<1>(s)->propagate(std::get<0>(s)); });
            }
        }

        void transition(double time) override {
            std::for_each(model->EIC.begin(), model->EIC.end(), [](auto& s) {std::get<1>(s)->propagate(std::get<0>(s)); });
            timeNext = std::numeric_limits<double>::infinity();
            for (auto& simulator: simulators) {
                simulator->transition(time);
                timeNext = std::min(timeNext, simulator->timeNext);
            }
        }

        void clear() override {
            std::for_each(simulators.begin(), simulators.end(), [](auto& s) { s->clear(); });
            AbstractSimulator::clear();
        }

        void simulate(long nIterations) {
            while (nIterations-- > 0 && timeNext < std::numeric_limits<double>::infinity()) {
                timeLast = timeNext;
                collection(timeLast);
                transition(timeLast);
                clear();
            }
        }

        void simulate(double timeInterval) {
            double timeFinal = timeLast + timeInterval;
            while(timeNext < timeFinal) {
                timeLast = timeNext;
                collection(timeLast);
                transition(timeLast);
                clear();
            }
            timeLast = timeFinal;
        }
    };
}

#endif //_CADMIUM_CORE_SIMULATION_COORDINATOR_HPP_
