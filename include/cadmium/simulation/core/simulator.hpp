/**
 * DEVS simulator.
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

#ifndef CADMIUM_SIMULATION_CORE_SIMULATOR_HPP_
#define CADMIUM_SIMULATION_CORE_SIMULATOR_HPP_

#include <memory>
#include <utility>
#include "abs_simulator.hpp"
#include "../../exception.hpp"
#ifndef NO_LOGGING
    #include "../logger/logger.hpp"
#endif
#include "../../modeling/devs/atomic.hpp"

namespace cadmium {
    //! DEVS simulator.
    class Simulator: public AbstractSimulator {
     private:
        std::shared_ptr<AtomicInterface> model;  //!< Pointer to the corresponding atomic DEVS model.
    #ifndef NO_LOGGING
        std::shared_ptr<Logger> logger;
    #endif
     public:
    #ifndef NO_LOGGING
        /**
         * Constructor function.
         * @param model pointer to the atomic model.
         * @param time initial simulation time.
         */
        Simulator(std::shared_ptr<AtomicInterface> model, double time): AbstractSimulator(time), model(std::move(model)), logger() {
            if (this->model == nullptr) {
                throw CadmiumSimulationException("no atomic model provided");
            }
            timeNext = timeLast + this->model->timeAdvance();
        }
    #else
        /**
         * Constructor function.
         * @param model pointer to the atomic model.
         * @param time initial simulation time.
         */
        Simulator(std::shared_ptr<AtomicInterface> model, double time): AbstractSimulator(time), model(std::move(model)) {
            if (this->model == nullptr) {
                throw CadmiumSimulationException("no atomic model provided");
            }
            timeNext = timeLast + this->model->timeAdvance();
        }
    #endif

        //! @return pointer to the corresponding atomic DEVS model.
        [[nodiscard]] std::shared_ptr<Component> getComponent() const override {
            return model;
        }

        /**
         * It sets the model ID of the simulator
         * @param next  number of the model ID.
         * @return returns next + 1.
         */
        long setModelId(long next) override {
            modelId = next;
            return next + 1;
        }

    #ifndef NO_LOGGING
        /**
         * Sets a new logger.
         * @param log pointer to the logger.
         */
        void setLogger(const std::shared_ptr<Logger>& newLogger) override {
            logger = newLogger;
        }
    #endif

        /**
         * It performs all the operations before running a simulation.
         * @param time initial simulation time.
         */
        void start(double time) override {
            timeLast = time;
        #ifndef NO_LOGGING
            if (logger != nullptr) {
                logger->logState(timeLast, modelId, model->getId(), model->logState());
            }
        #endif
        };

        /**
         * It performs all the operations after running a simulation.
         * @param time final simulation time.
         */
        void stop(double time) override {
            timeLast = time;
        #ifndef NO_LOGGING
            if (logger != nullptr) {
                logger->logState(timeLast, modelId, model->getId(), model->logState());
            }
        #endif
        }

        /**
         * It calls to the output function of the atomic model.
         * @param time current simulation time.
         */
        void collection(double time) override {
            if (time >= timeNext) {
                model->output();
            }
        }

        /**
         * It calls to the corresponding state transition function.
         * @param time current simulation time.
         */
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
            }
        #ifndef NO_LOGGING
            if (logger != nullptr) {
                logger->logModel(time, modelId, model, time >= timeNext);
            }
        #endif
            timeLast = time;
            timeNext = time + model->timeAdvance();
        }

        //! It clears all the ports of the model.
        void clear() override {
            model->clearPorts();
        }
    };
}

#endif // CADMIUM_SIMULATION_CORE_SIMULATOR_HPP_
