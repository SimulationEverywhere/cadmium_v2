/**
 * Coordinator for executing simulations in parallel.
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

#ifndef CADMIUM_SIMULATION_LOGGER_MUTEX_HPP_
#define CADMIUM_SIMULATION_LOGGER_MUTEX_HPP_

#include <mutex>
#include <type_traits>
#include "logger.hpp"

namespace cadmium {
    //! Cadmium logger guarded by a mutex. It is necessary for parallel execution.
    template<typename T>
    class MutexLogger : public Logger {
        static_assert(std::is_base_of<Logger, T>::value, "T must inherit cadmium::Logger");
     private:
        T logger;
        std::mutex mutex;  //!< Mutex for synchronizing parallel logging.
     public:
        //! Constructor function.
        explicit MutexLogger(T logger) : Logger(), logger(std::move(logger)), mutex() {
        }

        //! Virtual method to execute any task prior to the simulation required by the logger.
        void start() override {
            mutex.lock();
            logger.start();
            mutex.unlock();
        };

        //! Virtual method to execute any task after the simulation required by the logger.
        void stop() override {
            mutex.lock();
            logger.stop();
            mutex.unlock();
        };

        /**
         * Virtual method to log the simulation time after a simulation step. By default, it does nothing.
         * @param time new simulation time.
         */
        void logTime(double time) override {
            mutex.lock();
            logger.logTime(time);
            mutex.unlock();
        }

        /**
         * Virtual method to log atomic models' output messages.
         * @param time current simulation time.
         * @param modelId ID of the model that generated the output message.
         * @param modelName name of the model that generated the output message.
         * @param portName name of the model port in which the output message was created.
         * @param output string representation of the output message.
         */
        void logOutput(double time,
            long modelId,
            const std::string& modelName,
            const std::string& portName,
            const std::string& output) override {
            mutex.lock();
            logger.logOutput(time, modelId, modelName, portName, output);
            mutex.unlock();
        }

        /**
         * Virtual method to log atomic models' states.
         * @param time current simulation time.
         * @param modelId ID of the model that generated the output message.
         * @param modelName name of the model that generated the output message.
         * @param state string representation of the state.
         */
        void logState(double time, long modelId, const std::string& modelName, const std::string& state) override {
            mutex.lock();
            logger.logState(time, modelId, modelName, state);
            mutex.unlock();
        }

        /**
          * method for logging the state of an atomic model after executing its output and/or transition functions.
          * @param time current simulation time.
          * @param modelId ID of the model that generated the output message.
          * @param modelName name of the model that generated the output message.
          * @param logOutput if true, it will print the output messages of the model.
          */
        void logModel(double time,
            long modelId,
            const std::shared_ptr<AtomicInterface>& model,
            bool logOutput) override {
            mutex.lock();
            logger.logModel(time, modelId, model, logOutput);
            mutex.unlock();
        }
    };
}

#endif //CADMIUM_SIMULATION_LOGGER_MUTEX_HPP_
