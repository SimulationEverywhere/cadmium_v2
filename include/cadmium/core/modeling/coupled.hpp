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
#include <unordered_map>
#include <utility>
#include <vector>
#include "component.hpp"
#include "port.hpp"
#include "../exception.hpp"

#include <iostream>

namespace cadmium {
    //! Couplings are unordered maps {portTo: [portFrom1, portFrom2, ...]}
    using couplings = std::unordered_map<std::shared_ptr<PortInterface>, std::vector<std::shared_ptr<PortInterface>>>;

	//! Class for coupled DEVS models.
	class Coupled: public Component {
     protected:
        std::unordered_map<std::string, std::shared_ptr<Component>> components;  //!< Components set.
        couplings EIC;  //!< External Input Coupling set.
        couplings IC;   //!< Internal Coupling set.
        couplings EOC;  //!< External Output Coupling set.

     public:
		/**
		 * Constructor function.
		 * @param id ID of the coupled model.
		 */
        explicit Coupled(const std::string& id): Component(id), components(), EIC(), IC(), EOC() {}

		//! @return reference to the component set.
        std::unordered_map<std::string, std::shared_ptr<Component>>& getComponents() {
			return components;
		}

		//! @return reference to the EIC set.
		couplings& getEICs() {
			return EIC;
		}

		//! @return reference to the IC set.
        couplings& getICs() {
			return IC;
		}

		//! @return reference to the EOC set.
        couplings& getEOCs() {
			return EOC;
		}

		/**
		 * Returns a pointer to a subcomponent with the provided ID.
		 * @param id ID of the subcomponent
		 * @return pointer to the subcomponent.
		 * @throw CadmiumModelException if the component is not found.
		 */
        [[nodiscard]] std::shared_ptr<Component> getComponent(const std::string& id) const {
            try {
                return components.at(id);
            } catch (std::out_of_range& _) {
                throw CadmiumModelException("component not found");
            }
        }

		/**
		 * Adds a new subcomponent by pointer.
		 * @param component pointer to the component to be added.
		 * @throw CadmiumModelException if there is already another model with the same ID.
		 */
		void addComponent(const std::shared_ptr<Component>& component) {
            if (components.find(component->getId()) != components.end()) {
                throw CadmiumModelException("component ID already defined");
            }
			component->setParent(this);
            components[component->getId()] = component;
		}

		/**
		 * Creates and adds a new subcomponent. Then, it returns a pointer to the new component.
		 * @tparam T type of the component to be added.
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
		[[nodiscard]] static bool containsCoupling(const couplings& couplings, const std::shared_ptr<PortInterface>& portFrom, const std::shared_ptr<PortInterface>& portTo) {
            if (couplings.find(portTo) == couplings.end()) {
                return false;
            }
            const auto& portsFrom = couplings.at(portTo);
            return std::find(portsFrom.begin(), portsFrom.end(), portFrom) != portsFrom.end();
		}

		/**
		 * Adds a coupling to a coupling list.
		 * @param coupList coupling list.
		 * @param portFrom origin port.
		 * @param portTo destination port.
		 * @throw CadmiumModelException if coupling already exists in the coupling list.
		 */
		static void addCoupling(couplings& coupList, const std::shared_ptr<PortInterface>& portFrom, const std::shared_ptr<PortInterface>& portTo) {
			auto aux = coupList.find(portTo);
            if (aux == coupList.end()) {
                coupList[portTo] = {portFrom};
            } else {
                auto& portsFrom = aux->second;
                if (std::find(portsFrom.begin(), portsFrom.end(), portFrom) != portsFrom.end()) {
                    throw CadmiumModelException("duplicate coupling");
                }
                portsFrom.push_back(portFrom);
            }
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
            if (containsInPort(portFrom)) {
                if (portToParent->getParent() == this && portToParent->containsInPort(portTo)) {
					addCoupling(EIC, portFrom, portTo);
                } else {
					throw CadmiumModelException("invalid destination port");
                }
            } else if (portFromParent->getParent() == this && portFromParent->containsOutPort(portFrom)) {
                if (containsOutPort(portTo)) {
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

		/**
		 * Adds an external input coupling dynamically. If the origin input port does not exist yet,
		 * it creates it and adds it to the input port set before creating the coupling.
		 * @param portFromId ID of the origin port.
		 * @param componentToId ID of the destination component.
		 * @param portToId ID of the destination port.
		 * @throw CadmiumModelException if the coupling is invalid or it already exists.
		 */
		void addDynamicEIC(const std::string& portFromId, const std::string& componentToId, const std::string& portToId) {
			auto componentTo = getComponent(componentToId);
			auto portTo = componentTo->getInPort(portToId);
			std::shared_ptr<PortInterface> portFrom;
			try {
				portFrom = getInPort(portFromId);
			} catch (CadmiumModelException& ex) {
				if (std::strcmp(ex.what(), "port not found") != 0) {
					throw CadmiumModelException(ex.what());
				}
				portFrom = portTo->newCompatiblePort(portFromId);
				addInPort(portFrom);
			}
			addCoupling(EIC, portFrom, portTo);
		}

		/**
		 * Adds an external output coupling dynamically. If the destination output port does not exist yet,
		 * it creates it and adds it to the output port set before creating the coupling.
		 * @param componentFromId ID of the origin component.
		 * @param portFromId ID of the origin port.
		 * @throw CadmiumModelException if the coupling is invalid or it already exists.
		 */
		void addDynamicEOC(const std::string& componentFromId, const std::string& portFromId, const std::string& portToId) {
			auto componentFrom = getComponent(componentFromId);
			auto portFrom = componentFrom->getOutPort(portFromId);
			std::shared_ptr<PortInterface> portTo;
			try {
				portTo = getOutPort(portToId);
			} catch (CadmiumModelException& ex) {
				if (std::strcmp(ex.what(), "port not found") != 0) {
					throw CadmiumModelException(ex.what());
				}
				portTo = portFrom->newCompatiblePort(portToId);
				addOutPort(portTo);
			}
			addCoupling(EOC, portFrom, portTo);
		}

		/**
		 * Flattens hierarchical structure.
		 * Works recursively, modifies itself and parent component as well.
		 */
		 /*
		void flatten(){
        	std::vector<std::shared_ptr<Coupled>> toFlatten;

        	for(auto& component: components){
        		auto coupled = std::dynamic_pointer_cast<Coupled>(component);
        		if (coupled != nullptr) {
        			toFlatten.push_back(coupled);
        		}
        	}

        	for(auto& coupled: toFlatten){
        		coupled->flatten();
        		removePortsAndCouplings(coupled);
        		for(auto itcomp = components.begin(); itcomp != components.end(); itcomp++){
        			if(coupled == *itcomp) {
        				components.erase(itcomp);
        			}
        		}
        	}

        	toFlatten.clear();

        	if(parent != nullptr) {
        		std::vector<std::shared_ptr<PortInterface>> leftBridgeEIC = createLeftBridge(parent->getEICs());
        		std::vector<std::shared_ptr<PortInterface>> leftBridgeIC = createLeftBridge(parent->getICs());

        		std::vector<std::shared_ptr<PortInterface>> rightBridgeEOC = createRightBridge(parent->getEOCs());
        		std::vector<std::shared_ptr<PortInterface>> rightBridgeIC = createRightBridge(parent->getICs());

        		completeLeftBridge(EIC, leftBridgeEIC, parent->getEICs());
        		completeLeftBridge(EIC, leftBridgeIC, parent->getICs());
        		completeRightBridge(EOC, rightBridgeEOC, parent->getEOCs());
        		completeRightBridge(EOC, rightBridgeIC, parent->getICs());

            	for (auto& component: components) {
            		parent->addComponent(component);
            	}

            	for (auto& ic: IC) {
            		parent->getICs().push_back(ic);
            	}
        	}
		}

     private:
        void removePortsAndCouplings(std::shared_ptr<Coupled> child) {
        	std::vector<std::shared_ptr<PortInterface>> inPorts = child->getInPorts();
        	for(auto& inport: inPorts){
        		for(auto itc = EIC.begin(); itc != EIC.end(); itc++){
        			if(inport == std::get<1>(*itc)){
            			EIC.erase(itc);
            		}
        		}
        		for(auto itc = IC.begin(); itc != IC.end(); itc++){
        			if(inport == std::get<1>(*itc)) {
            			IC.erase(itc);
            		}
        		}
        	}
        	std::vector<std::shared_ptr<PortInterface>> outPorts = child->getOutPorts();
        	for(auto& outport: outPorts){
        		for(auto itc = EOC.begin(); itc != EOC.end(); itc++){
        			if(outport == std::get<0>(*itc)){
        				EOC.erase(itc);
        			}
        		}
        		for(auto itc = IC.begin(); itc != IC.end(); itc++){
        			if(outport == std::get<1>(*itc)){
        				IC.erase(itc);
        			}
        		}
        	}
        }

        std::vector<std::shared_ptr<PortInterface>> createLeftBridge(std::vector<coupling> couplings) {
        	std::vector<std::shared_ptr<PortInterface>> leftBridge;
            for (auto& iPort: this->getInPorts()) {
                for (auto& c: couplings) {
                    if (std::get<1>(c) == iPort) {
                    	leftBridge.push_back(std::get<0>(c));
                    }
                }
            }
            return leftBridge;
        }

        std::vector<std::shared_ptr<PortInterface>> createRightBridge(std::vector<coupling> couplings) {
        	std::vector<std::shared_ptr<PortInterface>> rightBridge;
            for (auto& oPort: this->getOutPorts()) {
                for (auto& c: couplings) {
                    if (std::get<0>(c) == oPort) {
                    	rightBridge.push_back(std::get<1>(c));
                    }
                }
            }
            return rightBridge;
        }

        void completeLeftBridge(std::vector<coupling> couplings, std::vector<std::shared_ptr<PortInterface>> leftBridge,
        	std::vector<coupling>& pCouplings) {
        	for (auto& c: couplings) {
        		for(auto& portFrom: leftBridge) {
        			std::shared_ptr<PortInterface> right = std::get<1>(c);
                	coupling tuple = std::make_tuple(portFrom, right);
                	pCouplings.push_back(tuple);
        		}
        	}
        }

        void completeRightBridge(std::vector<coupling> couplings, std::vector<std::shared_ptr<PortInterface>> rightBridge,
        	std::vector<coupling>& pCouplings) {
        	for (auto& c: couplings) {
        		for(auto& portTo: rightBridge) {
        			std::shared_ptr<PortInterface> left = std::get<0>(c);
                	coupling tuple = std::make_tuple(left, portTo);
                	pCouplings.push_back(tuple);
        		}
        	}
        }
        */
    };
}

#endif //CADMIUM_CORE_MODELING_COUPLED_HPP_
