/**
 * Virtual interface for implementing loggers in Cadmium 2
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2021-present Román Cárdenas Rodríguez
 * ARSLab - Carleton University
 * GreenLSI - Polytechnic University of Madrid
 */

#ifndef CADMIUM_SIMULATION_LOGGER_LOGGER_HPP_
#define CADMIUM_SIMULATION_LOGGER_LOGGER_HPP_

#include <string>
#include <mutex>
#include <optional>
#include "../../modeling/devs/atomic.hpp"

namespace cadmium {
    //! Cadmium Logger abstract class.
    class Logger {
     private:
        // std::optional<std::mutex> mutex;  //!< Mutex for enabling a good parallel execution.
     public:
        //! Constructor function.
        Logger()/*: mutex()*/ {}

        //! Destructor function.
        virtual ~Logger() = default;

        // //! Creates a new mutex for parallel execution.
        // void createMutex() {
        //     mutex.emplace();
        // }

        // //! Removes a mutex for sequential execution.
        // void removeMutex() {
        //     mutex.reset();
        // }

        // //! It locks the logger mutex (if needed).
        // inline void lock() {
        //     if (mutex.has_value()) {
        //         mutex->lock();
        //     }
        // }

        // //! It unlocks the logger mutex (if needed).
        // inline void unlock() {
        //     if (mutex.has_value()) {
        //         mutex->unlock();
        //     }
        // }

        //! Virtual method to execute any task prior to the simulation required by the logger.
        virtual void start() = 0;

        //! Virtual method to execute any task after the simulation required by the logger.
        virtual void stop() = 0;

        /**
         * Virtual method to log the simulation time after a simulation step. By default, it does nothing.
         * @param time new simulation time.
         */
        virtual void logTime(double time) {}

        /**
         * Virtual method to log atomic models' output messages.
         * @param time current simulation time.
         * @param modelId ID of the model that generated the output message.
         * @param modelName name of the model that generated the output message.
         * @param portName name of the model port in which the output message was created.
         * @param output string representation of the output message.
         */
        virtual void logOutput(double time,
            long modelId,
            const std::string& modelName,
            const std::string& portName,
            const std::string& output) = 0;

        /**
         * Virtual method to log atomic models' states.
         * @param time current simulation time.
         * @param modelId ID of the model that generated the output message.
         * @param modelName name of the model that generated the output message.
         * @param state string representation of the state.
         */
        virtual void logState(double time, long modelId, const std::string& modelName, const std::string& state) = 0;

        /**
         * method for logging the state of an atomic model after executing its output and/or transition functions.
         * @param time current simulation time.
         * @param modelId ID of the model that generated the output message.
         * @param modelName name of the model that generated the output message.
         * @param logOutput if true, it will print the output messages of the model.
         */
        virtual void logModel(double time,
            long modelId,
            const std::shared_ptr<AtomicInterface>& model,
            bool logOutput) {
            if (logOutput) {
                for (const auto& outPort: model->getOutPorts()) {
                    for (std::size_t i = 0; i < outPort->size(); ++i) {
                        this->logOutput(time, modelId, model->getId(), outPort->getId(), outPort->logMessage(i));
                    }
                }
            }
            this->logState(time, modelId, model->getId(), model->logState());
        }
    };
}

#endif // CADMIUM_SIMULATION_LOGGER_LOGGER_HPP_
