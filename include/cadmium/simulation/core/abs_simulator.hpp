/**
 * Abstract simulator.
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

#ifndef CADMIUM_SIMULATION_CORE_ABS_SIMULATOR_HPP_
#define CADMIUM_SIMULATION_CORE_ABS_SIMULATOR_HPP_

#include <limits>
#include <memory>
#ifndef NO_LOGGING
    #include "../logger/logger.hpp"
#endif
#include "../../modeling/devs/component.hpp"

namespace cadmium {
    //! Abstract simulator class.
    class AbstractSimulator {
     protected:
        long modelId;     //!< Model identification number.
        double timeLast;  //!< Last simulation time.
        double timeNext;  //!< Next simulation time.
     public:
        /**
         * Constructor function.
         * @param time initial simulation time.
         */
        explicit AbstractSimulator(double time): modelId(), timeLast(time), timeNext(std::numeric_limits<double>::infinity()) {}

        //! default destructor function.
        virtual ~AbstractSimulator() = default;

        //! @return last simulation time.
        [[nodiscard]] double getTimeLast() const {
            return timeLast;
        }

        //! @return next simulation time.
        [[nodiscard]] double getTimeNext() const {
            return timeNext;
        }

        //! @return pointer to the component corresponding to the abstract simulator.
        [[nodiscard]] virtual std::shared_ptr<Component> getComponent() const = 0;

        /**
         * Sets the model number ID.
         * @param next corresponding model ID number.
         * @return the next model ID number.
         */
        virtual long setModelId(long next) = 0;

    #ifndef NO_LOGGING
        /**
         * Sets a logger (atomic states and output messages).
         * @param newLogger pointer to the new logger.
         */
        virtual void setLogger(const std::shared_ptr<Logger>&  newLogger) = 0;
    #endif

        /**
         * It performs all the tasks needed before the simulation.
         * @param time initial simulation time.
         */
        virtual void start(double time) = 0;

        /**
         * It performs all the tasks needed after the simulation.
         * @param time last simulation time.
         */
        virtual void stop(double time) = 0;

        /**
         * It executes the model collection function.
         * @param time simulation time.
         */
        virtual void collection(double time) = 0;

        /**
         * It executes the model transition function.
         * @param time
         */
        virtual void transition(double time) = 0;

        //! it clears the input and output ports of the model.
        virtual void clear() = 0;
    };
}

#endif // CADMIUM_SIMULATION_CORE_ABS_SIMULATOR_HPP_
