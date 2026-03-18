/**
 * DEVS simulator.
 * Copyright (C) 2026 Sasisekhar Govind
 * ARSLab - Carleton University
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

#ifndef CADMIUM_SIMULATION_REMOTE_SIMULATOR_HPP_
#define CADMIUM_SIMULATION_REMOTE_SIMULATOR_HPP_

#include <memory>
#include <iostream>
#include <utility>
#include "abs_simulator.hpp"
#include "../../exception.hpp"
#ifndef NO_LOGGING
    #include "../logger/logger.hpp"
#endif
#include "../../modeling/distributed/atomic.hpp"

#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>

#include <iomanip>

namespace cadmium {

    void print_bytes(std::vector<std::byte> bytes) {
        // Set output stream to hexadecimal, uppercase, and fill with '0'
        std::cout << std::hex << std::uppercase << std::setfill('0');

        for (auto const b : bytes) {
            // Cast each std::byte to int before printing
            std::cout << std::setw(2) << std::to_integer<int>(b) << ' ';
        }

        // Reset stream manipulators to default decimal and no fill
        std::cout << std::dec << std::setfill(' ');
    }

    //! DEVS simulator.
    class Simulator: public AbstractSimulator {
     private:
        std::shared_ptr<AtomicInterface> model;  //!< Pointer to the corresponding atomic DEVS model.
    #ifndef NO_LOGGING
        std::shared_ptr<Logger> logger;
    #endif
    int connectionSocket;
    std::string recvBuffer;

    std::pair<std::string, double> split_delim(std::string input, std::string delim) {
        std::vector<std::string> tokens;
        std::size_t pos = 0;
        std::string token;
        while ((pos = input.find(delim)) != std::string::npos) {
            token = input.substr(0, pos);
            tokens.push_back(token);
            input.erase(0, pos + delim.length());
        }
        tokens.push_back(input);

        std::string event = "err";
        double time = -1;

        if(tokens.size() != 2) {
            std::cerr << "Malformed input from server" << std::endl;
            return {event, time};
        } else {
            event = tokens.at(0);
            time = stod(tokens.at(1));
        }

        return {event, time};
    }
    
    void send_event(const std::string& event, double time) {
        std::string msg = event + "," + std::to_string(time) + "\n";
        send(connectionSocket, msg.data(), msg.size(), 0);
    }

    std::string recv_line() {
        char temp[256];
        while (true) {
            size_t pos = recvBuffer.find('\n');
            if (pos != std::string::npos) {
                std::string line = recvBuffer.substr(0, pos);
                recvBuffer.erase(0, pos + 1);
                return line;
            }

            ssize_t n = recv(connectionSocket, temp, sizeof(temp), 0);
            if (n <= 0) {
                throw std::runtime_error("connection closed");
            }
            recvBuffer.append(temp, n);
        }
    }

    void recv_all(int sock, void* data, size_t n) {
        size_t total = 0;
        char* ptr = static_cast<char*>(data);

        while (total < n) {
            ssize_t r = recv(sock, ptr + total, n - total, 0);
            if (r <= 0) throw std::runtime_error("socket closed");
            total += r;
        }
    }

    public:
    double Tn;
    double Tl;
    std::vector<Simulator*> influencees;
    bool imm;
    bool schedulable;

    #ifndef NO_LOGGING
        /**
         * Constructor function.
         * @param model pointer to the atomic model.
         * @param time initial simulation time.
         */
        Simulator(std::shared_ptr<AtomicInterface> model, double time, int serverSocket): 
        AbstractSimulator(time), model(std::move(model)), logger(), imm(false), schedulable(false), Tl(0) {
            if (this->model == nullptr) {
                throw CadmiumSimulationException("no atomic model provided");
            }

            connectionSocket = accept(serverSocket, nullptr, nullptr);

            send_event("init", time);
            // std::cout << "waiting for tn, current time = " << time << std::endl;
            auto [done, timeN] = split_delim(recv_line(), ",");
            // std::cout << "done waiting, tn = " << timeN << std::endl;

            timeNext = timeN;
            Tn = timeNext;

        }
    #else
        /**
         * Constructor function.
         * @param model pointer to the atomic model.
         * @param time initial simulation time.
         */
        // Simulator(std::shared_ptr<AtomicInterface> model, double time): AbstractSimulator(time), model(std::move(model)) {
        //     if (this->model == nullptr) {
        //         throw CadmiumSimulationException("no atomic model provided");
        //     }
        //     timeNext = timeLast + this->model->timeAdvance();
        //     Tn = timeNext;
        //     Tl = timeLast;
        // }
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
            send_event("start", time);
            timeLast = time;
            Tl = timeLast;
        #ifndef NO_LOGGING
            if (logger != nullptr) {
                // logger->logState(timeLast, modelId, model->getId(), model->logState());
                std::cout << "Remote sim started for: " << model->getId() << std::endl;
            }
        #endif
        };

        /**
         * It performs all the operations after running a simulation.
         * @param time final simulation time.
         */
        void stop(double time) override {
            send_event("stop", time);
            close(connectionSocket);
            timeLast = time;
            Tl = time;
        #ifndef NO_LOGGING
            if (logger != nullptr) {
                // logger->logState(timeLast, modelId, model->getId(), model->logState());
                std::cout << "Remote sim closed for: " << model->getId() << std::endl;
            }
        #endif
        }

        void send_yt() {
            send_event("y", timeNext);
            auto [done, timeN] = split_delim(recv_line(), ",");
            for(const auto& port : model->getInPorts()) {
                if(!port->empty()) {
                    auto bytes = port->getBagAsBytes();
                    auto port_id = port->getId();
                    uint64_t size_payload = bytes.size();
                    uint64_t size_id = port_id.size();

                    send(connectionSocket, &size_id, sizeof(size_id), 0);           // header
                    send(connectionSocket, port_id.data(), port_id.size(), 0);      // body
                    send(connectionSocket, &size_payload, sizeof(size_payload), 0); // header
                    send(connectionSocket, bytes.data(), bytes.size(), 0);          // body
                }
            }
            uint64_t end_token = 0;
            send(connectionSocket, &end_token, sizeof(end_token), 0);
        }

        /**
         * It calls to the output function of the atomic model.
         * @param time current simulation time.
         */
        void collection(double time) override {
            send_event("collection", time);
            

            while(true) {
                uint64_t size_id, size_payload;
                char port_id[256];
                

                recv_all(connectionSocket, &size_id, sizeof(size_id));
                if(size_id == 0) {
                    break;
                } 
                recv_all(connectionSocket, port_id, size_id);
                port_id[size_id] = '\0';

                recv_all(connectionSocket, &size_payload, sizeof(size_payload));
                std::vector<std::byte> buf(size_payload);
                recv_all(connectionSocket, buf.data(), size_payload);
                model->getOutPort(port_id)->setBagAsBytes(buf);
            }
        }

        /**
         * It calls to the corresponding state transition function.
         * @param time current simulation time.
         */
        void transition(double time) override {

            send_event("transition", time);
            auto [done, timeN] = split_delim(recv_line(), ",");
            
            timeLast = time;
            timeNext = timeN;

            Tl = timeLast;
            Tn = timeNext;
        }

        //! It clears all the ports of the model.
        void clear() override {
            model->clearPorts();
            send_event("clear", .0);
        }
    };
}

#endif // CADMIUM_SIMULATION_CORE_SIMULATOR_HPP_
