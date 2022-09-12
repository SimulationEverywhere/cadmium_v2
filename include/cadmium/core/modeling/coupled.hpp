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
    using couplingMap = std::unordered_map<std::shared_ptr<PortInterface>, std::vector<std::shared_ptr<PortInterface>>>;
    using Serialcoupling = std::tuple<std::shared_ptr<PortInterface>, std::shared_ptr<PortInterface>>;

	//! Class for coupled DEVS models.
	class Coupled: public Component {
     protected:
        std::unordered_map<std::string, std::shared_ptr<Component>> components;  //!< Components set.
        couplingMap EIC;  //!< External Input Coupling set.
        couplingMap IC;   //!< Internal Coupling set.
        couplingMap EOC;  //!< External Output Coupling set.
        // Auxiliary structs for flatenning
        std::vector<Serialcoupling> eic;
        std::vector<Serialcoupling> ic;
        std::vector<Serialcoupling> eoc;

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
        couplingMap& getEICs() {
			return EIC;
		}

		//! @return reference to the IC set.
        couplingMap& getICs() {
			return IC;
		}

		//! @return reference to the EOC set.
        couplingMap& getEOCs() {
			return EOC;
		}

		//! @return reference to the eic set.
        std::vector<Serialcoupling>& geteics() {
			return eic;
		}

		//! @return reference to the ic set.
        std::vector<Serialcoupling>& getics() {
			return ic;
		}

		//! @return reference to the eoc set.
        std::vector<Serialcoupling>& geteocs() {
			return eoc;
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
		[[nodiscard]] static bool containsCoupling(const couplingMap& couplings, const std::shared_ptr<PortInterface>& portFrom, const std::shared_ptr<PortInterface>& portTo) {
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
		static void addCoupling(couplingMap& coupList, const std::shared_ptr<PortInterface>& portFrom, const std::shared_ptr<PortInterface>& portTo) {
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
		void flatten_r() {

            for (auto& [portTo, portsFrom]: EIC) {
                for (auto& portFrom: portsFrom) {
                    eic.push_back({portFrom, portTo});
                }
            }
            for (auto& [portTo, portsFrom]: IC) {
                for (auto& portFrom: portsFrom) {
                    ic.push_back({portFrom, portTo});
                }
            }
            for (auto& [portTo, portsFrom]: EOC) {
                for (auto& portFrom: portsFrom) {
                    eoc.push_back({portFrom, portTo});
                }
            }

            //flatten_recursive();

        	//EIC.clear();
        	//IC.clear();
        	//EOC.clear();

        	for(auto i=0; i<eic.size();i++){
            	auto aux = EIC.find(std::get<0>(eic[i]));
            	if (aux == EIC.end()) {
            		EIC[std::get<0>(eic[i])] = {std::get<1>(eic[i])};
            	} else {
            		auto& portsFrom = aux->second;
            		portsFrom.push_back(std::get<1>(eic[i]));
            	}
            }

            for(auto i=0; i<ic.size();i++){
            	auto aux = IC.find(std::get<0>(ic[i]));
            	if (aux == IC.end()) {
            		IC[std::get<0>(ic[i])] = {std::get<1>(ic[i])};
            	} else {
            		auto& portsFrom = aux->second;
            		portsFrom.push_back(std::get<1>(ic[i]));
            	}
            }

            for(auto i=0; i<eoc.size();i++){
            	auto aux = EOC.find(std::get<0>(eoc[i]));
            	if (aux == EOC.end()) {
            		EOC[std::get<0>(eoc[i])] = {std::get<1>(eoc[i])};
            	} else {
            		auto& portsFrom = aux->second;
            		portsFrom.push_back(std::get<1>(eoc[i]));
            	}
            }

		}

		void flatten() {

			for (auto& [portTo, portsFrom]: EIC) {
                for (auto& portFrom: portsFrom) {
                    eic.push_back({portFrom, portTo});
                }
            }
            for (auto& [portTo, portsFrom]: IC) {
                for (auto& portFrom: portsFrom) {
                    ic.push_back({portFrom, portTo});
                }
            }
            for (auto& [portTo, portsFrom]: EOC) {
                for (auto& portFrom: portsFrom) {
                    eoc.push_back({portFrom, portTo});
                }
            }
			std::vector<std::shared_ptr<Coupled>> toFlatten;

			for(auto& component: components){
				auto coupled = std::dynamic_pointer_cast<Coupled>(component.second);
				if (coupled != nullptr) {
					toFlatten.push_back(coupled);
				}
			}

			for(auto& coupled: toFlatten){
				coupled->flatten();
				removePortsAndCouplings(coupled);
				components.erase(coupled->getId());
			}

			toFlatten.clear();

			if(parent != nullptr) {

				//couplingMap leftBridgeEIC = createLeftBridge(parent->getEICs());
				std::vector<std::shared_ptr<PortInterface>> leftBridgeEIC = createLeftBridge(parent->geteics());

				//couplingMap leftBridgeIC = createLeftBridge(parent->getICs());
				std::vector<std::shared_ptr<PortInterface>> leftBridgeIC = createLeftBridge(parent->getics());

				//couplingMap rightBridgeEOC = createRightBridge(parent->getEOCs());
				std::vector<std::shared_ptr<PortInterface>> rightBridgeEOC = createRightBridge(parent->geteocs());

				//couplingMap rightBridgeIC = createRightBridge(parent->getICs());
				std::vector<std::shared_ptr<PortInterface>> rightBridgeIC = createRightBridge(parent->getics());

				completeLeftBridge(eic, leftBridgeEIC, parent->geteics());
				completeLeftBridge(eic, leftBridgeIC, parent->getics());
				completeRightBridge(eoc, rightBridgeEOC, parent->geteocs());
				completeRightBridge(eoc, rightBridgeIC, parent->getics());

				for (auto& component: components) {
					parent->addComponent(component.second);
				}

				for (auto& i: ic) {
					parent->getics().push_back(i);
				}
			}

        	//EIC.clear();
        	//IC.clear();
        	//EOC.clear();

        	for(auto& e: eic){
            	auto aux = EIC.find(std::get<0>(e));
            	if (aux == EIC.end()) {
            		EIC[std::get<0>(e)] = {std::get<1>(e)};
            	} else {
            		auto& portsFrom = aux->second;
            		portsFrom.push_back(std::get<1>(e));
            	}
            }

        	for(auto& i: ic){
            	auto aux = IC.find(std::get<0>(i));
            	if (aux == IC.end()) {
            		IC[std::get<0>(i)] = {std::get<1>(i)};
            	} else {
            		auto& portsFrom = aux->second;
            		portsFrom.push_back(std::get<1>(i));
            	}
            }

        	for(auto& e: eoc){
            	auto aux = EOC.find(std::get<0>(e));
            	if (aux == EOC.end()) {
            		EOC[std::get<0>(e)] = {std::get<1>(e)};
            	} else {
            		auto& portsFrom = aux->second;
            		portsFrom.push_back(std::get<1>(e));
            	}
            }


		}





/*
		void flatten_recursive() {

			std::vector<std::shared_ptr<Coupled>> toFlatten;

			for(auto& component: components){
				auto coupled = std::dynamic_pointer_cast<Coupled>(component.second);
				if (coupled != nullptr) {
					toFlatten.push_back(coupled);
				}
			}

			for(auto& coupled: toFlatten){
				coupled->flatten_recursive();
				removePortsAndCouplings(coupled);
				components.erase(coupled->getId());
			}

			toFlatten.clear();

			if(parent != nullptr) {

				//couplingMap leftBridgeEIC = createLeftBridge(parent->getEICs());
				std::vector<std::shared_ptr<PortInterface>> leftBridgeEIC = createLeftBridge(parent->geteics());

				//couplingMap leftBridgeIC = createLeftBridge(parent->getICs());
				std::vector<std::shared_ptr<PortInterface>> leftBridgeIC = createLeftBridge(parent->getics());

				//couplingMap rightBridgeEOC = createRightBridge(parent->getEOCs());
				std::vector<std::shared_ptr<PortInterface>> rightBridgeEOC = createRightBridge(parent->geteocs());

				//couplingMap rightBridgeIC = createRightBridge(parent->getICs());
				std::vector<std::shared_ptr<PortInterface>> rightBridgeIC = createRightBridge(parent->getics());

				completeLeftBridge(eic, leftBridgeEIC, parent->geteics());
				completeLeftBridge(eic, leftBridgeIC, parent->getics());
				completeRightBridge(eoc, rightBridgeEOC, parent->geteocs());
				completeRightBridge(eoc, rightBridgeIC, parent->getics());

				for (auto& component: components) {
					parent->addComponent(component.second);
				}

				for (auto& i: ic) {
					parent->getics().push_back(i);
				}
			}
		}
*/























     private:
        void removePortsAndCouplings(std::shared_ptr<Coupled> child) {
        	std::vector<std::shared_ptr<PortInterface>> inPorts = child->getInPorts();
        	for(auto& inport: inPorts){
        		for(auto itc = eic.begin(); itc != eic.end(); itc++){
        			if(inport == std::get<1>(*itc)){
            			eic.erase(itc);
            		}
        		}
        		for(auto itc = ic.begin(); itc != ic.end(); itc++){
        			if(inport == std::get<1>(*itc)) {
            			ic.erase(itc);
            		}
        		}
        	}
        	std::vector<std::shared_ptr<PortInterface>> outPorts = child->getOutPorts();
        	for(auto& outport: outPorts){
        		for(auto itc = eoc.begin(); itc != eoc.end(); itc++){
        			if(outport == std::get<0>(*itc)){
        				eoc.erase(itc);
        			}
        		}
        		for(auto itc = ic.begin(); itc != ic.end(); itc++){
        			if(outport == std::get<0>(*itc)){
        				ic.erase(itc);
        			}
        		}
        	}
        }

        std::vector<std::shared_ptr<PortInterface>> createLeftBridge(std::vector<Serialcoupling> couplings) {
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

        std::vector<std::shared_ptr<PortInterface>> createRightBridge(std::vector<Serialcoupling> couplings) {
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

        void completeLeftBridge(std::vector<Serialcoupling> couplings, std::vector<std::shared_ptr<PortInterface>> leftBridge,
        	std::vector<Serialcoupling>& pCouplings) {
        	for (auto& c: couplings) {
        		for(auto& portFrom: leftBridge) {
        			std::shared_ptr<PortInterface> right = std::get<1>(c);
                	Serialcoupling tuple = std::make_tuple(portFrom, right);
                	pCouplings.push_back(tuple);
        		}
        	}
        }

        void completeRightBridge(std::vector<Serialcoupling> couplings, std::vector<std::shared_ptr<PortInterface>> rightBridge,
        	std::vector<Serialcoupling>& pCouplings) {
        	for (auto& c: couplings) {
        		for(auto& portTo: rightBridge) {
        			std::shared_ptr<PortInterface> left = std::get<0>(c);
                	Serialcoupling tuple = std::make_tuple(left, portTo);
                	pCouplings.push_back(tuple);
        		}
        	}
        }


/*
     private:
        void removePortsAndCouplings(std::shared_ptr<Coupled> child) {
        	std::vector<std::shared_ptr<PortInterface>> inPorts = child->getInPorts();
        	for(auto& inport: inPorts){
        		for(auto itc = EIC.begin(); itc != EIC.end(); itc++){
        			for(auto in = itc->second.begin(); in != itc->second.end(); in++){
            			if(inport == *in){
                			itc->second.erase(in);
                		}
            			//if vector of senders is empty remove coupling
            			if(itc->second.empty()) {
            				EIC.erase(itc);
            			}
        			}
        		}
        		for(auto itc = IC.begin(); itc != IC.end(); itc++){
        			for(auto in = itc->second.begin(); in != itc->second.end(); in++){
            			if(inport == *in){
                			itc->second.erase(in);
                		}
            			//if vector of senders is empty remove coupling
            			if(itc->second.empty()) {
            				IC.erase(itc);
            			}
        			}
        		}
        	}

        	std::vector<std::shared_ptr<PortInterface>> outPorts = child->getOutPorts();
        	for(auto& outport: outPorts){
        		for(auto itc = EOC.begin(); itc != EOC.end(); itc++){
        			if(outport == itc->first){
        				EOC.erase(itc);
        			}
        		}
        		for(auto itc = IC.begin(); itc != IC.end(); itc++){
        			if(outport == itc->first){
        				IC.erase(itc);
        			}
        		}
        	}

        }
*/
/*
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
*/
/*
        private HashMap<Port<?>, LinkedList<Port<?>>> createLeftBrige(LinkedList<Coupling<?>> couplings) {
             HashMap<Port<?>, LinkedList<Port<?>>> leftBridge = new HashMap<>();
             for (Port<?> iPort : this.inPorts) {
                 for (Coupling<?> c : couplings) {
                     if (c.portTo == iPort) {
                         LinkedList<Port<?>> list = leftBridge.get(iPort);
                         if (list == null) {
                             list = new LinkedList<>();
                             leftBridge.put(iPort, list);
                         }
                         list.add(c.portFrom);
                     }
                 }
             }
             return leftBridge;
         }
*/
/*
        couplingMap createLeftBridge(couplingMap couplings) {
        	couplingMap leftBridge;
            for (auto& iPort: this->getInPorts()) {
            	for (auto& c: couplings) {
                	if(c.first == iPort){
                		couplingMap::iterator it = leftBridge.find(iPort);
                		if(it == NULL){
                			//leftBridge.insert(*it);
                		} else {
                			it->second.push_back(iPort);
                		}
                	}
                }
            }
            return leftBridge;
        }
*/
/*
        std::vector<std::shared_ptr<PortInterface>> createLeftBridge(couplingMap couplings) {
        	std::vector<std::shared_ptr<PortInterface>> leftBridge;
        	for (auto& iPort: this->getInPorts()) {
        		for (auto& c: couplings) {
        			if(c.first == iPort){
        				leftBridge.push_back(c.first);
        			}
        		}
        	}
        	return leftBridge;
        }
*/
        /*
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
        */

        couplingMap createLeftBridge(couplingMap couplings) {
        	couplingMap leftBridge;
            for (auto& iPort: this->getInPorts()) {
            	for (auto& c: couplings) {
            		if (c.first == iPort) {
            			couplingMap::iterator it = leftBridge.find(iPort);
                         //if (it == leftBridge.end()) {
                         //    leftBridge.insert({iPort, {}});
                         //}
                         //it->second.push_back(iPort);
                         leftBridge.insert({iPort, c.second});
                     }
                 }
             }
             return leftBridge;
         }



/*
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
*/


        couplingMap createRightBridge(couplingMap couplings) {
        	couplingMap rightBridge;
            for (auto& oPort: this->getOutPorts()) {
            	for (auto& c: couplings) {
            		for (auto& sender: c.second){
            			if(sender == oPort){
            				couplingMap::iterator it = rightBridge.find(c.first);
            				//if(it == NULL){
            				//	rightBridge.insert({oPort, {}});
            				//}
            				//for (auto& sender: c.second){
            				//	it->second.push_back(sender);
            				//}
            				rightBridge.insert({c.first, {oPort}});
            				//it->second.push_back(oPort);
            			}
            		}
                }
            }
            return rightBridge;
        }

/*
        std::vector<std::shared_ptr<PortInterface>> createRightBridge(couplingMap couplings) {
        	std::vector<std::shared_ptr<PortInterface>> rightBridge;
        	for (auto& oPort: this->getOutPorts()) {
        		for (auto& c: couplings) {
        			for (auto& sender: c.second){
        				if(sender == oPort){
        					rightBridge.push_back(c.first);
        				}
        			}
        		}
        	}
        	return rightBridge;
        }
*/



/*
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
*/


        void completeLeftBridge(couplingMap couplings, couplingMap leftBridge, couplingMap& pCouplings) {
        	for (auto& c: couplings) {
        		for(auto& b: leftBridge) {
        			//pCouplings.insert({b.first, c.second});
        			pCouplings.insert({c.first, b.second});
        		}
        	}
        }

/*
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

        void completeRightBridge(couplingMap couplings, couplingMap rightBridge, couplingMap& pCouplings) {
        	for (auto& c: couplings) {
        		for(auto& b: rightBridge) {
        			//for(auto& s: b.second){
        				//couplingMap::iterator it = rightBridge.find(c.first);
        				//if(it == NULL){
        				//	rightBridge.insert({oPort, {}});
        				//}
        				//for (auto& sender: c.second){
        				//	it->second.push_back(sender);
        				//}
        				//pCouplings.insert({c.first, {s}});
        				//it->second.push_back(oPort);
        				//pCouplings.insert({c.first, b.second});
        				//pCouplings.insert({c.first, b.second});
        				pCouplings.insert({b.first, c.second});
        			//}
        		}
        	}
        }

    };
}

#endif //CADMIUM_CORE_MODELING_COUPLED_HPP_
