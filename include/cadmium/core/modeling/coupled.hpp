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
#include "../exception.hpp"

namespace cadmium {

    using coupling = std::tuple<const std::shared_ptr<PortInterface>, const std::shared_ptr<PortInterface>>;

    class Coupled: public Component {
     protected:
        std::vector<std::shared_ptr<Component>> components;
        std::vector<coupling> EIC;
        std::vector<coupling> IC;
        std::vector<coupling> EOC;
     public:
        explicit Coupled(const std::string& id): Component(id), components(), EIC(), IC(), EOC() {}

        [[nodiscard]] std::shared_ptr<Component> getComponent(const std::string& id) const {
            for (auto const& component: components) {
                if (component->getId() == id) {
                    return component;
                }
            }
            return nullptr;
        }

        std::vector<std::shared_ptr<Component>>& getComponents() {
        	return components;
    	}

		template <typename T>
		void addComponent(const std::shared_ptr<T>& component) {
			auto compPointer = std::dynamic_pointer_cast<Component>(component);
			if (compPointer == nullptr) {
				throw CadmiumModelException("The provided argument is not a Cadmium component");
			} else if (getComponent(compPointer->getId()) != nullptr) {
				throw CadmiumModelException("A component with ID " + compPointer->getId() + " already exists");
			}
			compPointer->setParent(this);
			components.push_back(compPointer);
		}

		template <typename T>
		void addComponent(const T component) {
			auto compPointer = std::dynamic_pointer_cast<Component>(std::make_shared<T>(std::move(component)));
			if (compPointer == nullptr) {
				throw CadmiumModelException("The provided argument is not a Cadmium component");
			} else if (getComponent(compPointer->getId()) != nullptr) {
				throw CadmiumModelException("A component with ID " + compPointer->getId() + " already exists");
			}
			compPointer->setParent(this);
			components.push_back(compPointer);
		}

        void addCoupling(const std::shared_ptr<PortInterface>& portFrom, const std::shared_ptr<PortInterface>& portTo) {
            if (!portTo->compatible(portFrom)) {
				throw CadmiumModelException("Port data types are not compatible");
            }
			if (!portFrom->getParent().has_value()) {
				throw CadmiumModelException("Port " + portFrom->getId() + " does not belong to any model");
			}
			if (!portTo->getParent().has_value()) {
				throw CadmiumModelException("Port " + portTo->getId() + " does not belong to any model");
			}

            auto portFromParent = portFrom->getParent().value();
            auto portToParent = portTo->getParent().value();
            if (inPorts.containsPort(portFrom)) {
                if (portToParent->getParent().value() == this && portToParent->containsInPort(portTo)) {
                    EIC.emplace_back(portFrom, portTo);
                } else {
					throw CadmiumModelException("Destination port " + portTo->getId() + " is invalid");
                }
            } else if (portFromParent->getParent().value() == this && portFromParent->containsOutPort(portFrom)) {
                if (outPorts.containsPort(portTo)) {
                    EOC.emplace_back(portFrom, portTo);
                } else if (portToParent->getParent().value() == this && portToParent->containsInPort(portTo)) {
                    IC.emplace_back(portFrom, portTo);
                } else {
					throw CadmiumModelException("Destination port " + portTo->getId() + " is invalid");
                }
            } else {
				throw CadmiumModelException("Origin port " + portFrom->getId() + " is invalid");
            }
        }

        std::vector<coupling>& getEICs() {
        	return EIC;
        }

        void addEIC(const std::string& portFromId, const std::string& componentToId, const std::string& portToId) {
            auto componentTo = getComponent(componentToId);
            if (componentTo == nullptr) {
				throw CadmiumModelException("There is no subcomponent with ID " + componentToId);
            }
            auto portFrom = getInPort(portFromId);
            auto portTo = componentTo->getInPort(portToId);

            if (portFrom == nullptr) {
				throw CadmiumModelException("Invalid origin port ID " + portFromId);
			} else if (portTo == nullptr) {
				throw CadmiumModelException("Invalid destination port ID " + portToId);
            }
            EIC.emplace_back(portFrom, portTo);
        }

        std::vector<coupling>& getICs() {
        	return IC;
        }

        void addIC(const std::string& componentFromId, const std::string& portFromId, const std::string& componentToId, const std::string& portToId) {
            auto componentFrom = getComponent(componentFromId);
            auto componentTo = getComponent(componentToId);
            if (componentFrom == nullptr) {
				throw CadmiumModelException("There is no subcomponent with ID " + componentFromId);
			} else if(componentTo == nullptr) {
				throw CadmiumModelException("There is no subcomponent with ID " + componentToId);
            }
            auto portFrom = componentFrom->getOutPort(portFromId);
            auto portTo = componentTo->getInPort(portToId);
			if (portFrom == nullptr) {
				throw CadmiumModelException("Invalid origin port ID " + portFromId);
			} else if (portTo == nullptr) {
				throw CadmiumModelException("Invalid destination port ID " + portToId);
			}
            IC.emplace_back(portFrom, portTo);
        }

        std::vector<coupling>& getEOCs() {
        	return EOC;
        }

        void addEOC(const std::string& componentFromId, const std::string& portFromId, const std::string& portToId) {
            auto componentFrom = getComponent(componentFromId);
            if (componentFrom == nullptr) {
				throw CadmiumModelException("There is no subcomponent with ID " + componentFromId);
            }
            auto portFrom = componentFrom->getOutPort(portFromId);
            auto portTo = getOutPort(portToId);
			if (portFrom == nullptr) {
				throw CadmiumModelException("Invalid origin port ID " + portFromId);
			} else if (portTo == nullptr) {
				throw CadmiumModelException("Invalid destination port ID " + portToId);
			}
            EOC.emplace_back(portFrom, portTo);
        }
    };
}

#endif //_CADMIUM_CORE_MODELING_COUPLED_HPP_
