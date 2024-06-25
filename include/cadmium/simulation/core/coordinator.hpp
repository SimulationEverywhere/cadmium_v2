/**
 * DEVS Coordinator class.
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

#ifndef CADMIUM_SIMULATION_CORE_COORDINATOR_HPP_
#define CADMIUM_SIMULATION_CORE_COORDINATOR_HPP_

#include <memory>
#include <utility>
#include <vector>
#include "abs_simulator.hpp"
#include "simulator.hpp"
#include "../../modeling/devs/atomic.hpp"
#include "../../modeling/devs/coupled.hpp"
#include "../../modeling/devs/component.hpp"

namespace cadmium {
    //! DEVS sequential coordinator class.
    class Coordinator: public AbstractSimulator {
     private:
        std::shared_ptr<Coupled> model;                              //!< Pointer to coupled model of the coordinator.
        std::vector<std::shared_ptr<AbstractSimulator>> simulators;  //!< Vector of child simulators.
     public:
        /**
         * Constructor function.
         * @param model pointer to the coordinator coupled model.
         * @param time initial simulation time.
         * @param parallel if true, simulators will use mutexes for logging.
         */
        Coordinator(std::shared_ptr<Coupled> model, double time): AbstractSimulator(time), model(std::move(model)) {
            if (this->model == nullptr) {
                throw CadmiumSimulationException("no coupled model provided");
            }
            timeLast = time;
            for (auto& [componentId, component]: this->model->getComponents()) {
                std::shared_ptr<AbstractSimulator> simulator;
                auto coupled = std::dynamic_pointer_cast<Coupled>(component);
                if (coupled != nullptr) {
                    simulator = std::make_shared<Coordinator>(coupled, time);
                } else {
                    auto atomic = std::dynamic_pointer_cast<AtomicInterface>(component);
                    if (atomic == nullptr) {
                        throw CadmiumSimulationException("component is not a coupled nor atomic model");
                    }
                    simulator = std::make_shared<Simulator>(atomic, time);
                }
                simulators.push_back(simulator);
                timeNext = std::min(timeNext, simulator->getTimeNext());
            }
        }

        //! @return pointer to the coupled model of the coordinator.
        [[nodiscard]] std::shared_ptr<Component> getComponent() const override {
            return model;
        }

        //! @return pointer to the coupled model of the coordinator without upcasting it to an abstract Component.
        [[nodiscard]] std::shared_ptr<Coupled> getCoupled() const {
            return model;
        }

        //! @return pointer to subcomponents.
        [[nodiscard]] const std::vector<std::shared_ptr<AbstractSimulator>>& getSubcomponents() {
            return simulators;
        }

        /**
         * Sets the model ID of its coupled model and all the models of its child simulators.
         * @param next next available model ID.
         * @return next available model ID after assiging the ID to all the child models.
         */
        long setModelId(long next) override {
            modelId = next++;
            for (auto& simulator: simulators) {
                next = simulator->setModelId(next);
            }
            return next;
        }

        //! It updates the initial simulation time and calls to the start method of all its child simulators.
        void start(double time) override {
            timeLast = time;
            std::for_each(simulators.begin(), simulators.end(), [time](auto& s) { s->start(time); });
        }

        //! It  updates the final simulation time and calls to the stop method of all its child simulators.
        void stop(double time) override {
            timeLast = time;
            std::for_each(simulators.begin(), simulators.end(), [time](auto& s) { s->stop(time); });
        }

        /**
         * It collects all the output messages and propagates them according to the ICs and EOCs.
         * @param time new simulation time.
         */
        void collection(double time) override {
            if (time >= timeNext) {
                std::for_each(simulators.begin(), simulators.end(), [time](auto& s) { s->collection(time); });
                for (auto& [portFrom, portTo]: model->getSerialICs()) {
                    portTo->propagate(portFrom);
                }
                for (auto& [portFrom, portTo]: model->getSerialEOCs()) {
                    portTo->propagate(portFrom);
                }
            }
        }

        /**
         * It propagates input messages according to the EICs and triggers the state transition function of child components.
         * @param time new simulation time.
         */
        void transition(double time) override {
            for (auto& [portFrom, portTo]: model->getSerialEICs()) {
                portTo->propagate(portFrom);
            }
            timeLast = time;
            timeNext = std::numeric_limits<double>::infinity();
            for (auto& simulator: simulators) {
                simulator->transition(time);
                timeNext = std::min(timeNext, simulator->getTimeNext());
            }
        }

        //! It clears the messages from all the ports of child components.
        void clear() override {
            std::for_each(simulators.begin(), simulators.end(), [](auto& s) { s->clear(); });
            model->clearPorts();
        }

    #ifndef NO_LOGGING
        /**
         * It sets the logger to all the child components.
         * @param log pointer to the new logger.
         */
        void setLogger(const std::shared_ptr<Logger>& log) override {
            std::for_each(simulators.begin(), simulators.end(), [log](auto& s) { s->setLogger(log); });
        }
    #endif
    };
}

#endif // CADMIUM_SIMULATION_CORE_COORDINATOR_HPP_