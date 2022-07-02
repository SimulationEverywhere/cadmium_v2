/**
 * DEVS coupled model.
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

#ifndef CADMIUM_CORE_MODELING_COUPLED_HPP_
#define CADMIUM_CORE_MODELING_COUPLED_HPP_

#include <memory>
#include <cstring>
#include <string>
#include <tuple>
#include <typeinfo>
#include <utility>
#include <vector>
#include "component.hpp"
#include "port.hpp"
#include "../exception.hpp"

namespace cadmium {
	/// Couplings are just tuples <portFrom, portTo>
    using coupling = std::tuple<const std::shared_ptr<PortInterface>, const std::shared_ptr<PortInterface>>;

    class Coupled: public Component {
     protected:
        std::vector<std::shared_ptr<Component>> components;  /// Components set.
        std::vector<coupling> EIC;                           /// External Input Coupling set.
        std::vector<coupling> IC;                            /// Internal Coupling set.
        std::vector<coupling> EOC;                           /// External Output Coupling set.
     public:
		/**
		 * Constructor function.
		 * @param id ID of the coupled model.
		 */
        explicit Coupled(const std::string& id): Component(id), components(), EIC(), IC(), EOC() {}

		/// @return reference to the component set.
		std::vector<std::shared_ptr<Component>>& getComponents() {
			return components;
		}

		/// @return reference to the EIC set.
		std::vector<coupling>& getEICs() {
			return EIC;
		}

		/// @return reference to the IC set.
		std::vector<coupling>& getICs() {
			return IC;
		}

		/// @return reference to the EOC set.
		std::vector<coupling>& getEOCs() {
			return EOC;
		}

		/**
		 * Returns a pointer to a subcomponent with the provided ID.
		 * @param id ID of the subcomponent
		 * @return pointer to the subcomponent.
		 * @throw CadmiumModelException if the component is not found.
		 */
        [[nodiscard]] std::shared_ptr<Component> getComponent(const std::string& id) const {
            for (auto const& component: components) {
                if (component->getId() == id) {
                    return component;
                }
            }
			throw CadmiumModelException("component not found");
        }

		/**
		 * Adds a new subcomponent by pointer.
		 * @param component pointer to the component to be added.
		 * @throw CadmiumModelException if there is already another model with the same ID.
		 */
		void addComponent(const std::shared_ptr<Component>& component) {
			auto componentIdAlreadyDefined = false;
			try {
				(void) getComponent(component->getId());
			} catch (CadmiumModelException& ex) {
				if (std::strcmp(ex.what(), "component not found") != 0) {
					throw CadmiumModelException(ex.what());
				}
				componentIdAlreadyDefined = true;
			}
			if (!componentIdAlreadyDefined) {
				throw CadmiumModelException("component ID already defined");
			}
			component->setParent(this);
			components.push_back(component);
		}

		/**
		 * Adds a new subcomponent and returns a pointer to the new component.
		 * @tparam T data type  of the component to be added.
		 * @tparam Args data types of all the constructor fields of the new component.
		 * @param args extra parameters required to generate the new component.
		 * @return pointer to the new component.
		 */
		template <typename T, typename... Args>
		std::shared_ptr<T> addComponent(Args&&... args) {
			auto component = std::make_shared<T>(std::forward<Args>(args)...);
			addComponent(component);
			return component;
		}

		/**
		 * Checks if coupling already exists.
		 * @param coupList coupling list.
		 * @param portFrom origin port.
		 * @param portTo destination port.
		 * @return true if coupling already exists.
		 */
		[[nodiscard]] static bool containsCoupling(const std::vector<coupling>& coupList, const std::shared_ptr<PortInterface>& portFrom, const std::shared_ptr<PortInterface>& portTo) {
			coupling coup = {portFrom, portTo};
			return std::find(coupList.begin(), coupList.end(), coup) != coupList.end();
		}

		/**
		 * Adds a coupling to a coupling list.
		 * @param coupList coupling list.
		 * @param portFrom origin port.
		 * @param portTo destination port.
		 * @throw CadmiumModelException if coupling already exists in the coupling list.
		 */
		static void addCoupling(std::vector<coupling>& coupList, const std::shared_ptr<PortInterface>& portFrom, const std::shared_ptr<PortInterface>& portTo) {
			if (containsCoupling(coupList, portFrom, portTo)) {
				throw CadmiumModelException("duplicate coupling");
			}
			coupList.emplace_back(portFrom, portTo);
		}

		/**
		 * Adds a coupling between two ports.
		 * @param portFrom origin port.
		 * @param portTo destination port.
		 * @throw CadmiumModelException if the coupling is invalid or it already exists.
		 */
        void addCoupling(const std::shared_ptr<PortInterface>& portFrom, const std::shared_ptr<PortInterface>& portTo) {
            if (!portTo->compatible(portFrom)) {
				throw CadmiumModelException("invalid port type");
            }
			if (portFrom->getParent() == nullptr || portTo->getParent() == nullptr) {
				throw CadmiumModelException("port does not belong to any model");
			}
            auto portFromParent = portFrom->getParent();
            auto portToParent = portTo->getParent();
            if (inPorts.containsPort(portFrom)) {
                if (portToParent->getParent() == this && portToParent->containsInPort(portTo)) {
					addCoupling(EIC, portFrom, portTo);
                } else {
					throw CadmiumModelException("invalid destination port");
                }
            } else if (portFromParent->getParent() == this && portFromParent->containsOutPort(portFrom)) {
                if (outPorts.containsPort(portTo)) {
					addCoupling(EOC, portFrom, portTo);
                } else if (portToParent->getParent() == this && portToParent->containsInPort(portTo)) {
					addCoupling(IC, portFrom, portTo);
                } else {
					throw CadmiumModelException("invalid destination port");
                }
            } else {
				throw CadmiumModelException("invalid origin port");
            }
        }

		/**
		 * Adds an external input coupling.
		 * @param portFromId ID of the origin port.
		 * @param componentToId ID of the destination component.
		 * @param portToId ID of the destination port.
		 * @throw CadmiumModelException if the coupling is invalid or it already exists.
		 */
        void addEIC(const std::string& portFromId, const std::string& componentToId, const std::string& portToId) {
            auto portFrom = getInPort(portFromId);
			auto componentTo = getComponent(componentToId);
            auto portTo = componentTo->getInPort(portToId);
			addCoupling(EIC, portFrom, portTo);
        }

		/**
		 * Adds an internal coupling.
		 * @param componentFromId ID of the origin component.
		 * @param portFromId ID of the origin port.
		 * @param componentToId ID of the destination component.
		 * @param portToId ID of the destination port.
		 * @throw CadmiumModelException if the coupling is invalid or it already exists.
		 */
        void addIC(const std::string& componentFromId, const std::string& portFromId, const std::string& componentToId, const std::string& portToId) {
            auto componentFrom = getComponent(componentFromId);
			auto portFrom = componentFrom->getOutPort(portFromId);
            auto componentTo = getComponent(componentToId);
            auto portTo = componentTo->getInPort(portToId);
			addCoupling(IC, portFrom, portTo);
        }

		/**
		 * Adds an external output coupling.
		 * @param componentFromId ID of the origin component.
		 * @param portFromId ID of the origin port.
		 * @throw CadmiumModelException if the coupling is invalid or it already exists.
		 */
        void addEOC(const std::string& componentFromId, const std::string& portFromId, const std::string& portToId) {
            auto componentFrom = getComponent(componentFromId);
            auto portFrom = componentFrom->getOutPort(portFromId);
            auto portTo = getOutPort(portToId);
			addCoupling(EOC, portFrom, portTo);
        }
    };
}

#endif //CADMIUM_CORE_MODELING_COUPLED_HPP_
