/**
 * <one line to give the program's name and a brief idea of what it does.>
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

#ifndef _CADMIUM_CORE_MODELING_ATOMIC_HPP_
#define _CADMIUM_CORE_MODELING_ATOMIC_HPP_

#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "component.hpp"
#include "port.hpp"
#include "../modeling/atomic.hpp"

#include "cadmium/core/modeling/message.hpp"

// TODO change time type

namespace cadmium {

    class AbstractAtomic: public Component {
     public:
        explicit AbstractAtomic(std::string id) : Component(std::move(id)) {}
        virtual void internalTransition() = 0;
        virtual void externalTransition(double e) = 0;
        virtual void confluentTransition(double e) = 0;
        virtual void output() = 0;
        [[nodiscard]] virtual double timeAdvance() const = 0;  // TODO change time type
		virtual void logState(std::shared_ptr<Logger>& logger, double time, long modelId) const = 0;
        virtual MessageType get_message_type() = 0;
    };

    template <typename S>
    class Atomic: public AbstractAtomic {
     protected:
        S state;
     public:
        explicit Atomic(std::string id, std::string className, S initialState) : AbstractAtomic(std::move(id)), state(initialState) {
            this->className = std::move(className);
        }

        explicit Atomic(std::string id, S initialState) : Atomic(std::move(id), "", initialState) {}

        virtual void internalTransition(S& s) const = 0;
        virtual void externalTransition(S& s, double e, const PortSet& x) const = 0;
        virtual void output(const S& s, const PortSet& y) const = 0;
        virtual double timeAdvance(const S& s) const = 0;
        virtual void confluentTransition(S& s, double e, const PortSet& x) const {
            this->internalTransition(s);
            this->externalTransition(s, 0., x);
        }

        void internalTransition() override {
            this->internalTransition(state);
        }

        void externalTransition(double e) override {
            this->externalTransition(state, e, inPorts);
        }

        void confluentTransition(double e) override {
            this->confluentTransition(state, e, inPorts);
        }

        void output() override {
            this->output(state, outPorts);
        }

        [[nodiscard]] double timeAdvance() const override {
            return this->timeAdvance(state);
        }

		void logState(std::shared_ptr<Logger>& logger, double time, long modelId) const override {
			std::stringstream ss;
			ss << state;
			logger->logState(time, modelId, id, ss.str());
		}

        MessageType get_message_type() override {
            return Message<S>::get_message_type();
        }
    };
}

#endif //_CADMIUM_CORE_MODELING_ATOMIC_HPP_
