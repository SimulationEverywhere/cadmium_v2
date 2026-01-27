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
#include "../../modeling/idevs/atomic.hpp"
#include "../../modeling/idevs/coupled.hpp"
#include "../../modeling/idevs/component.hpp"
#include "simulator.hpp"
#include <iostream>
#include <execution>

namespace cadmium {   
    //! DEVS sequential coordinator class.
    class Coordinator: public AbstractSimulator {
    private:
        std::shared_ptr<Coupled> model;                              //!< Pointer to coupled model of the coordinator.
        // std::vector<std::shared_ptr<AbstractSimulator>> simulators;  //!< Vector of child simulators.

        std::vector<std::shared_ptr<Simulator>> models;
        std::vector<Simulator*> imminent;

        static constexpr double inf = std::numeric_limits<double>::infinity();

        // void check_schedulable(std::vector<simulator_t*> imm_list) {
        //     for(auto& i : imm_list) {
        //         if(i->imm) {
        //             /* SOME TEST TO CHECK SCHEDULABILITY*/
        //             i->schedulable = true;
        //         }
        //     }
        // }

    public:
        Coordinator(std::shared_ptr<Coupled> model, double time): AbstractSimulator(time), model(std::move(model)) {
            if (this->model == nullptr) {
                throw CadmiumSimulationException("no coupled model provided");
            }
            timeLast = time;
            for (auto& [componentId, component]: this->model->getComponents()) {
                auto coupled = std::dynamic_pointer_cast<Coupled>(component);
                if (coupled == nullptr) {
                    auto atomic = std::dynamic_pointer_cast<AtomicInterface>(component);
                    if (atomic == nullptr) {
                        throw CadmiumSimulationException("component is not a coupled nor atomic model");
                    }

                    auto m = std::make_shared<Simulator>(atomic, time);

                    models.push_back(m);
                    timeNext = std::min(timeNext, m->getTimeNext());
                }
            }

            for(auto& [portFrom, portTo] : this->model->getSerialICs()) {
                auto parentFrom = portFrom->getParent();
                auto parentTo = portTo->getParent();
                for(auto& m : models) {
                    if(parentFrom && parentFrom == m->getComponent().get()){
                        for(auto& inf_m: models) {
                            if(parentTo && parentTo == inf_m->getComponent().get()){
                                m->influencees.push_back(inf_m.get());
                            }
                        }
                    }
                }
            }

            #ifdef DEBUG
                std::cout << "Printing influencees: " << std::endl;
                std::for_each(models.begin(), models.end(), [](const auto& m){
                    std::cout << "\t Model: " << m->getComponent()->getId();
                    std::cout << ((m->influencees.empty())? " has no influencees" : " has influencees:") << std::endl;
                    std::for_each(m->influencees.begin(), m->influencees.end(), [](const auto& inf){
                        std::cout << "\t\t" << inf->getComponent()->getId() << std::endl;
                    });
                });

                std::cout << "Serialized: " << std::endl;
                for(const auto& [portFrom, portTo] : this->model->getSerialICs()) {
                    std::cout << "\t{" << portFrom->getId() << "(" << portFrom->getParent()->getId() << ")" << " -> " << portTo->getId() << "(" << portTo->getParent()->getId() << ")" << "}" << std::endl;
                }

                std::cout << "\nMapped IC: " << std::endl;
                for(const auto& [portTo, portFroms] : this->model->getICs()) {
                    std::cout << "\t{";
                    for(const auto& portFrom : portFroms) {
                        std::cout << portFrom->getId() << "(" << portFrom->getParent()->getId() << ")" << ", ";
                    }
                    std::cout << "} -> " <<  portTo->getId() << "(" << portTo->getParent()->getId() << ")" << std::endl;
                    
                }

                std::cout << "\nMapped IC (inverse): " << std::endl;
                for(const auto& [portFrom, portTos] : this->model->getInverseICs()) {
                    std::cout << "\t" <<  portFrom->getId() << "(" << portFrom->getParent()->getId() << ")" << "-> {";
                    for(const auto& portTo : portTos) {
                        std::cout << portTo->getId() << "(" << portTo->getParent()->getId() << ")" << ", ";
                    }
                    std::cout << "}" << std::endl;
                    
                }
            #endif

            for(auto& m : models) {
                if(m->Tn <= timeNext) {
                    m->imm = true;
                    imminent.push_back(m.get());
                }
            }

        }

        // ─────────────────── basic getters/boilerplate ────────────────
        std::shared_ptr<Component>      getComponent() const override { return model; }
        std::shared_ptr<Coupled>        getCoupled()  const           { return model; }
        const std::vector<std::shared_ptr<Simulator>>& getSubcomponents() { return models; }

        long setModelId(long next) override {
            modelId = next++;
            for (auto& s : models) next = s->setModelId(next);
            return next;
        }

        void start(double t) override { timeLast = t; for (auto& s : models) s->start(t); }
        void stop (double t) override { timeLast = t; for (auto& s : models) s->stop(t); }

        // ─────────────────────── collection ───────────────────────────

        /**
         * It collects all the output messages and propagates them according to the ICs.
         * @param time new simulation time.
         */
        void collection(double time) override {
            if (time >= timeNext) {

                const auto cache = imminent;

                for(const auto& s : cache) {
                        s->collection(time);
                        for(auto& infl: s->influencees) {
                            if(!infl->imm) {
                                infl->imm = true;
                                imminent.push_back(infl);
                            }
                        }
                }
                
                for (auto& [portFrom, portTo]: model->getSerialICs()) {
                    if(!portFrom->empty())
                        portTo->propagate(portFrom);
                }

                //This is only present to handle outputs to the external world. No other EOCs should exist in flat.
                for (auto& [portFrom, portTo]: model->getSerialEOCs()) {
                    if(!portFrom->empty())
                        portTo->propagate(portFrom);
                }
            }
        }

        // ─────────────────────── transition ───────────────────────────

        /**
         * It propagates input messages according to the EICs and triggers the state transition function of child components.
         * @param time new simulation time.
         */
        void transition(double time) override {
            //This is only present to handle inputs from the external world. No other EICs should exist in flat.
            for (auto& [portFrom, portTo]: model->getSerialEICs()) {
                if(!portFrom->empty())
                    portTo->propagate(portFrom);
            }

            timeLast = time;
            timeNext = inf;

            for (auto& sim: imminent) {

                sim->transition(time);
                sim->clear();

                sim->imm = false;
            }

            std::for_each(models.begin(), models.end(), [&](const auto& m){ timeNext = std::min(timeNext, m->getTimeNext()); });

            imminent.clear();
            for(auto& m : models) {
                if(m->Tn <= timeNext) {
                    m->imm = true;
                    imminent.push_back(m.get());
                }
            }

            // std::cout << "Imminent at time " << timeNext << " s:"  << std::endl;
            // std::for_each(imminent.begin(), imminent.end(), [](const auto& m){ std::cout << "\t" << m->model->getId() << std::endl; });

        }

        //! It clears the messages from all the ports of child components.
        void clear() override {
            // for satisfying virtual
        }

    #ifndef NO_LOGGING
        /**
         * It sets the logger to all the child components.
         * @param log pointer to the new logger.
         */
        void setLogger(const std::shared_ptr<Logger>& log) override {
            std::for_each(models.begin(), models.end(), [log](const auto& s) { s->setLogger(log); });
        }
    #endif
    };
}


#endif // CADMIUM_SIMULATION_CORE_COORDINATOR_HPP_