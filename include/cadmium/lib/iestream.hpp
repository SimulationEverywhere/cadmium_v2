/**
 * CSV logger.
 * Copyright (C) 2022  Jon Menard
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

#ifndef CADMIUM_LIB_IESTREAM_HPP_
#define CADMIUM_LIB_IESTREAM_HPP_

#include <iostream>
#include <optional>
#include "../core/modeling/atomic.hpp"
#include "../core/exception.hpp"

namespace cadmium {
    /**
     * Input event parser.
     * @tparam MSG data type of the event to be parsed
     */
    template<typename MSG>
    class EventParser {
     private:
        std::ifstream file; //!< input file stream of the file with all the events to be injected.
     public:
        /**
         * Constructor function.
         * @param file_path path to the file containing the events.
         * The format of this file is the following:
         * - Each line corresponds to one event.
         * - First, we need to
         */
        explicit EventParser(const char* file_path) {
            file.open(file_path);
            if(!file.is_open()) {
                throw CadmiumModelException("input event file could not be opened");
            }
        };

        // TODO Don't you have to define a special destructor that closes the file?

        // TODO What do you think about this other implementation with optionals and avoiding exceptions?
        std::pair<double, std::optional<MSG>> nextTimedInput() {
            // Default return values: infinity and none
            double sigma = std::numeric_limits<double>::infinity();
            std::optional<MSG> contents = std::optional<MSG>();
            if(file.is_open() && !file.eof()) {
                file >> sigma; // read time of next message
                MSG value;
                file >> value; // read values to go into message
                contents.template emplace(value);
                // std::cout << next_time << "  " << contents << std::endl; // uncomment to debug input file
            }
            return std::make_pair(sigma,contents);
        }
    };

    /**
     * Class for representing the Input Event Stream DEVS model state.
     * @tparam MSG data type of the event to be parsed
     */
    template<typename MSG>
    struct IEStreamState {
        EventParser<MSG> parser;
        std::optional<MSG> lastInputRead; // TODO use camelCaseNotation in the future
        double clock;
        double sigma;
        // TODO I got rid of initialized (look below)

        /**
         * Processor state constructor. By default, the processor is idling.
         * @param file_path path to the file containing the events.
         */
        explicit IEStreamState(const char* file_path): parser(EventParser<MSG>(file_path)),
                                                       lastInputRead(), clock(), sigma() {}
    };

    /**
     * Insertion operator for ProcessorState objects. It only displays the value of sigma.
     * @param out output stream.
     * @param s state to be represented in the output stream.
     * @return output stream with sigma already inserted.
     */
    template<typename MSG>
    std::ostream& operator<<(std::ostream &out, const IEStreamState<MSG>& s) {
        // TODO why do you start with a comma? this might be a problem for CSV files
        // TODO I recommend you to use something like <> or {} to encapsulate the state in something parseable
        out << "," << s.clock << " ," << s.sigma << ",";
        if (s.lastInputRead.has_value()) {
            out << s.lastInputRead.value();
        } else {
            out << "none";
        }
        return out;
    }

    /**
     * Atomic DEVS model for injecting input events from a file.
     * @tparam MSG message type of the events to be injected.
     */
    template<typename MSG>
    class Iestream : public Atomic<IEStreamState<MSG>> {
     public:
        Port<MSG> out;

        /**
         * Constructor function.
         * @param id ID of the new input event stream model.
         * @param file_path path to the file with the events to be injected
         */
        Iestream(const std::string& id, const char* file_path): Atomic<IEStreamState<MSG>>(id, IEStreamState<MSG>(file_path)) {
            out = Atomic<IEStreamState<MSG>>::template addOutPort<MSG>("out");
        }

        /**
         * Read the next line of file and when that message should be sent
         * if the time to be sent is in the past, then passivate model
         * @param state reference to the current state of the model.
         */
         // TODO changed s to state to comply with the other examples
        void internalTransition(IEStreamState<MSG>& state) const override {
            state.clock += state.sigma;

            auto [nextTime, nextEvent] = state.parser.nextTimedInput();  // TODO look at this fancy way of deconstructing tuples
            if (nextTime < state.clock) {  // TODO I don't modify the model until I'm sure that it is correct
                throw CadmiumModelException("Events are not properly sorted in input file");
            }
            state.sigma = nextTime - state.clock;
            state.lastInputRead = nextEvent;
        }

        /**
         * External transitions should never occur (no input ports).
         * @param state reference to the current model state.
         * @param e time elapsed since the last state transition function was triggered.
         */
        void externalTransition(IEStreamState<MSG>& state, double e) const override {
            state.clock += e;
            state.sigma -= e;
        }

        /**
         * It outputs the next message
         * @param state reference to the current model state.
         * @param y reference to the atomic model output port set.
         */
        void output(const IEStreamState<MSG>& state) const override {
            if(state.lastInputRead.has_value()){  // TODO this is why optional is nice
                out->addMessage(state.lastInputRead.value());
            }
        }

        /**
         * It returns the value of State::sigma.
         * @param state reference to the current model state.
         * @return the sigma value.
         */
        [[nodiscard]] double timeAdvance(const IEStreamState<MSG>& state) const override {
            return state.sigma;
        }
    };
}  //namespace cadmium

#endif //CADMIUM_LIB_IESTREAM_HPP_