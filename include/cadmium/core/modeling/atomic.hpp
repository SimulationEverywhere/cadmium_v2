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
#include <utility>
#include <vector>
#include "component.hpp"
#include "port.hpp"

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
    };

    template <typename S>
    class Atomic: public AbstractAtomic {
     protected:
        S state;
     public:
        explicit Atomic(std::string id, S initialState) : state(initialState), AbstractAtomic(std::move(id)) {};

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
            this->externalTransition(state, e, interface->inPorts);
        }

        void confluentTransition(double e) override {
            this->confluentTransition(state, e, interface->inPorts);
        }

        void output() override {
            this->output(state, interface->outPorts);
        }

        [[nodiscard]] double timeAdvance() const override {
            return this->timeAdvance(state);
        }
    };
}

#endif //_CADMIUM_CORE_MODELING_ATOMIC_HPP_
