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

#ifndef _CADMIUM_CORE_MODELING_COMPONENT_HPP_
#define _CADMIUM_CORE_MODELING_COMPONENT_HPP_

#include <exception>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "port.hpp"

namespace cadmium {

    class Component {
     protected:
		friend class Simulator;
		friend class Coordinator;

        void clearPorts() {
            inPorts.clear();
            outPorts.clear();
        }

     public:
        long uid;
        std::string id;
        std::string className;
        PortSet inPorts, outPorts;

        explicit Component(std::string id) : id(std::move(id)), inPorts(), outPorts() {};

        virtual ~Component() = default;

        virtual long setUid(long next) {
            uid = next++;
            return next;
        }

        std::shared_ptr<PortInterface> getInPort(const std::string& id) const {
            return inPorts.getPort(id);
        }

        template <typename T>
        std::shared_ptr<Port<T>> getInPort(const std::string& id) const {
            return inPorts.getPort(id);
        }

        std::shared_ptr<PortInterface> getOutPort(const std::string& id) const {
            return outPorts.getPort(id);
        }

        template <typename T>
        std::shared_ptr<Port<T>> getOutPort(const std::string& id) const {
            return outPorts.getPort(id);
        }

        void addInPort(const std::shared_ptr<PortInterface>& port) {
            inPorts.addPort(port);
            port->parent = std::make_shared<Component>(*this);
        }

		template <typename T>
		[[maybe_unused]] void addInPort(Port<T> port) {
			addInPort(std::make_shared<Port<T>>(std::move(port)));
		}

        template <typename T>
        [[maybe_unused]] void addInPort(const std::string id) {
            addInPort(std::make_shared<Port<T>>(id));
        }

        void addOutPort(const std::shared_ptr<PortInterface>& port) {
            outPorts.addPort(port);
            port->parent = std::make_shared<Component>(*this);
        }

		template <typename T>
		[[maybe_unused]] void addOutPort(Port<T> port) {
			addOutPort(std::make_shared<Port<T>>(std::move(port)));
		}

        template <typename T>
        [[maybe_unused]] void addOutPort(const std::string id) {
            addOutPort(std::make_shared<Port<T>>(id));
        }

        virtual void traverse(std::function<void(Component* c)> fn){
            fn(this);
        }

        virtual std::vector<std::shared_ptr<Component>> get_components() {
            return { };
        }
    };
}

#endif //_CADMIUM_CORE_MODELING_COMPONENT_HPP_
