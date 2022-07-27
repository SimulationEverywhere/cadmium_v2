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

#include <iostream>

namespace cadmium {
	//! Couplings are just tuples <portFrom, portTo>
//    using coupling = std::tuple<const std::shared_ptr<PortInterface>, const std::shared_ptr<PortInterface>>;
	using coupling = std::tuple<std::shared_ptr<PortInterface>, std::shared_ptr<PortInterface>>;

	//! Class for coupled DEVS models.
    class Coupled: public Component {
     public:
        std::vector<std::shared_ptr<Component>> components;  //!< Components set.
        std::vector<coupling> EIC;                           //!< External Input Coupling set.
        std::vector<coupling> IC;                            //!< Internal Coupling set.
        std::vector<coupling> EOC;                           //!< External Output Coupling set.
     public:
		/**
		 * Constructor function.
		 * @param id ID of the coupled model.
		 */
        explicit Coupled(const std::string& id): Component(id), components(), EIC(), IC(), EOC() {}

		//! @return reference to the component set.
		std::vector<std::shared_ptr<Component>>& getComponents() {
			return components;
		}

		//! @return reference to the EIC set.
		std::vector<coupling>& getEICs() {
			return EIC;
		}

		//! @return reference to the IC set.
		std::vector<coupling>& getICs() {
			return IC;
		}

		//! @return reference to the EOC set.
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
 //       void addEIC(std::string& portFromId, std::string& componentToId, std::string& portToId) {
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


		void flatten(){
        	//std::vector<Coupled> toFlatten;
        	std::vector<std::shared_ptr<Coupled>> toFlatten;
        	for (auto& component: components) {
        		auto coupled = std::dynamic_pointer_cast<Coupled>(component);
        		if (coupled != nullptr) {
        			toFlatten.push_back(coupled);
        		}
        	}

        	//for (Coupled component : toFlatten){
        	for(auto itc = toFlatten.begin(); itc != toFlatten.end(); itc++){
        	//for (auto& component: toFlatten) {
        		//component.flatten();
        		(*itc)->flatten();
        		removePortsAndCouplings((*itc));
        		//remove(components.begin(),components.end(),(*itc));
        	//}


        		for(auto itcomp = components.begin(); itcomp != components.end(); itcomp++){
        		//for (auto& flat: components) {
        			//if(component.getId().compare((*itc)->getId()) == 0){
        			//auto coupled = std::dynamic_pointer_cast<Coupled>(*itcomp);
        			//auto coupled = std::dynamic_pointer_cast<Coupled>(flat);
        			//if(coupled != nullptr){
        				if(*itc == *itcomp) {
        					components.erase(itcomp);
        					//itcomp--;
        				}
        			//}
        		}

        	}

        	toFlatten.clear();

        	if(parent != nullptr) {

        		//get all parent ports connected to input ports
        		//std::vector<coupling> leftEIC = getLeftSides(parent.getEIC());
        		//std::vector<coupling> leftIC = getLeftSides(parent.getIC());
        		//auto parent_coupled = dynamic_pointer_cast<Coupled*>(parent->value());

        		//if EICs are connected to this model, connect parent EICs directly
        		//connectEICs(parent);

        		std::vector<std::shared_ptr<PortInterface>> leftBridgeEIC = createLeftBridge(parent->getEICs());
        		std::vector<std::shared_ptr<PortInterface>> leftBridgeIC = createLeftBridge(parent->getICs());

        		std::vector<std::shared_ptr<PortInterface>> rightBridgeEOC = createRightBridge(parent->getEOCs());
        		std::vector<std::shared_ptr<PortInterface>> rightBridgeIC = createRightBridge(parent->getICs());

        		completeLeftBridge(EIC, leftBridgeEIC, parent->getEICs());
        		completeLeftBridge(EIC, leftBridgeIC, parent->getICs());
        		completeRightBridge(EOC, rightBridgeEOC, parent->getEOCs());
        		completeRightBridge(EOC, rightBridgeIC, parent->getICs());

        		//if ICs are connected to this model, connect parent ICs directly
//        		connectToICs(parent);
        		//if EOCs are connected from this model, connect parent EOCs directly
//        		connectEOCs(parent);
        		//if ICs are connected from this model, connect parent ICs directly
//        		connectFromICs(parent);
        		//get all parent ports connected to output ports
        		//std::vector<coupling> leftEIC = getLeftSides(parent.getEIC());
        		//std::vector<coupling> leftIC = getLeftSides(parent.getEIC());

            	for (auto& component: components) {
            		parent->addComponent(component);
            	}

            	for (auto ic: IC) {
            		parent->getICs().push_back(ic);
            	}

        	}

		}

        void removePortsAndCouplings(std::shared_ptr<Coupled> child) {
        	std::vector<std::shared_ptr<PortInterface>> inPorts = child->getInPorts();
        	for(auto it = inPorts.begin(); it != inPorts.end(); it++){
        		for(auto itc = EIC.begin(); itc != EIC.end(); itc++){
            		//if((*it)->getId().compare(std::get<1>(*itc)->getId()) == 0){
        			if(*it == std::get<1>(*itc)) {
            			EIC.erase(itc);
            			//itc--;
            		}
        		}
        		for(auto itc = IC.begin(); itc != IC.end(); itc++){
            		//if((*it)->getId().compare(std::get<1>(*itc)->getId()) == 0){
        			if(*it == std::get<1>(*itc)) {
            			IC.erase(itc);
            			//itc--;
            		}
        		}
        	}

        	std::vector<std::shared_ptr<PortInterface>> outPorts = child->getOutPorts();
        	for(auto it = outPorts.begin(); it != outPorts.end(); it++){
        		for(auto itc = EOC.begin(); itc != EOC.end(); itc++){
        			//if((*it)->getId().compare(std::get<0>(*itc)->getId()) == 0){
        			if(*it == std::get<0>(*itc)) {
        				EOC.erase(itc);
        				//itc--;
        			}
        		}
        		for(auto itc = IC.begin(); itc != IC.end(); itc++){
        			//if((*it)->getId().compare(std::get<0>(*itc)->getId()) == 0){
        			if(*it == std::get<0>(*itc)) {
        				IC.erase(itc);
        				//itc--;
        			}
        		}
        	}
        }

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

        std::vector<std::shared_ptr<PortInterface>> createLeftBridge(std::vector<coupling> couplings) {
        	std::vector<std::shared_ptr<PortInterface>> leftBridge;
            for (auto iPort : this->getInPorts()) {
                for (auto c : couplings) {
                    if (std::get<1>(c) == iPort) {
                    	//std::cout << "ACA CreateLeftBridge" << std::endl;
                    	//leftBridge.push_back(iPort);
                    	leftBridge.push_back(std::get<0>(c));
                    }
                }
            }
            return leftBridge;
        }

        std::vector<std::shared_ptr<PortInterface>> createRightBridge(std::vector<coupling> couplings) {
        	std::vector<std::shared_ptr<PortInterface>> rightBridge;
            for (auto oPort : this->getOutPorts()) {
                for (auto c : couplings) {
                    if (std::get<0>(c) == oPort) {
                    	//std::cout << "ACA CreateRightBridge" << std::endl;
                    	//rightBridge.push_back(oPort);
                    	rightBridge.push_back(std::get<1>(c));
                    }
                }
            }
            return rightBridge;
        }

/*
    private void completeLeftBridge(LinkedList<Coupling<?>> couplings, HashMap<Port<?>, LinkedList<Port<?>>> leftBridge,
            LinkedList<Coupling<?>> pCouplings) {
        for (Coupling<?> c : couplings) {
            LinkedList<Port<?>> list = leftBridge.get(c.portFrom);
            if (list != null) {
                for (Port<?> port : list) {
                    pCouplings.add(new Coupling(port, c.portTo));
                }
            }
        }
    }
*/
        void completeLeftBridge(std::vector<coupling> couplings, std::vector<std::shared_ptr<PortInterface>> leftBridge,
        	std::vector<coupling>& pCouplings) {
        	for (auto c : couplings) {
        		for(auto portFrom: leftBridge) {
        			//if(std::get<0>(c) == portFrom){
        				std::shared_ptr<PortInterface> right = std::get<1>(c);
                		coupling tuple = std::make_tuple(portFrom, right);
                		//std::cout << "COMPLETE LEFT BRIDGE" << std::endl;
                		pCouplings.push_back(tuple);
        			//}
        		}
        	}
        }
/*
        private HashMap<Port<?>, LinkedList<Port<?>>> createRightBrige(LinkedList<Coupling<?>> couplings) {
            HashMap<Port<?>, LinkedList<Port<?>>> rightBridge = new HashMap<>();
            for (Port<?> oPort : this.outPorts) {
                for (Coupling<?> c : couplings) {
                    if (c.portFrom == oPort) {
                        LinkedList<Port<?>> list = rightBridge.get(oPort);
                        if (list == null) {
                            list = new LinkedList<>();
                            rightBridge.put(oPort, list);
                        }
                        list.add(c.portTo);
                    }
                }
            }
            return rightBridge;
        }
*/

        void completeRightBridge(std::vector<coupling> couplings, std::vector<std::shared_ptr<PortInterface>> rightBridge,
        	std::vector<coupling>& pCouplings) {
        	for (auto c : couplings) {
        		for(auto portTo: rightBridge) {
        			//if(std::get<1>(c) == portTo){
        				std::shared_ptr<PortInterface> left = std::get<0>(c);
                		coupling tuple = std::make_tuple(left, portTo);
                		//std::cout << "COMPLETE RIGHT BRIDGE" << std::endl;
                		pCouplings.push_back(tuple);
        			//}
        		}
        	}
        }



/*
        void connectEICs(Coupled* coupled_parent){
        	std::vector<std::shared_ptr<PortInterface>> inPorts = this->getInPorts();
        	for(auto it=inPorts.begin(); it!= inPorts.end(); it++){
        		for(auto itp=coupled_parent->getEICs().begin(); itp!= coupled_parent->getEICs().end(); itp++){

    				//std::cout << (*it)->getId() << std::endl;
    				//std::cout << std::get<1>(*itp)->getId() << std::endl;

        			//if((*it)->getId().compare(std::get<1>(*itp)->getId()) == 0){
        			if((*it) == std::get<1>(*itp)){
        				auto PortFrom = std::get<0>(*itp);
        				auto PortTo = (*it);

        				//coupled_parent->addEIC(leftSide->getId(), this->getId(), (*it)->getId());
        				//coupled_parent->addEIC(leftSide->getId(), this->getId(), "1");
        				//coupled_parent->getEICs().emplace_back(leftSide, *it);

        				coupling tuple = std::make_tuple(PortFrom, PortTo);

        				//std::cout << (*it)->getId() << std::endl;
        				//std::cout << std::get<1>(*itp)->getId() <<std::endl;

        				coupled_parent->getEICs().push_back(tuple);

						//coupled_parent->addDynamicEIC();


        				//coupled_parent->getEICs().push_back(tuple);
        				//std::cout << "ACAAAAAAAAAAA";
        		        //void addIC(const std::string& componentFromId, const std::string& portFromId, const std::string& componentToId, const std::string& portToId) {
        				//coupled_parent->addEIC(PortFrom->getId(), this->getId(), PortFrom->getId());

        				//void addEIC(const std::string& portFromId, const std::string& componentToId, const std::string& portToId) {

        				//coupled_parent->getEICs().emplace_back(PortFrom, PortTo);
        				//coupled_parent->addEIC("1", "2", "1");

        				//void addDynamicEIC(const std::string& portFromId, const std::string& componentToId, const std::string& portToId) {
        				//coupled_parent->addDynamicEIC(leftSide->getId(), this->getId(), (*it)->getId());
        				//coupled_parent->getEICs.
        			}
        		}
        	}
        }

        void connectToICs(Coupled* coupled_parent){
        	std::vector<std::shared_ptr<PortInterface>> inPorts = this->getInPorts();
        	for(auto it=inPorts.begin(); it!= inPorts.end(); it++){
        		for(auto itp=coupled_parent->getICs().begin(); itp!= coupled_parent->getICs().end(); itp++){
        			if((*it)->getId().compare(std::get<1>(*itp)->getId()) == 0){
        				auto leftSide = std::get<0>(*itp);
        		        //void addIC(const std::string& componentFromId, const std::string& portFromId, const std::string& componentToId, const std::string& portToId) {
        				coupled_parent->addIC(leftSide->getParent()->getId(), leftSide->getId(), this->getId(),(*it)->getId());
        			}
        		}
        	}
        }

        void connectEOCs(Coupled* coupled_parent){
        	std::vector<std::shared_ptr<PortInterface>> outPorts = this->getOutPorts();
        	for(auto it=outPorts.begin(); it!= outPorts.end(); it++){
        		for(auto itp=coupled_parent->getEOCs().begin(); itp!= coupled_parent->getEOCs().end(); itp++){
        			if((*it)->getId().compare(std::get<0>(*itp)->getId()) == 0){
        				auto rightSide = std::get<1>(*itp);
        				//coupled_parent->addEIC(leftSide->getId(), this->getId(),(*it)->getId());
        		        //void addEOC(const std::string& componentFromId, const std::string& portFromId, const std::string& portToId) {
        				coupled_parent->addEOC(this->getId(), (*it)->getId(), rightSide->getId());
        			}
        		}
        	}
        }

        void connectFromICs(Coupled* coupled_parent){
        	std::vector<std::shared_ptr<PortInterface>> outPorts = this->getOutPorts();
        	for(auto it=outPorts.begin(); it!= outPorts.end(); it++){
        		for(auto itp=coupled_parent->getICs().begin(); itp!= coupled_parent->getICs().end(); itp++){
        			if((*it)->getId().compare(std::get<0>(*itp)->getId()) == 0){
        				auto rightSide = std::get<1>(*itp);
        		        //void addIC(const std::string& componentFromId, const std::string& portFromId, const std::string& componentToId, const std::string& portToId) {
        				//coupled_parent->addIC(leftSide->getParent().value()->getId(), leftSide->getId(), this->getId(),(*it)->getId());
        				coupled_parent->addIC(this->getId(),(*it)->getId(), rightSide->getParent()->getId(), rightSide->getId());
        			}
        		}
        	}
        }
*/

    };
}

#endif //CADMIUM_CORE_MODELING_COUPLED_HPP_
