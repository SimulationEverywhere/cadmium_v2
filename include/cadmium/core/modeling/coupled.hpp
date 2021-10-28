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

#ifndef _CADMIUM_CORE_MODELING_COUPLED_HPP_
#define _CADMIUM_CORE_MODELING_COUPLED_HPP_

#include <memory>
#include <tuple>
#include <typeinfo>
#include <utility>
#include <vector>
#include "component.hpp"
#include "port.hpp"

namespace cadmium {

    using coupling = std::tuple<const std::shared_ptr<PortInterface>, const std::shared_ptr<PortInterface>>;

    class Coupled: public Component {
     private:
        friend class Coordinator;
        std::vector<std::shared_ptr<Component>> components;
        std::vector<coupling> EIC;
        std::vector<coupling> IC;
        std::vector<coupling> EOC;
     public:
        explicit Coupled(std::string id): Component(std::move(id)), components(), EIC(), IC(), EOC() {};

        [[nodiscard]] std::shared_ptr<Component> getComponent(const std::string& id) const {
            for (auto const& component: components) {
                if (component->getId() == id) {
                    return component;
                }
            }
            return nullptr;
        }

		template <typename T>
		void addComponent(const T component) {
			auto compPointer = std::dynamic_pointer_cast<Component>(std::make_shared<T>(std::move(component)));
			if (compPointer == nullptr || getComponent(compPointer->getId()) != nullptr) {
				throw std::bad_exception();  // TODO custom exceptions
			}
			compPointer->setParent(interface);
			components.push_back(compPointer);
		}

        void addCoupling(const std::shared_ptr<PortInterface>& portFrom, const std::shared_ptr<PortInterface>& portTo) {
            if (!portTo->compatible(portFrom)) {
                throw std::bad_cast();  // TODO custom exceptions
            }
            auto portFromParent = portFrom->getParent();
            auto portToParent = portTo->getParent();
            if (portFromParent == interface && interface->inPorts.containsPort(portFrom)) {
                if (portToParent->parent.lock() == interface && portToParent->inPorts.containsPort(portTo)) {
                    EIC.emplace_back(portFrom, portTo);
                } else {
                    throw std::bad_exception();  // TODO custom exceptions
                }
            } else if (portFromParent->parent.lock() == interface && portFromParent->outPorts.containsPort(portFrom)) {
                if (portToParent == interface && interface->outPorts.containsPort(portTo)) {
                    EOC.emplace_back(portFrom, portTo);
                } else if (portToParent->parent.lock() == interface && portToParent->inPorts.containsPort(portTo)) {
                    IC.emplace_back(portFrom, portTo);
                } else {
                    throw std::bad_exception();  // TODO custom exceptions
                }
            } else {
                throw std::bad_exception();  // TODO custom exceptions
            }
        }

        void addExternalInputCoupling(const std::string& portFromId, const std::string& componentToId, const std::string& portToId) {
            auto componentTo = getComponent(componentToId);
            if (componentTo == nullptr) {
                throw std::bad_exception();  // TODO custom exceptions
            }
            auto portFrom = getInPort(portFromId);
            auto portTo = componentTo->getInPort(portToId);
            if (portFrom == nullptr || portTo == nullptr) {
                throw std::bad_exception();  // TODO custom exceptions
            }
            EIC.emplace_back(portFrom, portTo);
        }

        void addInternalCoupling(const std::string& componentFromId, const std::string& portFromId, const std::string& componentToId, const std::string& portToId) {
            auto componentFrom = getComponent(componentFromId);
            auto componentTo = getComponent(componentToId);
            if (componentFrom == nullptr || componentTo == nullptr) {
                throw std::bad_exception();  // TODO custom exceptions
            }
            auto portFrom = componentFrom->getOutPort(portFromId);
            auto portTo = componentTo->getInPort(portToId);
            if (portFrom == nullptr || portTo == nullptr) {
                throw std::bad_exception();  // TODO custom exceptions
            }
            IC.emplace_back(portFrom, portTo);
        }

        void addExternalOutputCoupling(const std::string& componentFromId, const std::string& portFromId, const std::string& portToId) {
            auto componentFrom = getComponent(componentFromId);
            if (componentFrom == nullptr) {
                throw std::bad_exception();  // TODO custom exceptions
            }
            auto portFrom = componentFrom->getOutPort(portFromId);
            auto portTo = getOutPort(portToId);
            if (portFrom == nullptr || portTo == nullptr) {
                throw std::bad_exception();  // TODO custom exceptions
            }
            EOC.emplace_back(portFrom, portTo);
        }
    };
}

#endif //_CADMIUM_CORE_MODELING_COUPLED_HPP_
