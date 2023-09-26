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

#include <fstream>
#include <iostream>
#include <optional>
#include "../modeling/devs/atomic.hpp"
#include "../exception.hpp"

namespace cadmium::lib {
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
         * - The time of event is the first value.
         * - The rest corresponds to a serialized version of the event to be sent.
         * - Messages are processed according to the >> operator.
         */
        explicit EventParser(const char* file_path) {
            file.open(file_path);
            if(!file.is_open()) {
                throw CadmiumModelException("input event file could not be opened");
            }
        };

        /**
         * It reads the next line of the input file and parses the time when the event happens and its value.
         * If the parser has reached the end of the file, it returns infinity and an empty event.
         * @return tuple <next time, next event>.
         */
        std::pair<double, std::optional<MSG>> nextTimedInput() {
            // Default return values: infinity and none
            double sigma = std::numeric_limits<double>::infinity();
            std::optional<MSG> contents = std::optional<MSG>();
            if (file.is_open() && !file.eof()) {
                file >> sigma; // read time of next message
                MSG value;
                file >> value; // read values to go into message
                contents.template emplace(value);
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
        EventParser<MSG> parser;  //!< Input events parser.
        std::optional<MSG> lastInputRead;  //!< las input messages read from the file.
        double clock;  //!< Current simulation time.
        double sigma;  //!< Time to wait before outputting the next event.

        /**
         * Processor state constructor. By default, the processor is idling.
         * @param filePath path to the file containing the events.
         */
        explicit IEStreamState(const char* filePath): parser(EventParser<MSG>(filePath)), lastInputRead(), clock(), sigma() {
            auto [nextTime, nextEvent] = parser.nextTimedInput();
            sigma = nextTime;
            lastInputRead = nextEvent;
        }
    };

    /**
     * Insertion operator for ProcessorState objects. It only displays the value of sigma.
     * @param out output stream.
     * @param s state to be represented in the output stream.
     * @return output stream with sigma already inserted.
     */
    template<typename MSG>
    std::ostream& operator<<(std::ostream &out, const IEStreamState<MSG>& state) {
        out << state.sigma;
        return out;
    }

    /**
     * Atomic DEVS model for injecting input events from a file.
     * @tparam MSG message type of the events to be injected.
     */
    template<typename MSG>
    class IEStream : public Atomic<IEStreamState<MSG>> {
     public:
        Port<MSG> out;

        /**
         * Constructor function.
         * @param id ID of the new input event stream model.
         * @param filePath path to the file with the events to be injected
         */
        IEStream(const std::string& id, const char* filePath): Atomic<IEStreamState<MSG>>(id, IEStreamState<MSG>(filePath)) {
            out = Atomic<IEStreamState<MSG>>::template addOutPort<MSG>("out");
        }

        /**
         * Read the next line of file and when that message should be sent
         * if the time to be sent is in the past, then passivate model
         * @param state reference to the current state of the model.
         */
        void internalTransition(IEStreamState<MSG>& state) const override {
            state.clock += state.sigma;
            while(true) {  // loop to ignore outdated events
                auto [nextTime, nextEvent] = state.parser.nextTimedInput();
                if (nextTime < state.clock) {
                    std::cerr << "Outdated event in input file. Scheduled time: " << nextTime << " Current time: " << state.clock << ". " << nextEvent.value() << std::endl;
                } else {
                    state.sigma = nextTime - state.clock;
                    state.lastInputRead = nextEvent;
                    break;
                }
            };
        }

        /**
         * External transitions should never occur (no input ports).
         * @param state reference to the current model state.
         * @param e time elapsed since the last state transition function was triggered.
         */
        void externalTransition(IEStreamState<MSG>& state, double e) const override {
            // External Events should not occur for this model
            state.clock += e;
            state.sigma -= e;
        }

        /**
         * It outputs the next message
         * @param state reference to the current model state.
         */
        void output(const IEStreamState<MSG>& state) const override {
            if(state.lastInputRead.has_value()){
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
}  //namespace cadmium::lib

#endif //CADMIUM_LIB_IESTREAM_HPP_
