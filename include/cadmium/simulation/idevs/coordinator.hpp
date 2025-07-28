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
#include "unordered_set"
#include "../../modeling/idevs/atomic.hpp"
#include "../../modeling/idevs/coupled.hpp"
#include "../../modeling/idevs/component.hpp"

#include <iostream>

namespace cadmium {
    //! DEVS sequential coordinator class.
    class Coordinator: public AbstractSimulator {
     private:
        std::shared_ptr<Coupled> model;                              //!< Pointer to coupled model of the coordinator.
        std::vector<std::shared_ptr<AbstractSimulator>> simulators;  //!< Vector of child simulators.
        // std::unordered_map<double, std::unordered_set<std::shared_ptr<AbstractSimulator>>> time_sim_map; //!< Map of time of event and simulator
        // std::vector<time_sim_map_t> time_sim_map;
        std::vector<std::pair<double, std::unordered_set<std::shared_ptr<AbstractSimulator>>>*> time_sim_map;
        std::unordered_map<std::shared_ptr<PortInterface>, std::shared_ptr<AbstractSimulator>> inport_sim_map;
        std::unordered_map<std::shared_ptr<PortInterface>, std::vector<std::shared_ptr<PortInterface>>> IC_map;

        std::pair<double, std::unordered_set<std::shared_ptr<AbstractSimulator>>>* get_sims_for_time(double time, std::vector<std::pair<double, std::unordered_set<std::shared_ptr<AbstractSimulator>>>*> map) {
            for(auto& item : map) {
                if(item->first == time) {
                    return item;
                }
            }
            return nullptr;
        }
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

                auto tn = simulator->getTimeNext();
                bool flag = false;
                for(auto element: time_sim_map) {
                    if(element->first == tn) {
                        element->second.insert(simulator);
                        flag = true;
                        break;
                    }
                }
                if(!flag) {
                    std::unordered_set<std::shared_ptr<AbstractSimulator>> k;
                    k.insert(simulator);
                    time_sim_map.push_back(new std::pair<double, std::unordered_set<std::shared_ptr<AbstractSimulator>>>(tn, k));
                }

                for(const auto& p : simulator->getComponent()->getInPorts()){
                    inport_sim_map[p] = simulator;
                }

                timeNext = std::min(timeNext, simulator->getTimeNext());
            }

            for(auto& [portFrom, portTo]: this->model->getSerialICs()) {
                IC_map[portFrom].push_back(std::move(portTo));
            }

            #ifdef DEBUG
                std::cout << "Initial imminent map:" << std::endl;
                for (const auto& [key, val] : time_sim_map) {
                    std::cout << "\t" << key << "s => " << std::endl;
                    for(const auto& s : val) {
                        std::cout << "\t\t" << s->getComponent()->getId() << std::endl;
                    }
                }

                std::cout << "Inport-Simulator Map: " << std::endl;
                for (const auto& [port, sim] : inport_sim_map) {
                    std::cout << "\t" << port->getId() << " => " << sim->getComponent()->getId() << std::endl;
                }
            #endif
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
            std::for_each(time_sim_map.begin(), time_sim_map.end(), [](auto& ptr){ delete ptr; });
        }

        /**
         * It collects all the output messages and propagates them according to the ICs and EOCs.
         * @param time new simulation time.
         */
        bool collection(double time) override {
            if (time >= timeNext) {


                #ifdef DEBUG
                    std::cout << "At time " << time << "s outputs are at:\n";
                #endif
                const auto active_pair = get_sims_for_time(time, time_sim_map);

                if(active_pair != nullptr) {
                    const auto local_cache = active_pair->second;
                    for(auto& s : local_cache){
                        if(s->collection(time)) {
                            for(const auto& p: s->getComponent()->getOutPorts()) {
                                #ifdef DEBUG
                                    std::cout << "\t port \"" << p->getId() << "\" of " << s->getComponent()->getId() << ((p->empty())? " is empty\n" : " propagates to:\n");
                                #endif
                                if(!p->empty()) {
                                    for(auto& portTo : IC_map[p]) {
                                        #ifdef DEBUG
                                            std::cout << "\t\t" << portTo->getId() << " of " << inport_sim_map[portTo]->getComponent()->getId() << std::endl;
                                        #endif
                                        // auto sims = get_sims_for_time(time, time_sim_map);
                                        active_pair->second.insert(inport_sim_map.at(portTo));
                                        portTo->propagate(p);
                                    }
                                }
                            }
                            #ifdef DEBUG
                                std::cout << std::endl;
                            #endif
                        }
                    }
                }



                #ifdef DEBUG
                    std::cout << "Imminent map after collection:" << std::endl;
                    for (const auto& [key, val] : time_sim_map) {
                        std::cout << "\t" << key << "s => " << std::endl;
                        for(const auto& s : val) {
                            std::cout << "\t\t" << s->getComponent()->getId() << std::endl;
                        }
                    }
                #endif


                // for (auto& [portFrom, portTo]: model->getSerialEOCs()) {
                //     if(!portFrom->empty()) {
                //         #ifdef DEBUG
                //             std::cout << "Propagate to " << portTo->getParent()->getId() << std::endl;
                //         #endif
                //         portTo->propagate(portFrom);
                //     }
                // }
            }
            return true;
        }

        /**
         * It propagates input messages according to the EICs and triggers the state transition function of child components.
         * @param time new simulation time.
         */
        void transition(double time) override {
            // for (auto& [portFrom, portTo]: model->getSerialEICs()) {
            //     portTo->propagate(portFrom);
            // }

            timeLast = time;
            timeNext = std::numeric_limits<double>::infinity();

            std::unordered_set<std::shared_ptr<AbstractSimulator>> local_cache; //<! to handle cases where tn == time

            auto iter_sims = get_sims_for_time(time, time_sim_map)->second;
            for(auto& sim : iter_sims) {
                sim->transition(time);

                auto tn = sim->getTimeNext();
                if(tn == time) {
                    local_cache.insert(sim);
                } else {
                    bool flag = false;
                    for(auto element: time_sim_map) {
                        if(element->first == tn) {
                            element->second.insert(sim);
                            flag = true;
                            break;
                        }
                    }
                    if(!flag) {
                        std::unordered_set<std::shared_ptr<AbstractSimulator>> k;
                        k.insert(sim);
                        time_sim_map.push_back(new std::pair<double, std::unordered_set<std::shared_ptr<AbstractSimulator>>>(tn, k));
                    }
                }
                sim->clear();
            }

            auto it = time_sim_map.begin();
            while (it != time_sim_map.end()) {
                if ((*it)->first == time) {
                    delete *it;
                    time_sim_map.erase(it);
                    if(!local_cache.empty()) {
                        time_sim_map.push_back(new std::pair<double, std::unordered_set<std::shared_ptr<AbstractSimulator>>>(time, local_cache));
                    }
                    break;
                }
                ++it;
            }

            for(const auto& pair : time_sim_map) {
                timeNext = std::min(timeNext, pair->first);
            }

            #ifdef DEBUG
                std::cout << "Imminent map after transition:" << std::endl;
                for (const auto& [key, val] : time_sim_map) {
                    std::cout << "\t" << key << "s => " << std::endl;
                    for(const auto& s : val) {
                        std::cout << "\t\t" << s->getComponent()->getId() << std::endl;
                    }
                }

                std::cout << "\nTimeNext: " << timeNext << std::endl;
            #endif

        }

        //! It clears the messages from all the ports of child components.
        void clear() override {
            // for(auto& s : simulators) {
            //     s->clear();
            // }
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