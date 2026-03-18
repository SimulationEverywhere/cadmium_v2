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
#include "../../modeling/distributed/atomic.hpp"
#include "../../modeling/distributed/coupled.hpp"
#include "../../modeling/distributed/component.hpp"
#include "remote_simulator.hpp"
#include <iostream>
#include <execution>

#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>

namespace cadmium {

    //! DEVS distributed coordinator class.
    class Coordinator: public AbstractSimulator {
    private:
        std::shared_ptr<Coupled> model;                              //!< Pointer to coupled model of the coordinator.
        std::vector<std::shared_ptr<Simulator>> models;
        std::vector<Simulator*> imminent;

        static constexpr double inf = std::numeric_limits<double>::infinity();

        sockaddr_in serverAddress;
        int serverSocket;



    public:
        Coordinator(std::shared_ptr<Coupled> model, double time): AbstractSimulator(time), model(std::move(model)) {
            if (this->model == nullptr) {
                throw CadmiumSimulationException("no coupled model provided");
            }

            // creating socket
            serverSocket = socket(AF_INET, SOCK_STREAM, 0);

            // specifying the address
            serverAddress.sin_family = AF_INET;
            serverAddress.sin_port = htons(8080);
            serverAddress.sin_addr.s_addr = INADDR_ANY;

            // binding socket.
            bind(serverSocket, (struct sockaddr*)&serverAddress,
                sizeof(serverAddress));

            // listening to the assigned socket
            listen(serverSocket, 10);

            timeLast = time;
            for (auto& [componentId, component]: this->model->getComponents()) {
                auto coupled = std::dynamic_pointer_cast<Coupled>(component);
                if (coupled == nullptr) {
                    auto atomic = std::dynamic_pointer_cast<AtomicInterface>(component);
                    if (atomic == nullptr) {
                        throw CadmiumSimulationException("component is not a coupled nor atomic model");
                    }

                    auto m = std::make_shared<Simulator>(atomic, time, serverSocket);

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

        void start(double t) override { timeLast = t; for (auto& s : models) { s->start(t); } }

        void stop (double t) override { 
            timeLast = t;
            for (auto& s : models) { s->stop(t); }
            close(serverSocket);
        }

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
                        if(!s->getComponent()->outEmpty()){
                            for(auto& infl: s->influencees) {
                                if(!infl->imm) {
                                    infl->imm = true;
                                    imminent.push_back(infl);
                                }
                            }
                        }
                }
                
                for (auto& [portFrom, portTo]: model->getSerialICs()) {
                    if(!portFrom->empty())
                        portTo->propagate(portFrom);

                }

                for(const auto& sim : imminent) {
                    if(!sim->getComponent()->inEmpty()) {
                        sim->send_yt();
                    }
                }

                //This is only present to handle outputs to the external world. No other EOCs should exist in flat.
                // for (auto& [portFrom, portTo]: model->getSerialEOCs()) {
                //     if(!portFrom->empty())
                //         portTo->propagate(portFrom);
                // }
            }
        }

        // ─────────────────────── transition ───────────────────────────

        /**
         * It propagates input messages according to the EICs and triggers the state transition function of child components.
         * @param time new simulation time.
         */
        void transition(double time) override {
            //This is only present to handle inputs from the external world. No other EICs should exist in flat.
            // for (auto& [portFrom, portTo]: model->getSerialEICs()) {
            //     if(!portFrom->empty())
            //         portTo->propagate(portFrom);
            // }

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

        }

        //! It clears the messages from all the ports of child components.
        void clear() override {
            // for satisfying virtual
            model->clearPorts();
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