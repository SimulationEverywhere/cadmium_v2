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
#include <cadmium/core/modeling/coupled.hpp>
#include "abs_simulator.hpp"
#include "simulator.hpp"

#include "../modeling/atomic.hpp"
#include "../modeling/coupled.hpp"
#include "../../logger/logger.hpp"

namespace cadmium {
    class Coordinator: public AbstractSimulator {
     protected:
        std::vector<std::shared_ptr<AbstractSimulator>> simulators;

		std::shared_ptr<Component> getComponent() override {
			return model;
		}

		void start(double time) override {
			timeLast = time;
			std::for_each(simulators.begin(), simulators.end(), [time](auto& s) { s->start(time); });
		}

		void stop(double time) override {
			timeLast = time;
			std::for_each(simulators.begin(), simulators.end(), [time](auto& s) { s->stop(time); });
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

            logger->logTime(timeNext);
		}

		void clear() override {
			std::for_each(simulators.begin(), simulators.end(), [](auto& s) { s->clear(); });
			getComponent()->clearPorts();
		}

     public:
        std::shared_ptr<cadmium::Coupled> model;

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
        explicit Coordinator(std::shared_ptr<T> model) : Coordinator(model, 0) {}

        template <typename T>
        explicit Coordinator(T model) : Coordinator(std::make_shared<T>(std::move(model)), 0) {}

		virtual void start() {
            model->setUid(0);
			if (logger != nullptr) {
				logger->start();
			}
            logger->logTime(timeLast);
			start(timeLast);
		}

		void stop() {
			stop(timeLast);
			if (logger != nullptr) {
				logger->stop();
			}
		}

        void setLogger(const std::shared_ptr<Logger>& log) override {
			logger = log;
			std::for_each(simulators.begin(), simulators.end(), [log](auto& s) { s->setLogger(log); });
		}

        [[maybe_unused]] void simulate(long nIterations) {
            logger->logTime(timeNext);
            while (nIterations-- > 0 && timeNext < std::numeric_limits<double>::infinity()) {
                timeLast = timeNext;
                collection(timeLast);
                transition(timeLast);
                clear();
            }
		}

        [[maybe_unused]] void simulate(double timeInterval) {
            logger->logTime(timeNext);
            double timeFinal = timeLast + timeInterval;
            while(timeNext < timeFinal) {
                timeLast = timeNext;
                collection(timeLast);
                transition(timeLast);
                clear();
            }
            timeLast = timeFinal;
		}

        [[maybe_unused]] void simulate() {
            simulate(std::numeric_limits<double>::infinity());
        }
    };
}

#endif //_CADMIUM_CORE_SIMULATION_COORDINATOR_HPP_
