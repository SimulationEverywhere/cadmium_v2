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

#ifndef CADMIUM_MODELING_DEVS_COUPLED_HPP_
#define CADMIUM_MODELING_DEVS_COUPLED_HPP_

#include <cstring>
#ifndef NO_LOGGING
    #include <iostream>
#endif
#include <memory>
#include <string>
#include <tuple>
#include <typeinfo>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>
#include "component.hpp"
#include "port.hpp"
#include "../../exception.hpp"

namespace cadmium {
    //! Couplings are unordered maps {portTo: [portFrom1, portFrom2, ...]}.
    using MappedCouplings = std::unordered_map<std::shared_ptr<PortInterface>, std::vector<std::shared_ptr<PortInterface>>>;
    //! Serialized representation of couplings.
    using SerialCouplings = std::vector<std::tuple<std::shared_ptr<PortInterface>, std::shared_ptr<PortInterface>>>;

    //! Class for coupled DEVS models.
    class Coupled: public Component {
     protected:
        std::unordered_map<std::string, std::shared_ptr<Component>> components;  //!< Components set.
        MappedCouplings EIC;  //!< External Input Coupling set.
        MappedCouplings IC;   //!< Internal Coupling set.
        MappedCouplings EOC;  //!< External Output Coupling set.
        SerialCouplings serialEIC;  //!< Serialized representation of External Input Coupling set.
        SerialCouplings serialIC;   //!< Serialized representation of Internal Coupling set.
        SerialCouplings serialEOC;  //!< Serialized representation of External Output Coupling set.

        void addEIC(const std::shared_ptr<PortInterface>& portFrom, const std::shared_ptr<PortInterface>& portTo) {
            addCouplingToMap(EIC, portFrom, portTo);
            serialEIC.emplace_back(portFrom, portTo);
        }

        void addIC(const std::shared_ptr<PortInterface>& portFrom, const std::shared_ptr<PortInterface>& portTo) {
            addCouplingToMap(IC, portFrom, portTo);
            serialIC.emplace_back(portFrom, portTo);
        }

        void addEOC(const std::shared_ptr<PortInterface>& portFrom, const std::shared_ptr<PortInterface>& portTo) {
            addCouplingToMap(EOC, portFrom, portTo);
            serialEOC.emplace_back(portFrom, portTo);
        }

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
        const MappedCouplings& getEICs() {
            return EIC;
        }

        //! @return reference to the IC set.
        const MappedCouplings& getICs() {
            return IC;
        }

        //! @return reference to the EOC set.
        const MappedCouplings& getEOCs() {
            return EOC;
        }

        //! @return reference to the EIC set (serialized version).
        const SerialCouplings& getSerialEICs() {
            return serialEIC;
        }

        //! @return reference to the IC set (serialized version).
        const SerialCouplings& getSerialICs() {
            return serialIC;
        }

        //! @return reference to the EOC set (serialized version).
        const SerialCouplings& getSerialEOCs() {
            return serialEOC;
        }

        /**
         * Returns a pointer to a subcomponent with the provided ID.
         * @param id ID of the subcomponent
         * @return pointer to the subcomponent.
         * @throw CadmiumModelException if the component is not found.
         */
        [[nodiscard]] std::shared_ptr<Component> getComponent(const std::string& id) const {
            if(components.find(id)==components.end()) throw CadmiumModelException("component not found");
            return components.find(id)->second;
            // try {
            //     return components.at(id);
            // } catch (std::out_of_range& _) {
            //     throw CadmiumModelException("component not found");
            // }
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
            static_assert(std::is_base_of<Component, T>::value, "T must inherit cadmium::Component");
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
        [[nodiscard]] static bool containsCoupling(const MappedCouplings& couplings, const std::shared_ptr<PortInterface>& portFrom, const std::shared_ptr<PortInterface>& portTo) {
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
        static void addCouplingToMap(MappedCouplings& coupList, const std::shared_ptr<PortInterface>& portFrom, const std::shared_ptr<PortInterface>& portTo) {
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
                    addEIC(portFrom, portTo);
                } else {
                    throw CadmiumModelException("invalid destination port");
                }
            } else if (portFromParent->getParent() == this && portFromParent->containsOutPort(portFrom)) {
                if (containsOutPort(portTo)) {
                    addEOC(portFrom, portTo);
                } else if (portToParent->getParent() == this && portToParent->containsInPort(portTo)) {
                    addIC(portFrom, portTo);
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
            addEIC(portFrom, portTo);
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
            addIC(portFrom, portTo);
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
            addEOC(portFrom, portTo);
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
            addEIC(portFrom, portTo);
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
            addEOC(portFrom, portTo);
        }

        /**
         * Flattens coupled model. It only flattens lower-level coupled models.
         * If you want a complete model flattening, you must call this method from the topmost coupled model.
         */
        [[maybe_unused]] void flatten() {
            flatten(nullptr);
        }

     private:

        /**
         * Flattens a coupled model and, if required, passes all atomic subcomponents and couplings to the parent coupled.
         * @param parentPointer pointer to parent coupled. If null, flattening is not propagated to parent.
         */
        void flatten(Coupled * parentPointer) {
            // First, we identify the coupled subcomponents that need to be flattened
            std::vector<std::shared_ptr<Coupled>> toFlatten;
            for (auto& component: components) {
                auto coupled = std::dynamic_pointer_cast<Coupled>(component.second);
                if (coupled != nullptr) {
                    toFlatten.push_back(coupled);
                }
            }
            // Next, we flatten all the coupled subcomponents
            for (auto& coupled: toFlatten) {
                coupled->flatten(this);
                removeFlattenedCouplings(coupled);
                components.erase(coupled->getId());
            }
            // Finally, we deserialize the resulting couplings. The model is already flat!
            EIC = deserializeCouplings(serialEIC);
            IC = deserializeCouplings(serialIC);
            EOC = deserializeCouplings(serialEOC);

            // If pointer to parent is not null, we propagate the flattening to the corresponding parent coupled model.
            if(parentPointer != nullptr) {
                // We add pass the components to the parent coupled model
                for (auto& [componentId, component]: components) {
                    parentPointer->addComponent(component);
                }
                // We adapt EIC couplings and add to the parent coupled model
                leftCouplings(serialEIC, parentPointer->serialEIC);
                leftCouplings(serialEIC, parentPointer->serialIC);
                // We add pass the ICs to the parent coupled model
                parentPointer->serialIC.insert(parentPointer->serialIC.end(), serialIC.begin(), serialIC.end());
                // We adapt EOC couplings and add to the parent coupled model
                rightCouplings(serialEOC, parentPointer->serialIC);
                rightCouplings(serialEOC, parentPointer->serialEOC);
            }
        }

        /**
         * Translates a vector of couplings to a coupling map.
         * @param serial reference vector of couplings
         * @return an unordered map with the topology {port_to: [port_from_1, port_from_2, ...]}.
         */
        [[nodiscard]] static MappedCouplings deserializeCouplings(const SerialCouplings& serial) {
            MappedCouplings map;
            for(auto& [portFrom, portTo]: serial){
                addCouplingToMap(map, portFrom, portTo);  // this static method checks that there are no duplicates!
            }
            return map;
        }

        /**
         * It removes outdated couplings related to already flattened child coupled models.
         * @param child pointer to already flattened child coupled model.
         */
        void removeFlattenedCouplings(const std::shared_ptr<Coupled>& child) {
            for (auto itc = serialEIC.begin(); itc != serialEIC.end(); itc++) {
                if (child->containsInPort(std::get<1>(*itc))) {
                    serialEIC.erase(itc--);
                }
            }
            for (auto itc = serialIC.begin(); itc != serialIC.end(); itc++) {
                if (child->containsOutPort(std::get<0>(*itc)) || child->containsInPort(std::get<1>(*itc))) {
                    serialIC.erase(itc--);
                }
            }
            for (auto itc = serialEOC.begin(); itc != serialEOC.end(); itc++) {
                if (child->containsOutPort(std::get<0>(*itc))) {
                    serialEOC.erase(itc--);
                }
            }
        }

        /**
         * It adapts EICs of the component to either EICs or ICs of the parent component.
         * @param componentEIC serialized version of the component's EICs.
         * @param parentCouplings serialized version of the parent component's EICs or ICs.
         */
        void leftCouplings(const SerialCouplings& componentEIC, SerialCouplings& parentCouplings) {
            // First, we identify component's input ports affected by parent couplings
            MappedCouplings leftBridge;
            for (const auto& [portFrom, portTo]: parentCouplings) {
                if (containsInPort(portTo)) {  // If destination port is an input port, we add it to the bridge
                    addCouplingToMap(leftBridge, portFrom, portTo);
                }
            }
            // Then, we detect affected couplings and add them accordingly to the parent
            for (auto& [left, portTo]: componentEIC) {
                if (leftBridge.find(left) != leftBridge.end()) {
                    for (auto& portFrom: leftBridge.at(left)) {
                        parentCouplings.emplace_back(portFrom, portTo);
                    }
                }
            }
        }

        /**
         * It adapts EOCs of the component to either ICs or EOCs of the parent component.
         * @param componentEOC serialized version of the component's EOCs.
         * @param parentCouplings serialized version of the parent component's ICs or EOCs.
         */
        void rightCouplings(const SerialCouplings& componentEOC, SerialCouplings& parentCouplings) {
            // First, we identify component's output ports affected by parent couplings
            MappedCouplings rightBridge;
            for (const auto& [portFrom, portTo]: parentCouplings) {
                if (containsOutPort(portFrom)) {  // If origin port is an output port, we add it to the bridge
                    addCouplingToMap(rightBridge, portTo, portFrom);  // IMPORTANT: topology of right bridges is inverse
                }
            }
            // Then, we detect affected couplings and add them accordingly to the parent
            for (auto& [portFrom, right]: componentEOC) {
                if (rightBridge.find(right) != rightBridge.end()) {
                    for (auto& portTo: rightBridge.at(right)) {
                        parentCouplings.emplace_back(portFrom, portTo);
                    }
                }
            }
        }
    };
}

#endif //CADMIUM_MODELING_DEVS_COUPLED_HPP_
