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
#include <execution>

namespace cadmium {   
    //! DEVS sequential coordinator class.
    class Coordinator: public AbstractSimulator {
    private:
        std::shared_ptr<Coupled> model;                              //!< Pointer to coupled model of the coordinator.
        std::vector<std::shared_ptr<AbstractSimulator>> simulators;  //!< Vector of child simulators.

        #ifndef NO_LOGGING
            std::shared_ptr<Logger> logger; 
        #endif

        struct simulator_t {
            std::shared_ptr<AtomicInterface> model;
            double Tn;
            double Tl;
            long modelId;
            std::vector<simulator_t*> influencees;
            bool imm;
            bool schedulable;

            simulator_t(std::shared_ptr<AtomicInterface> m, double t):  model(m), Tn(t), Tl(0.0), 
                                                                        modelId(0), influencees(), 
                                                                        imm(false), schedulable(false) {}
        };
        std::vector<std::shared_ptr<simulator_t>> models;

        std::vector<simulator_t*> imminent;

        static constexpr double inf = std::numeric_limits<double>::infinity();

        void check_schedulable(std::vector<simulator_t*> imm_list) {
            for(auto& i : imm_list) {
                if(i->imm) {
                    /* SOME TEST TO CHECK SCHEDULABILITY*/
                    i->schedulable = true;
                }
            }
        }

    public:
        Coordinator(std::shared_ptr<Coupled> model, double time): AbstractSimulator(time), model(std::move(model)) {
            if (this->model == nullptr) {
                throw CadmiumSimulationException("no coupled model provided");
            }
            timeLast = time;
            for (auto& [componentId, component]: this->model->getComponents()) {
                double Tn = inf;
                auto coupled = std::dynamic_pointer_cast<Coupled>(component);
                if (coupled == nullptr) {
                    auto atomic = std::dynamic_pointer_cast<AtomicInterface>(component);
                    if (atomic == nullptr) {
                        throw CadmiumSimulationException("component is not a coupled nor atomic model");
                    }

                    Tn = (timeLast + atomic->timeAdvance());

                    auto m = std::make_shared<simulator_t>(atomic, Tn);

                    models.push_back(m);
                }
                timeNext = std::min(timeNext, Tn);
            }

            for(auto& [portFrom, portTo] : this->model->getSerialICs()) {
                auto parentFrom = dynamic_cast<const AtomicInterface*>(portFrom->getParent());
                auto parentTo = dynamic_cast<const AtomicInterface*>(portTo->getParent());
                for(auto& m : models) {
                    if(parentFrom && parentFrom == m->model.get()){
                        for(auto& inf_m: models) {
                            if(parentTo && parentTo == inf_m->model.get()){
                                m->influencees.push_back(inf_m.get());
                            }
                        }
                    }
                }
            }

            #ifdef DEBUG
                std::cout << "Printing influencees: " << std::endl;
                std::for_each(models.begin(), models.end(), [](const auto& m){
                    std::cout << "\t Model: " << m->model->getId();
                    std::cout << ((m->influencees.empty())? " has no influencees" : " has influencees:") << std::endl;
                    std::for_each(m->influencees.begin(), m->influencees.end(), [](const auto& inf){
                        std::cout << "\t\t" << inf->model->getId() << std::endl;
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
                    std::cout << "\b\b} -> " <<  portTo->getId() << "(" << portTo->getParent()->getId() << ")" << std::endl;
                    
                }

                std::cout << "\nMapped IC (inverse): " << std::endl;
                for(const auto& [portFrom, portTos] : this->model->getInverseICs()) {
                    std::cout << "\t" <<  portFrom->getId() << "(" << portFrom->getParent()->getId() << ")" << "-> {";
                    for(const auto& portTo : portTos) {
                        std::cout << portTo->getId() << "(" << portTo->getParent()->getId() << ")" << ", ";
                    }
                    std::cout << "\b\b}" << std::endl;
                    
                }
            #endif

            for(auto& m : models) {
                if(m->Tn == timeNext) {
                    m->imm = true;
                    imminent.push_back(m.get());
                }
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
            #ifndef NO_LOGGING
                if (logger != nullptr) {
                    for (auto& s : models) logger->logState(s->Tl, s->modelId, s->model->getId(), s->model->logState());
                }
            #endif
        }
        void stop (double t) override { timeLast = t; 
            #ifndef NO_LOGGING
                if (logger != nullptr) {
                    for (auto& s : models) logger->logState(s->Tl, s->modelId, s->model->getId(), s->model->logState());
                }
            #endif
        }

        // ─────────────────────── collection ───────────────────────────

        /**
         * It collects all the output messages and propagates them according to the ICs (no EOCs since flat).
         * @param time new simulation time.
         */
        bool collection(double time) override {
            if (time >= timeNext) {

                const auto cache = imminent;

                for(auto& s : cache) {
                        s->model->output();
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

            // std::cout << "Imminent at time " << time << " s:"  << std::endl;
            // std::for_each(imminent.begin(), imminent.end(), [](const auto& m){ std::cout << "\t" << m->model->getId() << std::endl; });

            for (auto& sim: imminent) {
                const auto inEmpty = sim->model->inEmpty();
                
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

                sim->imm = false;
            }

            std::for_each(std::execution::par, models.begin(), models.end(), [&](const auto& x){ timeNext = std::min(timeNext, x->Tn); });
            // std::for_each(models.begin(), models.end(), [&](const auto& x){ timeNext = std::min(timeNext, x->Tn); });

            imminent.clear();
            for(auto& m : models) {
                if(m->Tn == timeNext) {
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
            std::for_each(simulators.begin(), simulators.end(), [log](auto& s) { s->setLogger(log); });
            logger = log;
        }
    #endif
    };
}


#endif // CADMIUM_SIMULATION_CORE_COORDINATOR_HPP_