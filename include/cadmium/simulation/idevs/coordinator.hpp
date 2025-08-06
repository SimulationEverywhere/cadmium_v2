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
// #include "../logger/logger.hpp"

#include <iostream>

// #define DIRECT


#if defined(MAP_VEC)
    namespace cadmium {
        //! DEVS sequential coordinator class.
        class Coordinator: public AbstractSimulator {
        private:
            std::shared_ptr<Coupled> model;                              //!< Pointer to coupled model of the coordinator.
            std::vector<std::shared_ptr<AbstractSimulator>> simulators;  //!< Vector of child simulators.
            std::unordered_map<std::shared_ptr<PortInterface>, std::shared_ptr<AbstractSimulator>> inport_sim_map;
            std::unordered_map<std::shared_ptr<PortInterface>, std::vector<std::shared_ptr<PortInterface>>> IC_map;

            std::unordered_map<double, std::vector<std::shared_ptr<AbstractSimulator>>> time_sim_map; //!< Map of time of event and simulator
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

                    time_sim_map[simulator->getTimeNext()].push_back(simulator);

                    for(const auto& p : simulator->getComponent()->getInPorts()){
                        inport_sim_map[p] = simulator;
                    }

                    timeNext = std::min(timeNext, simulator->getTimeNext());
                }

                // std::cout << "Size: " << time_sim_map.size() << std::endl;

                for(auto& [portFrom, portTo]: this->model->getSerialICs()) {
                    IC_map[portFrom].push_back(std::move(portTo));
                }

                #ifdef DEBUG
                    std::cout << "Initial imminent map:" << std::endl;
                    for (const auto& [key, val] : time_sim_map) {
                        std::cout << "\t" << key << "s => " << val->getComponent()->getId() << std::endl;
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
            }

            bool collection(double time) override {
                if (time >= timeNext) {


                    #ifdef DEBUG
                        std::cout << "At time " << time << "s outputs are at:\n";
                    #endif
                    auto local_cache = time_sim_map.at(time);

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
                                        time_sim_map[time].push_back(inport_sim_map.at(portTo));
                                        portTo->propagate(p);
                                    }
                                }
                            }
                            #ifdef DEBUG
                                std::cout << std::endl;
                            #endif
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

                for(auto& sim : time_sim_map[time]) {
                    sim->transition(time);

                    auto tn = sim->getTimeNext();
                    if(tn == time) {
                        local_cache.insert(sim);
                    } else if(tn != std::numeric_limits<double>::infinity()) {
                        time_sim_map[tn].push_back(sim);
                    }
                    
                }
                time_sim_map.erase(time);

                if(!local_cache.empty()) {
                    time_sim_map[time].insert(time_sim_map[time].begin(), local_cache.begin(), local_cache.end());
                }

                for(const auto& [k, v] : time_sim_map) {
                    timeNext = std::min(timeNext, k);
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
                for(auto s:simulators) {
                    s->clear();
                }
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

#elif defined(MAP_SET)
    namespace cadmium {
        //! DEVS sequential coordinator class.
        class Coordinator: public AbstractSimulator {
        private:
            std::shared_ptr<Coupled> model;                              //!< Pointer to coupled model of the coordinator.
            std::vector<std::shared_ptr<AbstractSimulator>> simulators;  //!< Vector of child simulators.
            std::unordered_map<std::shared_ptr<PortInterface>, std::shared_ptr<AbstractSimulator>> inport_sim_map;
            std::unordered_map<std::shared_ptr<PortInterface>, std::vector<std::shared_ptr<PortInterface>>> IC_map;

            std::unordered_map<double, std::unordered_set<std::shared_ptr<AbstractSimulator>>> time_sim_map; //!< Map of time of event and simulator
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

                    time_sim_map[simulator->getTimeNext()].insert(simulator);

                    for(const auto& p : simulator->getComponent()->getInPorts()){
                        inport_sim_map[p] = simulator;
                    }

                    timeNext = std::min(timeNext, simulator->getTimeNext());
                }

                // std::cout << "Size: " << time_sim_map.size() << std::endl;

                for(auto& [portFrom, portTo]: this->model->getSerialICs()) {
                    IC_map[portFrom].push_back(std::move(portTo));
                }

                #ifdef DEBUG
                    std::cout << "Initial imminent map:" << std::endl;
                    for (const auto& [key, val] : time_sim_map) {
                        std::cout << "\t" << key << "s => " << val->getComponent()->getId() << std::endl;
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
            }

            bool collection(double time) override {
                if (time >= timeNext) {


                    #ifdef DEBUG
                        std::cout << "At time " << time << "s outputs are at:\n";
                    #endif
                    auto local_cache = time_sim_map.at(time);

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
                                        time_sim_map[time].insert(inport_sim_map.at(portTo));
                                        portTo->propagate(p);
                                    }
                                }
                            }
                            #ifdef DEBUG
                                std::cout << std::endl;
                            #endif
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

                for(auto& sim : time_sim_map[time]) {
                    sim->transition(time);

                    auto tn = sim->getTimeNext();
                    if(tn == time) {
                        local_cache.insert(sim);
                    } else if(tn != std::numeric_limits<double>::infinity()) {
                        time_sim_map[tn].insert(sim);
                    }
                    sim->clear();
                    
                }
                time_sim_map.erase(time);

                if(!local_cache.empty()) {
                    time_sim_map[time] = local_cache;
                }

                for(const auto& [k, v] : time_sim_map) {
                    timeNext = std::min(timeNext, k);
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
                // for(auto s:simulators) {
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

#elif defined(VECTOR)

    namespace cadmium {
        //! DEVS sequential coordinator class.
        class Coordinator: public AbstractSimulator {
        private:
            std::shared_ptr<Coupled> model;                              //!< Pointer to coupled model of the coordinator.
            std::vector<std::shared_ptr<AbstractSimulator>> simulators;  //!< Vector of child simulators.
            std::unordered_map<std::shared_ptr<PortInterface>, std::shared_ptr<AbstractSimulator>> inport_sim_map;
            std::unordered_map<std::shared_ptr<PortInterface>, std::vector<std::shared_ptr<PortInterface>>> IC_map;
            std::vector<std::pair<double, std::shared_ptr<AbstractSimulator>>> time_sim_map;

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

                    time_sim_map.push_back({simulator->getTimeNext(), simulator});

                    for(const auto& p : simulator->getComponent()->getInPorts()){
                        inport_sim_map[p] = simulator;
                    }

                    timeNext = std::min(timeNext, simulator->getTimeNext());
                }

                // std::cout << "Size: " << time_sim_map.size() << std::endl;

                for(auto& [portFrom, portTo]: this->model->getSerialICs()) {
                    IC_map[portFrom].push_back(std::move(portTo));
                }

                #ifdef DEBUG
                    std::cout << "Initial imminent map:" << std::endl;
                    for (const auto& [key, val] : time_sim_map) {
                        std::cout << "\t" << key << "s => " << val->getComponent()->getId() << std::endl;
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
            }

            /**
             * It collects all the output messages and propagates them according to the ICs and EOCs.
             * @param time new simulation time.
             */
            bool collection(double time) override {
                if (time >= timeNext) {
                    // std::cout << "Size: " << time_sim_map.size() << std::endl;

                    #ifdef DEBUG
                        std::cout << "At time " << time << "s outputs are at:\n";
                    #endif

                    const auto local_cache = time_sim_map;
                    for(auto& pair : local_cache){
                        if(pair.first == time){
                            auto s = pair.second;
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
                                            time_sim_map.push_back({time, inport_sim_map.at(portTo)});
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
                            std::cout << "\t" << key << "s => " << val->getComponent()->getId() << std::endl;
                        }
                    #endif


                    for (auto& [portFrom, portTo]: model->getSerialEOCs()) {
                        if(!portFrom->empty()) {
                            #ifdef DEBUG
                                std::cout << "Propagate to " << portTo->getParent()->getId() << std::endl;
                            #endif
                            portTo->propagate(portFrom);
                        }
                    }
                }
                return true;
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
                
                std::vector<decltype(time_sim_map)::value_type> postponed;

                std::erase_if(time_sim_map, [&](auto& pr){
                    if (pr.first != time) return false;          // keep it
                    pr.second->transition(time);                 // run child
                    double tn = pr.second->getTimeNext();
                    if (tn != time){ 
                        postponed.push_back({tn, pr.second});
                        return true;
                    }
                    return false;                                 // erase old pair
                });

                // Add the rescheduled simulators
                time_sim_map.insert(time_sim_map.end(),
                                    postponed.begin(), postponed.end());

                // Recompute next event time
                timeNext = std::numeric_limits<double>::infinity();
                for (const auto& [t, _] : time_sim_map)
                    timeNext = std::min(timeNext, t);

                        
                #ifdef DEBUG
                    std::cout << "Imminent map after transition:" << std::endl;
                    for (const auto& [key, val] : time_sim_map) {
                        std::cout << "\t" << key << "s => " << val->getComponent()->getId() << std::endl;
                    }

                    std::cout << "\nTimeNext: " << timeNext << std::endl;
                #endif

            }

            //! It clears the messages from all the ports of child components.
            void clear() override {
                for(auto& s : simulators) {
                    s->clear();
                }
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

#elif defined(LL)
    #include "simulator_list.hpp"
    namespace cadmium {
        //! DEVS sequential coordinator class.
        class Coordinator: public AbstractSimulator {
        private:
            std::shared_ptr<Coupled> model;                              //!< Pointer to coupled model of the coordinator.
            std::vector<std::shared_ptr<AbstractSimulator>> simulators;  //!< Vector of child simulators.
            std::unordered_map<std::shared_ptr<PortInterface>, std::shared_ptr<AbstractSimulator>> inport_sim_map;
            std::unordered_map<std::shared_ptr<PortInterface>, std::vector<std::shared_ptr<PortInterface>>> IC_map;
            static constexpr double inf = std::numeric_limits<double>::infinity();

            struct time_sim_t {
                double time;
                std::shared_ptr<AbstractSimulator> sim;

                time_sim_t(double t, std::shared_ptr<AbstractSimulator> s): sim(s), time(t) {}
            };

            simulator_ll_raw<time_sim_t> time_sim_map;

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

                    time_sim_map.push_back(time_sim_t(simulator->getTimeNext(), simulator));

                    for(const auto& p : simulator->getComponent()->getInPorts()){
                        inport_sim_map[p] = simulator;
                    }

                    timeNext = std::min(timeNext, simulator->getTimeNext());
                }

                // std::cout << "Size: " << time_sim_map.size() << std::endl;

                for(auto& [portFrom, portTo]: this->model->getSerialICs()) {
                    IC_map[portFrom].push_back(std::move(portTo));
                }

                #ifdef DEBUG
                    std::cout << "Initial imminent map:" << std::endl;
                    time_sim_map.traverse_forward([&](auto pair) {
                        std::cout << "\t" << pair.first << "s => " << pair.second->getComponent()->getId() << std::endl;
                    });

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
            }

            /**
             * It collects all the output messages and propagates them according to the ICs and EOCs.
             * @param time new simulation time.
             */
            bool collection(double time) override {
                if (time >= timeNext) {
                    // std::cout << "Size: " << time_sim_map.size() << std::endl;

                    #ifdef DEBUG
                        std::cout << "At time " << time << "s outputs are at:\n";
                    #endif

                    time_sim_map.traverse_forward([&](auto pair) {
                        if(pair.time == time){
                            auto s = pair.sim;
                            if(s->collection(time)) {
                                for(const auto& p: s->getComponent()->getOutPorts()) {
                                    #ifdef DEBUG
                                        std::cout << "\t port \"" << p->getId() << "\" of " << s->getComponent()->getId() << ((p->empty())? " is empty\n" : " propagates to:\n");
                                    #endif
                                    if(!p->empty()) {
                                        auto ports = IC_map[p];
                                        for(auto& portTo : ports) {
                                            #ifdef DEBUG
                                                std::cout << "\t\t" << portTo->getId() << " of " << inport_sim_map[portTo]->getComponent()->getId() << std::endl;
                                            #endif
                                            time_sim_map.push_front(time_sim_t(time, inport_sim_map[portTo]));
                                            portTo->propagate(p);
                                        }
                                    }
                                }
                                #ifdef DEBUG
                                    std::cout << std::endl;
                                #endif
                            }
                        }
                    });


                    #ifdef DEBUG
                        std::cout << "Imminent map after collection:" << std::endl;
                        time_sim_map.traverse_forward([&](auto pair) {
                            std::cout << "\t" << pair.first << "s => " << pair.second->getComponent()->getId() << std::endl;
                        });
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
                for (auto& [portFrom, portTo]: model->getSerialEICs()) {
                    portTo->propagate(portFrom);
                }

                timeLast = time;
                timeNext = inf;

                time_sim_map.erase_if([&](auto& pr){
                    if (pr.time != time){ 
                        timeNext = std::min(timeNext, pr.time);
                        return false;
                    }
                    pr.sim->transition(time);                 // run child
                    double tn = pr.sim->getTimeNext();
                    if (tn != time){ 
                        timeNext = std::min(timeNext, tn);
                        if(tn != inf) {
                            time_sim_map.push_front(time_sim_t(tn, pr.sim));
                        }
                        return true;
                    }
                    timeNext = std::min(timeNext, tn);
                    return false;                                 // erase old pair
                });

                // Recompute next event time
                // timeNext = std::numeric_limits<double>::infinity();
                // for (const auto& [t, _] : time_sim_map)
                //     timeNext = std::min(timeNext, t);

                        
                #ifdef DEBUG
                    std::cout << "Imminent map after transition:" << std::endl;
                    time_sim_map.traverse_forward([&](auto pair) {
                        std::cout << "\t" << pair.first << "s => " << pair.second->getComponent()->getId() << std::endl;
                    });

                    std::cout << "\nTimeNext: " << timeNext << std::endl;
                #endif

            }

            //! It clears the messages from all the ports of child components.
            void clear() override {
                for(auto& s : simulators) {
                    s->clear();
                }
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
#elif defined(TIME_LL)
    #include "simulator_time_list.hpp"
    namespace cadmium {
        //! DEVS sequential coordinator class.
        class Coordinator: public AbstractSimulator {
        private:
            std::shared_ptr<Coupled> model;                              //!< Pointer to coupled model of the coordinator.
            std::vector<std::shared_ptr<AbstractSimulator>> simulators;  //!< Vector of child simulators.
            std::unordered_map<std::shared_ptr<PortInterface>, std::shared_ptr<AbstractSimulator>> inport_sim_map;
            std::unordered_map<std::shared_ptr<PortInterface>, std::vector<std::shared_ptr<PortInterface>>> IC_map;

            simulator_time_ll_raw<std::shared_ptr<AbstractSimulator>> time_sim_map;

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

                    time_sim_map.push_back(simulator, simulator->getTimeNext());

                    for(const auto& p : simulator->getComponent()->getInPorts()){
                        inport_sim_map[p] = simulator;
                    }

                    timeNext = std::min(timeNext, simulator->getTimeNext());
                }

                // std::cout << "Size: " << time_sim_map.size() << std::endl;

                for(auto& [portFrom, portTo]: this->model->getSerialICs()) {
                    IC_map[portFrom].push_back(std::move(portTo));
                }

                #ifdef DEBUG
                    std::cout << "Initial imminent map:" << std::endl;
                    time_sim_map.traverse_forward([&](auto pair) {
                        std::cout << "\t" << pair.first << "s => " << pair.second->getComponent()->getId() << std::endl;
                    });

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
            }

            /**
             * It collects all the output messages and propagates them according to the ICs and EOCs.
             * @param time new simulation time.
             */
            bool collection(double time) override {
                if (time >= timeNext) {
                    // std::cout << "Size: " << time_sim_map.size() << std::endl;

                    #ifdef DEBUG
                        std::cout << "At time " << time << "s outputs are at:\n";
                    #endif

                    for(auto s: simulators) {
                        if(s->collection(time)) {
                            for(const auto& p: s->getComponent()->getOutPorts()) {
                                #ifdef DEBUG
                                    std::cout << "\t port \"" << p->getId() << "\" of " << item->getComponent()->getId() << ((p->empty())? " is empty\n" : " propagates to:\n");
                                #endif
                                if(!p->empty()) {
                                    auto ports = IC_map[p];
                                    for(auto& portTo : ports) {
                                        #ifdef DEBUG
                                            std::cout << "\t\t" << portTo->getId() << " of " << inport_sim_map[portTo]->getComponent()->getId() << std::endl;
                                        #endif
                                        time_sim_map.push_front(inport_sim_map[portTo], time);
                                        portTo->propagate(p);
                                    }
                                }
                            }
                            #ifdef DEBUG
                                std::cout << std::endl;
                            #endif
                        }
                    }

                    // time_sim_map.traverse_forward([&](auto item, auto t) {
                    //     if(t == time){
                    //         if(item->collection(time)) {
                    //             for(const auto& p: item->getComponent()->getOutPorts()) {
                    //                 #ifdef DEBUG
                    //                     std::cout << "\t port \"" << p->getId() << "\" of " << item->getComponent()->getId() << ((p->empty())? " is empty\n" : " propagates to:\n");
                    //                 #endif
                    //                 if(!p->empty()) {
                    //                     auto ports = IC_map[p];
                    //                     for(auto& portTo : ports) {
                    //                         #ifdef DEBUG
                    //                             std::cout << "\t\t" << portTo->getId() << " of " << inport_sim_map[portTo]->getComponent()->getId() << std::endl;
                    //                         #endif
                    //                         time_sim_map.push_front(inport_sim_map[portTo], time);
                    //                         portTo->propagate(p);
                    //                     }
                    //                 }
                    //             }
                    //             #ifdef DEBUG
                    //                 std::cout << std::endl;
                    //             #endif
                    //         }
                    //     }
                    // });


                    #ifdef DEBUG
                        std::cout << "Imminent map after collection:" << std::endl;
                        time_sim_map.traverse_forward([&](auto pair) {
                            std::cout << "\t" << pair.first << "s => " << pair.second->getComponent()->getId() << std::endl;
                        });
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
                auto inf = std::numeric_limits<double>::infinity();

                time_sim_map.erase_if([&](auto& item, double t){
                    if (t != time){ 
                        timeNext = std::min(timeNext, t);
                        return false;
                    }
                    item->transition(time);                 // run child
                    double tn = item->getTimeNext();
                    if (tn != time){ 
                        if(tn != inf) {
                            timeNext = std::min(timeNext, tn);
                            time_sim_map.push_front(item, tn);
                        }
                        return true;
                    }
                    timeNext = std::min(timeNext, tn);
                    return false;
                });

                // Recompute next event time
                // timeNext = std::numeric_limits<double>::infinity();
                // for (const auto& [t, _] : time_sim_map)
                //     timeNext = std::min(timeNext, t);

                        
                #ifdef DEBUG
                    std::cout << "Imminent map after transition:" << std::endl;
                    time_sim_map.traverse_forward([&](auto pair) {
                        std::cout << "\t" << pair.first << "s => " << pair.second->getComponent()->getId() << std::endl;
                    });

                    std::cout << "\nTimeNext: " << timeNext << std::endl;
                #endif

            }

            //! It clears the messages from all the ports of child components.
            void clear() override {
                for(auto& s : simulators) {
                    s->clear();
                }
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
#elif defined(TIME_LL_UNIQUE)
    #include "simulator_time_list.hpp"
    #define RAW

    namespace cadmium {
        //! DEVS sequential coordinator class.
        class Coordinator: public AbstractSimulator {
        private:
            std::shared_ptr<Coupled> model;                              //!< Pointer to coupled model of the coordinator.
            std::vector<AbstractSimulator*> simulators;  //!< Vector of child simulators.
            std::unordered_map<std::shared_ptr<PortInterface>, AbstractSimulator*> inport_sim_map;
            std::unordered_map<std::shared_ptr<PortInterface>, std::vector<std::shared_ptr<PortInterface>>> IC_map;

            simulator_time_ll_raw<AbstractSimulator*> time_sim_map;

            static constexpr double inf = std::numeric_limits<double>::infinity();

        public:
            Coordinator(std::shared_ptr<Coupled> model, double time): AbstractSimulator(time), model(std::move(model)) {
                if (this->model == nullptr) {
                    throw CadmiumSimulationException("no coupled model provided");
                }
                timeLast = time;
                for (auto& [componentId, component]: this->model->getComponents()) {
                    AbstractSimulator* simulator;
                    auto coupled = std::dynamic_pointer_cast<Coupled>(component);
                    if (coupled != nullptr) {
                        simulator = new Coordinator(coupled, time);
                    } else {
                        auto atomic = std::dynamic_pointer_cast<AtomicInterface>(component);
                        if (atomic == nullptr) {
                            throw CadmiumSimulationException("component is not a coupled nor atomic model");
                        }
                        simulator = new Simulator(atomic, time);
                    }
                    simulators.push_back(simulator);

                    time_sim_map.push_back(simulator, simulator->getTimeNext());

                    for(const auto& p : simulator->getComponent()->getInPorts()){
                        inport_sim_map[p] = simulator;
                    }

                    timeNext = std::min(timeNext, simulator->getTimeNext());
                }

                for(auto& [portFrom, portTo]: this->model->getSerialICs()) {
                    IC_map[portFrom].push_back(std::move(portTo));
                }

            }

            // ─────────────────── basic getters/boilerplate ────────────────
            std::shared_ptr<Component>      getComponent() const override { return model; }
            std::shared_ptr<Coupled>        getCoupled()  const           { return model; }
            const std::vector<AbstractSimulator*>&      getSubcomponents()            { return simulators; }

            long setModelId(long next) override {
                modelId = next++;
                for (auto& s : simulators) next = s->setModelId(next);
                return next;
            }

            void start(double t) override { timeLast = t;  for (auto& s : simulators) s->start(t);  }
            void stop (double t) override { 
                timeLast = t;  for (auto& s : simulators) s->stop(t); 
                std::for_each(simulators.begin(), simulators.end(), [&](auto sim){ delete sim; });
                // std::for_each(inport_sim_map.begin(), inport_sim_map.end(), [&](auto kv){ delete kv.second; });
                inport_sim_map.clear();
                simulators.clear();
                time_sim_map.clear();
            }

            // ─────────────────────── collection ───────────────────────────

            /**
             * It collects all the output messages and propagates them according to the ICs and EOCs.
             * @param time new simulation time.
             */
            bool collection(double time) override {
                if (time >= timeNext) {
                    // std::cout << "Size: " << time_sim_map.size() << std::endl;

                    #ifdef DEBUG
                        std::cout << "At time " << time << "s outputs are at:\n";
                    #endif

                    for(auto s: simulators) {
                        if(s->collection(time)) {
                            for(const auto& p: s->getComponent()->getOutPorts()) {
                                if(!p->empty()) {
                                    auto ports = IC_map[p];
                                    for(auto& portTo : ports) {
                                        time_sim_map.push_front(inport_sim_map[portTo], time);
                                        portTo->propagate(p);
                                    }
                                }
                            }
                        }
                    }

                }
                return true;
            }

            // ─────────────────────── transition ───────────────────────────

            /**
             * It propagates input messages according to the EICs and triggers the state transition function of child components.
             * @param time new simulation time.
             */
            void transition(double time) override {

                timeLast = time;
                timeNext = inf;

                time_sim_map.erase_if([&](auto& item, double t){
                    if (t != time){ 
                        timeNext = std::min(timeNext, t);
                        return false;
                    }
                    item->transition(time);                 // run child
                    double tn = item->getTimeNext();
                    if (tn != time){ 
                        if(tn != inf) {
                            timeNext = std::min(timeNext, tn);
                            time_sim_map.push_front(item, tn);
                        }
                        // item->clear();
                        return true;
                    }
                    // item->clear();
                    timeNext = std::min(timeNext, tn);
                    return false;
                });

                // Recompute next event time
                // timeNext = std::numeric_limits<double>::infinity();
                // time_sim_map.traverse_forward([&](auto item, double t) {
                //     timeNext = std::min(timeNext, t);
                // } );
                   

                        
                #ifdef DEBUG
                    std::cout << "Imminent map after transition:" << std::endl;
                    time_sim_map.traverse_forward([&](auto pair) {
                        std::cout << "\t" << pair.first << "s => " << pair.second->getComponent()->getId() << std::endl;
                    });

                    std::cout << "\nTimeNext: " << timeNext << std::endl;
                #endif

            }

            //! It clears the messages from all the ports of child components.
            void clear() override {
                for(auto& s : simulators) {
                    s->clear();
                }
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
#elif defined(PQUEUE)

    #include <queue>
    
    namespace cadmium {
        //! DEVS sequential coordinator class.
        class Coordinator: public AbstractSimulator {
        private:
            using SimPtr = std::shared_ptr<AbstractSimulator>;
            // using Event  = std::pair<double, SimPtr>; //Make this a struct to see how different it is?

            struct Event {
                double time;
                SimPtr sim;

                Event(double t, SimPtr s): time(t), sim(s) {}
            };

            struct Earlier {
                bool operator()(const Event& a, const Event& b) const {
                    return a.time > b.time;     // reverse ⇒ min‑heap
                }
            };
                

            static constexpr double inf = std::numeric_limits<double>::infinity();

            std::shared_ptr<Coupled> model;                              //!< Pointer to coupled model of the coordinator.
            std::vector<std::shared_ptr<AbstractSimulator>> simulators;  //!< Vector of child simulators.
            std::unordered_map<std::shared_ptr<PortInterface>, std::shared_ptr<AbstractSimulator>> inport_sim_map;
            std::unordered_map<std::shared_ptr<PortInterface>, std::vector<std::shared_ptr<PortInterface>>> IC_map;

            std::priority_queue<Event,
                        std::vector<Event>,
                        Earlier> agenda;

            void schedule(const SimPtr& s, double t) {
                if (t != inf) agenda.emplace(t, s);
            }

            void refresh_timeNext() {
                timeNext = agenda.empty() ? inf : agenda.top().time;
            }

        public:
            // ───────────────────── constructor ─────────────────────────────
            Coordinator(std::shared_ptr<Coupled> mdl, double t0)
                : AbstractSimulator(t0), model(std::move(mdl)) {
                if (!model) throw CadmiumSimulationException("no coupled model provided");

                timeLast = t0;

                /* build child simulators & initial agenda */
                for (auto& [id, comp] : model->getComponents()) {
                    SimPtr sim;

                    if (auto c = std::dynamic_pointer_cast<Coupled>(comp))
                        sim = std::make_shared<Coordinator>(c, t0);
                    else if (auto a = std::dynamic_pointer_cast<AtomicInterface>(comp))
                        sim = std::make_shared<Simulator>(a, t0);
                    else
                        throw CadmiumSimulationException("component is neither coupled nor atomic");

                    simulators.push_back(sim);
                    schedule(sim, sim->getTimeNext());

                    for (auto& p : sim->getComponent()->getInPorts())
                        inport_sim_map[p] = sim;
                }

                for (auto& [pFrom, pTo] : model->getSerialICs())
                    IC_map[pFrom].push_back(pTo);

                refresh_timeNext();
            }

            // ─────────────────── basic getters/boilerplate ────────────────
            std::shared_ptr<Component>      getComponent() const override { return model; }
            std::shared_ptr<Coupled>        getCoupled()  const           { return model; }
            const std::vector<SimPtr>&      getSubcomponents()            { return simulators; }

            long setModelId(long next) override {
                modelId = next++;
                for (auto& s : simulators) next = s->setModelId(next);
                return next;
            }

            void start(double t) override { timeLast = t;  for (auto& s : simulators) s->start(t);  }
            void stop (double t) override { timeLast = t;  for (auto& s : simulators) s->stop(t);   }

            // ─────────────────────── collection ───────────────────────────
            bool collection(double t) override {
                if (t < timeNext) return true;                 // nothing due

                std::vector<SimPtr> bucket;                    // all sims at this time
                while (!agenda.empty() && agenda.top().time == t) {
                    bucket.push_back(agenda.top().sim);
                    agenda.pop();
                }

                for (auto& s : bucket) {
                    if (!s->collection(t)) continue;
                    schedule(s, t);  // immediate reaction

                    for (auto& pout : s->getComponent()->getOutPorts()) {
                        if (pout->empty()) continue;

                        for (auto& pin : IC_map[pout]) {
                            schedule(inport_sim_map[pin], t);  // immediate reaction
                            pin->propagate(pout);
                        }
                    }
                }
                return true;
            }

            // ─────────────────────── transition ───────────────────────────
            void transition(double t) override {

                timeLast = t;

                std::vector<SimPtr> bucket;
                while (!agenda.empty() && agenda.top().time == t) {
                    bucket.push_back(agenda.top().sim);
                    agenda.pop();
                }

                for (auto& s : bucket) {
                    s->transition(t);
                    schedule(s, s->getTimeNext());
                }

                refresh_timeNext();
            }

            // ─────────────────────── housekeeping ─────────────────────────
            void clear() override {
                for (auto& s : simulators) s->clear();
                model->clearPorts();
            }

            #ifndef NO_LOGGING
                void setLogger(const std::shared_ptr<Logger>& L) override {
                    for (auto& s : simulators) s->setLogger(L);
                }
            #endif
        };

    }   // namespace cadmium
#elif defined(VEC_SET)
    namespace cadmium {
        struct time_sim_t {
            double time;
            std::unordered_set<std::shared_ptr<AbstractSimulator>> sim;

            time_sim_t(double t, std::unordered_set<std::shared_ptr<AbstractSimulator>> s): time(t), sim(s) {}
        };
        // std::ostream &operator<<(std::ostream& out, const time_sim_t& map) {
        //     out << "time: " << map.time << "\n\t";
        //     for(const auto& sim : map.sim) {
        //         out << sim->getComponent()->getId() << " ";
        //     }
        //     out << std::endl;

        //     return out;

        // }
        //! DEVS sequential coordinator class.
        class Coordinator: public AbstractSimulator {
        private:
            std::shared_ptr<Coupled> model;                              //!< Pointer to coupled model of the coordinator.
            std::vector<std::shared_ptr<AbstractSimulator>> simulators;  //!< Vector of child simulators.
            std::unordered_map<std::shared_ptr<PortInterface>, std::shared_ptr<AbstractSimulator>> inport_sim_map;
            std::unordered_map<std::shared_ptr<PortInterface>, std::vector<std::shared_ptr<PortInterface>>> IC_map;

            std::vector<time_sim_t> time_sim_map;

            void schedule(double t, std::shared_ptr<AbstractSimulator> s) {
                for(auto& v: time_sim_map) {
                    if(v.time == t) {
                        v.sim.insert(s);
                    }
                }
                std::unordered_set<std::shared_ptr<AbstractSimulator>> temp;
                temp.insert(s);
                time_sim_map.push_back(time_sim_t(t, temp));

            }

            static constexpr double inf = std::numeric_limits<double>::infinity();

        public:
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

                    schedule(simulator->getTimeNext(), simulator);

                    for(const auto& p : simulator->getComponent()->getInPorts()){
                        inport_sim_map[p] = simulator;
                    }

                    timeNext = std::min(timeNext, simulator->getTimeNext());
                }

                for(auto& [portFrom, portTo]: this->model->getSerialICs()) {
                    IC_map[portFrom].push_back(std::move(portTo));
                }

            }

            // ─────────────────── basic getters/boilerplate ────────────────
            std::shared_ptr<Component>      getComponent() const override { return model; }
            std::shared_ptr<Coupled>        getCoupled()  const           { return model; }
            const std::vector<std::shared_ptr<AbstractSimulator>>&      getSubcomponents()            { return simulators; }

            long setModelId(long next) override {
                modelId = next++;
                for (auto& s : simulators) next = s->setModelId(next);
                return next;
            }

            void start(double t) override { timeLast = t;  for (auto& s : simulators) s->start(t);  }
            void stop (double t) override { timeLast = t;  for (auto& s : simulators) s->stop(t);}

            // ─────────────────────── collection ───────────────────────────

            /**
             * It collects all the output messages and propagates them according to the ICs and EOCs.
             * @param time new simulation time.
             */
            bool collection(double time) override {
                if (time >= timeNext) {

                    std::unordered_set<std::shared_ptr<AbstractSimulator>> local_cache;

                    auto it = time_sim_map.begin();
                    while (it != time_sim_map.end()) {
                        if ((*it).time == time) {
                            local_cache = (*it).sim;
                            for(auto& s : local_cache) {
                                if(s->collection(time)) {
                                    for(const auto& p: s->getComponent()->getOutPorts()) {
                                        if(!p->empty()) {
                                            auto ports = IC_map[p];
                                            for(auto& portTo : ports) {
                                                schedule(time, inport_sim_map[portTo]);
                                                portTo->propagate(p);
                                            }
                                        }
                                    }
                                }
                            }
                            break;
                        }
                        ++it;
                    }

                }
                return true;
            }

            // ─────────────────────── transition ───────────────────────────

            /**
             * It propagates input messages according to the EICs and triggers the state transition function of child components.
             * @param time new simulation time.
             */
            void transition(double time) override {
                timeLast = time;
                timeNext = inf;

                std::unordered_set<std::shared_ptr<AbstractSimulator>> local_cache;

                auto it = time_sim_map.begin();
                while (it != time_sim_map.end()) {
                    if ((*it).time == time) {
                        local_cache = (*it).sim;
                        time_sim_map.erase(it);
                        for(auto& simulator : local_cache) {
                            simulator -> transition(time);
                            double tn = simulator -> getTimeNext();
                            schedule(tn, simulator);
                            simulator -> clear();
                        }
                        break;
                    }
                    ++it;
                }

                // std::cout << "Times:\n\t";
                for(const auto v: time_sim_map) {
                    if(!v.sim.empty()){
                        // std::cout << v.time << " ";
                        timeNext = std::min(timeNext, v.time);
                    }
                }
                // std::cout << std::endl;

                // for_each(time_sim_map.begin(), time_sim_map.end(), [](const auto& map) { std::cout << map << std::endl; });

                // std::cout << "Tn" << timeNext << std::endl;

            }

            //! It clears the messages from all the ports of child components.
            void clear() override {
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

#elif defined (FLAT)
    namespace cadmium {
       
        //! DEVS sequential coordinator class.
        class Coordinator: public AbstractSimulator {
        private:
            std::shared_ptr<Coupled> model;                              //!< Pointer to coupled model of the coordinator.
            std::vector<std::shared_ptr<AbstractSimulator>> simulators;  //!< Vector of child simulators.

            static constexpr double inf = std::numeric_limits<double>::infinity();

        public:
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

            // ─────────────────── basic getters/boilerplate ────────────────
            std::shared_ptr<Component>      getComponent() const override { return model; }
            std::shared_ptr<Coupled>        getCoupled()  const           { return model; }
            const std::vector<std::shared_ptr<AbstractSimulator>>&      getSubcomponents()            { return simulators; }

            long setModelId(long next) override {
                modelId = next++;
                for (auto& s : simulators) next = s->setModelId(next);
                return next;
            }

            void start(double t) override { timeLast = t;  for (auto& s : simulators) s->start(t);  }
            void stop (double t) override { timeLast = t;  for (auto& s : simulators) s->stop(t);}

            // ─────────────────────── collection ───────────────────────────

            /**
             * It collects all the output messages and propagates them according to the ICs and EOCs.
             * @param time new simulation time.
             */
            bool collection(double time) override {
                if (time >= timeNext) {
                    std::for_each(simulators.begin(), simulators.end(), [time](auto& s) { s->collection(time); });
                    for (auto& [portFrom, portTo]: model->getSerialICs()) {
                        portTo->propagate(portFrom);
                    }
                    for (auto& [portFrom, portTo]: model->getSerialEOCs()) {
                        portTo->propagate(portFrom);
                    }
                }
                return true;
            }

            // ─────────────────────── transition ───────────────────────────

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
                for(auto& s : simulators) {
                    s->clear();
                }
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
#elif defined (DIRECT)
    namespace cadmium {   
        //! DEVS sequential coordinator class.
        class Coordinator: public AbstractSimulator {
        private:
            std::shared_ptr<Coupled> model;                              //!< Pointer to coupled model of the coordinator.
            std::vector<std::shared_ptr<AbstractSimulator>> simulators;  //!< Vector of child simulators.

            #ifndef NO_LOGGING
                std::shared_ptr<Logger> logger; 
            #endif

            struct model_time_t {
                std::shared_ptr<AtomicInterface> model;
                double Tn;
                double Tl;
                long modelId;

                model_time_t(std::shared_ptr<AtomicInterface> m, double t): model(m), Tn(t), Tl(0.0) {}
            };
            std::vector<model_time_t*> models;

            static constexpr double inf = std::numeric_limits<double>::infinity();

        public:
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
                        models.push_back(new model_time_t(atomic, (timeLast + atomic->timeAdvance())));
                        simulator = std::make_shared<Simulator>(atomic, time);
                    }
                    // simulators.push_back(simulator);

                    timeNext = std::min(timeNext, simulator->getTimeNext());
                }

            }

            // ─────────────────── basic getters/boilerplate ────────────────
            std::shared_ptr<Component>      getComponent() const override { return model; }
            std::shared_ptr<Coupled>        getCoupled()  const           { return model; }
            const std::vector<std::shared_ptr<AbstractSimulator>>&      getSubcomponents()            { return simulators; }

            long setModelId(long next) override {
                modelId = next++;
                for (auto& s : models) s->modelId = next++;
                return next;
            }

            void start(double t) override { timeLast = t;
                // #ifndef NO_LOGGING
                //     if (logger != nullptr) {
                //         for (auto& s : models) logger->logState(s.Tl, s.modelId, s.model->getId(), s.model->logState());
                //     }
                // #endif
            }
            void stop (double t) override { timeLast = t; 
                for(auto v : models) {
                    delete v;
                }
                // #ifndef NO_LOGGING
                //     if (logger != nullptr) {
                //         for (auto& s : models) logger->logState(s.Tl, s.modelId, s.model->getId(), s.model->logState());
                //     }
                // #endif
            }

            // ─────────────────────── collection ───────────────────────────

            /**
             * It collects all the output messages and propagates them according to the ICs and EOCs.
             * @param time new simulation time.
             */
            bool collection(double time) override {
                if (time >= timeNext) {
                    
                    for(auto& s : models) {
                        if(time >= s->Tn) {
                            s->model->output();
                        }
                    }
                    
                    for (auto& [portFrom, portTo]: model->getSerialICs()) {
                        portTo->propagate(portFrom);
                    }
                }
                return true;
            }

            // ─────────────────────── transition ───────────────────────────

            /**
             * It propagates input messages according to the EICs and triggers the state transition function of child components.
             * @param time new simulation time.
             */
            void transition(double time) override {
                timeLast = time;
                timeNext = std::numeric_limits<double>::infinity();
                for (auto& sim: models) {
                    const auto inEmpty = sim->model->inEmpty();

                    if(sim->Tn > time && inEmpty){
                        timeNext = std::min(timeNext, sim->Tn);
                        continue;
                    }
                    
                    if(inEmpty) {
                        sim->model->internalTransition();
                    } else {
                        auto e = time - sim->Tl;
                        (time < sim->Tn) ? sim->model->externalTransition(e) : sim->model->confluentTransition(e);
                    }

                    #ifndef NO_LOGGING
                        if (logger != nullptr) {
                            logger->logModel(time, sim->modelId, sim->model, time >= sim->Tn);
                        }
                    #endif
                    sim->Tl = time;
                    sim->Tn = time + sim->model->timeAdvance();

                    sim->model->clearPorts();

                    timeNext = std::min(timeNext, sim->Tn);
                }

            }

            //! It clears the messages from all the ports of child components.
            void clear() override {
                // for(auto& s : simulators) {
                //     s->clear();
                // }
                // model->clearPorts();
            }

        #ifndef NO_LOGGING
            /**
             * It sets the logger to all the child components.
             * @param log pointer to the new logger.
             */
            void setLogger(const std::shared_ptr<Logger>& log) override {
                std::for_each(simulators.begin(), simulators.end(), [log](auto& s) { s->setLogger(log); });
                logger = log;
            }
        #endif
        };
    }
#endif


#endif // CADMIUM_SIMULATION_CORE_COORDINATOR_HPP_