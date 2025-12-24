/**
 * DEVS simulator.
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2021-present Román Cárdenas Rodríguez
 * ARSLab - Carleton University
 * GreenLSI - Polytechnic University of Madrid
 */


#ifndef CADMIUM_CORE_SIMULATION_SIMULATOR_HPP_
#define CADMIUM_CORE_SIMULATION_SIMULATOR_HPP_

#include <memory>
#include <utility>
#include "abs_simulator.hpp"
#include "../exception.hpp"
#include "../logger/logger.hpp"
#include "../modeling/atomic.hpp"

namespace cadmium {
    //! DEVS simulator.
    class Simulator: public AbstractSimulator {
     private:
        std::shared_ptr<AtomicInterface> model;  //!< Pointer to the corresponding atomic DEVS model.
        std::shared_ptr<Logger> logger;          //!< Pointer to logger (for output messages and state).
     public:
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

        /**
         * Sets a new logger.
         * @param log pointer to the logger.
         */
        void setLogger(const std::shared_ptr<Logger>& log) override {
            logger = log;
        }

        /**
         * It performs all the operations before running a simulation.
         * @param time initial simulation time.
         */
        void start(double time) override {
            timeLast = time;
            if (logger != nullptr) {
                logger->lock();
                logger->logState(timeLast, modelId, model->getId(), model->logState());
                logger->unlock();
            }
        };

        /**
         * It performs all the operations after running a simulation.
         * @param time final simulation time.
         */
        void stop(double time) override {
            timeLast = time;
            if (logger != nullptr) {
                logger->lock();
                logger->logState(timeLast, modelId, model->getId(), model->logState());
                logger->unlock();
            }
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
            if (logger != nullptr) {
                logger->lock();  // TODO leave lock/unlock calls only for parallel execution
                if (time >= timeNext) {
                    for (const auto& outPort: model->getOutPorts()) {
                        for (std::size_t i = 0; i < outPort->size(); ++i) {
                            logger->logOutput(time, modelId, model->getId(), outPort->getId(), outPort->logMessage(i));
                        }
                    }
                }
                logger->logState(time, modelId, model->getId(), model->logState());
                logger->unlock();  // TODO leave lock/unlock calls only for parallel execution
            }
            timeLast = time;
            timeNext = time + model->timeAdvance();
        }

        //! It clears all the ports of the model.
        void clear() override {
            model->clearPorts();
        }
    };
}

#endif //CADMIUM_CORE_SIMULATION_SIMULATOR_HPP_
