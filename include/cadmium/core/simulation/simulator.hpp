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

#include <memory>
#include <utility>
#include "abs_simulator.hpp"
#include "../exception.hpp"
#include "../logger/logger.hpp"
#include "../modeling/atomic.hpp"

namespace cadmium {
    class Simulator: public AbstractSimulator {
     private:
        std::shared_ptr<AtomicInterface> model;
		std::shared_ptr<Logger> logger, debugLogger;
     public:
        Simulator(std::shared_ptr<AtomicInterface> model, double time): AbstractSimulator(time), model(std::move(model)), logger(), debugLogger() {
			if (this->model == nullptr) {
				throw CadmiumSimulationException("No atomic model provided");
			}
			timeNext = timeLast + this->model->timeAdvance();
        }
        ~Simulator() override = default;

		[[nodiscard]] std::shared_ptr<Component> getComponent() const override {
			return model;
		}

		long setModelId(long next) override {
			modelId = next;
			return next + 1;
		}

		void setLogger(const std::shared_ptr<Logger>& log) override {
			logger = log;
		}

		void setDebugLogger(const std::shared_ptr<Logger>& log) override {
			debugLogger = log;
		}

		void start(double time) override {
			timeLast = time;
			if (logger != nullptr) {
				logger->lock();
				logger->logState(timeLast, modelId, model->getId(), model->logState());
				logger->unlock();
			}
		};

		void stop(double time) override {
			timeLast = time;
			if (logger != nullptr) {
				logger->lock();
				logger->logState(timeLast, modelId, model->getId(), model->logState());
				logger->unlock();
			}
		}

		void collection(double time) override {
			if (time >= timeNext) {
				model->output();
			}
		}

		void transition(double time) override {
			auto inEmpty = model->inEmpty();
			if (inEmpty && time < timeNext) {
				return;
			}
			if (inEmpty) {
				model->internalTransition();
			} else {
				auto e = time - timeLast;
				(time < timeNext) ? model->externalTransition(e) : model->confluentTransition(e);
				if (debugLogger != nullptr) {
					debugLogger->lock();
					for (const auto& inPort: model->getInPorts().getPorts()) {
						for (const auto& msg: inPort->logMessages()) {
							debugLogger->logOutput(time, modelId, model->getId(), inPort->getId(), msg);
						}
					}
					debugLogger->unlock();
				}
			}
			if (logger != nullptr) {
				logger->lock();
				if (time >= timeNext) {
					for (const auto& outPort: model->getOutPorts().getPorts()) {
						for (const auto& msg: outPort->logMessages()) {
							logger->logOutput(time, modelId, model->getId(), outPort->getId(), msg);
						}
					}
				}
				logger->logState(time, modelId, model->getId(), model->logState());
				logger->unlock();
			}
			timeLast = time;
			timeNext = time + model->timeAdvance();
		}

		void clear() override {
			getComponent()->clearPorts();
		}
    };
}

#endif //_CADMIUM_CORE_SIMULATION_SIMULATOR_HPP_
