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
#include <string>
#include <tuple>
#include <typeinfo>
#include <utility>
#include <vector>
#include "component.hpp"
#include "port.hpp"

namespace cadmium {

    // TODO: Why not make this into a simple class or struct?
    using coupling = std::tuple<const std::shared_ptr<PortInterface>, const std::shared_ptr<PortInterface>>;

    class Coupled: public Component {
     private:
        friend class Coordinator;
     public:
        std::vector<std::shared_ptr<Component>> components;
        std::vector<coupling> EIC;
        std::vector<coupling> IC;
        std::vector<coupling> EOC;

        explicit Coupled(std::string id, std::string className): Component(std::move(id)), components(), EIC(), IC(), EOC() {
            this->className = std::move(className);
        }

        explicit Coupled(std::string id): Coupled(std::move(id), "") {}

        [[nodiscard]] std::shared_ptr<Component> getComponent(const std::string& id) const {
            for (auto const& component: components) {
                if (component->id == id) {
                    return component;
                }
            }
            return nullptr;
        }

        template <typename T>
        void addComponent(const T component) {
            auto compPointer = std::dynamic_pointer_cast<Component>(std::make_shared<T>(std::move(component)));
            if (compPointer == nullptr || getComponent(compPointer->id) != nullptr) {
                throw std::bad_exception();  // TODO custom exceptions
            }
            components.push_back(compPointer);
        }

        void addComponent(const std::shared_ptr<Component>& compPointer) {
            if (compPointer == nullptr || getComponent(compPointer->id) != nullptr) {
                throw std::bad_exception();  // TODO custom exceptions
            }
            components.push_back(compPointer);
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

        long setUid(long next) override {
            Component::setUid(next++);
            for (auto& component: components) {
                next = component->setUid(next);
            }
            return next;
        }

        void traverse(std::function<void(Component* c)> fn) override{
            fn(this);

            for (auto& component: components) {
                component->traverse(fn);
            }
        }

        std::vector<std::shared_ptr<Component>> get_components() override {
            return components;
        }
    };
}

#endif //_CADMIUM_CORE_MODELING_COUPLED_HPP_
