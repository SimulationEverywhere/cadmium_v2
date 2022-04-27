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

#ifndef _CADMIUM_CORE_SIMULATION_HIERARCHICAL_SIMULATOR_HPP_
#define _CADMIUM_CORE_SIMULATION_HIERARCHICAL_SIMULATOR_HPP_

#include <memory>
#include <utility>
#include "abs_simulator.hpp"
#include "../logger/logger.hpp"
#include "../modeling/atomic.hpp"

namespace cadmium {
    class Simulator: {
     private:
        std::shared_ptr<AbstractAtomic> model;

		std::shared_ptr<Component> getComponent() override {
			return model;
		}

		long setModelId(long next) override {
			modelId = next++;
			return next;
		}

		void setLogger(const std::shared_ptr<Logger>& log) override {
			logger = log;
		}

		void start(double time) override {
			timeLast = time;
			if (logger != nullptr) {
				logger->lock();
				model->logState(logger, timeLast, modelId);
				logger->unlock();
			}
		};

		void stop(double time) override {
			timeLast = time;
			if (logger != nullptr) {
				logger->lock();
				model->logState(logger, timeLast, modelId);
				logger->unlock();
			}
		}

		void execute_output_function (double time) {
			if (time >= timeNext) {
				model->output();
			}
		}

		void execute_state_transition(double time) {
			auto inEmpty = model->inEmpty();
			if (inEmpty && time < timeNext) {
				return;
			}
			if (inEmpty) {
				model->internalTransition();
			} else {
				auto e = time - timeLast;
				(time < timeNext) ? model->externalTransition(e) : model->confluentTransition(e);
			}
			if (logger != nullptr) {
				logger->lock();
				if (time >= timeNext) {
					model->interface->outPorts.logMessages(logger, time, modelId, model->getId());
				}
				model->logState(logger, time, modelId);
				logger->unlock();
			}
			timeLast = time;
			timeNext = time + model->timeAdvance();
		}

		void clear() override {
			getComponent()->clearPorts();
		}

     public:
        Simulator(std::shared_ptr<AbstractAtomic> model, double time): AbstractSimulator(time), model(std::move(model)) {
			if (this->model == nullptr) {
				throw std::bad_exception(); // TODO custom exceptions
			}
			timeNext = timeLast + this->model->timeAdvance();
        }
        ~Simulator() override = default;
    };
}

#endif //_CADMIUM_CORE_SIMULATION_SIMULATOR_HPP_
