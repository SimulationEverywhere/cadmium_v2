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

#ifndef CADMIUM_SIMULATION_MQTT_SIMULATOR_HPP_
#define CADMIUM_SIMULATION_MQTT_SIMULATOR_HPP_

#include <memory>
#include <utility>
#include "abs_simulator.hpp"
#include "../../exception.hpp"
#ifndef NO_LOGGING
    #include "../logger/logger.hpp"
#endif
#include "../../modeling/idevs/atomic.hpp"

namespace cadmium {
    //! DEVS simulator.
    class MQTT_Simulator: public AbstractSimulator {
     private:
        std::shared_ptr<AtomicInterface> model;  //!< Pointer to the corresponding atomic DEVS model.
    #ifndef NO_LOGGING
        std::shared_ptr<Logger> logger;
    #endif
        bool collect_flag;
        bool transition_flag;
     public:
    #ifndef NO_LOGGING
        /**
         * Constructor function.
         * @param model pointer to the atomic model.
         * @param time initial simulation time.
         */
        MQTT_Simulator(std::shared_ptr<AtomicInterface> model, double time): AbstractSimulator(time), model(std::move(model)), logger() {
            if (this->model == nullptr) {
                throw CadmiumSimulationException("no atomic model provided");
            }
            timeNext = timeLast + this->model->timeAdvance();
            collect_flag = false;
            transition_flag = false;
        }
    #else
        /**
         * Constructor function.
         * @param model pointer to the atomic model.
         * @param time initial simulation time.
         */
        MQTT_Simulator(std::shared_ptr<AtomicInterface> model, double time): AbstractSimulator(time), model(std::move(model)) {
            if (this->model == nullptr) {
                throw CadmiumSimulationException("no atomic model provided");
            }
            timeNext = timeLast + this->model->timeAdvance();
            collect_flag = false;
            transition_flag = false;
        }
    #endif

    #ifdef RAW_PTR
        //! @return pointer to the corresponding atomic DEVS model.
        [[nodiscard]] Component* getComponent() const override {
            return model.get();
        }
    #else
        //! @return pointer to the corresponding atomic DEVS model.
        [[nodiscard]] std::shared_ptr<Component> getComponent() const override {
            return model;
        }
    #endif
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

        bool collection(double time) override {
            if (time >= timeNext && !collect_flag) {
                model->output();
                collect_flag = true;
                return true;
            }
            return false;
        }

        /**
         * It calls to the corresponding state transition function.
         * @param time current simulation time.
         */
        void transition(double time) override {
            auto inEmpty = model->inEmpty();
            if (transition_flag) {
                return;
            }
            if (inEmpty) {
                model->internalTransition();
                transition_flag = true;
            } else {
                auto e = time - timeLast;
                (time < timeNext) ? model->externalTransition(e) : model->confluentTransition(e);
                transition_flag = true;
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
            transition_flag = false;
            collect_flag = false;
        }
    };
}

#endif // CADMIUM_SIMULATION_MQTT_SIMULATOR_HPP_
