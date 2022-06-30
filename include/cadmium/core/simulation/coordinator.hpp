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

#ifndef CADMIUM_CORE_SIMULATION_COORDINATOR_HPP_
#define CADMIUM_CORE_SIMULATION_COORDINATOR_HPP_

#include <memory>
#include <utility>
#include <vector>
#include "abs_simulator.hpp"
#include "simulator.hpp"
#include "../modeling/atomic.hpp"
#include "../modeling/coupled.hpp"
#include "../modeling/component.hpp"

namespace cadmium {
    class Coordinator: public AbstractSimulator {
     private:
        std::shared_ptr<Coupled> model;
        std::vector<std::shared_ptr<AbstractSimulator>> simulators;
	 public:
        Coordinator(std::shared_ptr<Coupled> model, double time): AbstractSimulator(time), model(std::move(model)) {
			if (this->model == nullptr) {
				throw CadmiumSimulationException("No coupled model provided");
			}
			timeLast = time;
			for (auto& component: this->model->getComponents()) {
				std::shared_ptr<AbstractSimulator> simulator;
				auto coupled = std::dynamic_pointer_cast<Coupled>(component);
				if (coupled != nullptr) {
					simulator = std::make_shared<Coordinator>(coupled, time);
				} else {
					auto atomic = std::dynamic_pointer_cast<AtomicInterface>(component);
					if (atomic == nullptr) {
						throw CadmiumSimulationException("Component is not a coupled nor atomic model");
					}
					simulator = std::make_shared<Simulator>(atomic, time);
				}
				simulators.push_back(simulator);
				timeNext = std::min(timeNext, simulator->getTimeNext());
			}
		}

		[[nodiscard]] std::shared_ptr<Component> getComponent() const override {
			return model;
		}

		long setModelId(long next) override {
			modelId = next++;
			for (auto& simulator: simulators) {
				next = simulator->setModelId(next);
			}
			return next;
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
				std::for_each(model->getICs().begin(), model->getICs().end(), [](auto& s) {std::get<1>(s)->propagate(std::get<0>(s)); });
				std::for_each(model->getEOCs().begin(), model->getEOCs().end(), [](auto& s) {std::get<1>(s)->propagate(std::get<0>(s)); });
			}
		}

		void transition(double time) override {
			std::for_each(model->getEICs().begin(), model->getEICs().end(), [](auto& s) {std::get<1>(s)->propagate(std::get<0>(s)); });
			timeLast = time;
			timeNext = std::numeric_limits<double>::infinity();
			for (auto& simulator: simulators) {
				simulator->transition(time);
				timeNext = std::min(timeNext, simulator->getTimeNext());
			}
		}

		void clear() override {
			std::for_each(simulators.begin(), simulators.end(), [](auto& s) { s->clear(); });
			model->clearPorts();
		}

		template <typename T>
		void inject(double e, std::shared_ptr<Port<T>> port, T value) {
			auto time = timeLast + e;
			if (time <= timeNext) {
				port->addMessage(value);
				timeLast = time;
				transition(time);
				clear();
			}
			else {
				throw CadmiumSimulationException("The lapsed time is too long for injecting a message");
			}
		}

		void start() {
			setModelId(0);
			start(timeLast);
		}

		void stop() {
			stop(timeLast);
		}

		void setDebugLogger(const std::shared_ptr<Logger>& log) override {
			std::for_each(simulators.begin(), simulators.end(), [log](auto& s) { s->setDebugLogger(log); });
        }

		void setLogger(const std::shared_ptr<Logger>& log) override {
			std::for_each(simulators.begin(), simulators.end(), [log](auto& s) { s->setLogger(log); });
		}
    };
}

#endif //CADMIUM_CORE_SIMULATION_COORDINATOR_HPP_
