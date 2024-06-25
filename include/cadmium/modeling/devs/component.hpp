/**
 * Abstract implementations of a DEVS component.
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

#ifndef CADMIUM_MODELING_DEVS_COMPONENT_HPP_
#define CADMIUM_MODELING_DEVS_COMPONENT_HPP_

#include <exception>
#include <memory>
#ifndef NO_LOGGING
	#include <sstream>
#endif
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include "port.hpp"
#include "../../exception.hpp"

namespace cadmium {

	class Coupled;

	//! Abstract Base class of a DEVS component.
    class Component {
     private:
        std::vector<std::shared_ptr<PortInterface>> serialInPorts;   //!< Serialized version of component input ports.
        std::vector<std::shared_ptr<PortInterface>> serialOutPorts;  //!< Serialized version of component output ports.
     protected:
		const std::string id;                                                      //!< ID of the DEVS component.
		const Coupled * parent;                                                    //!< Pointer to parent component.
		std::unordered_map<std::string, std::shared_ptr<PortInterface>> inPorts;   //!< Input ports of the component.
        std::unordered_map<std::string, std::shared_ptr<PortInterface>> outPorts;  //!< Output ports of the component.
     public:
		/**
		 * Constructor function.
		 * @param id ID of the new DEVS components.
		 */
        explicit Component(std::string id): serialInPorts(), serialOutPorts(),
                                            id(std::move(id)), parent(nullptr), inPorts(), outPorts() {}

		//! Destructor function.
		virtual ~Component() = default;

		//! @return ID of the DEVS component.
        [[nodiscard]] const std::string& getId() const {
            return id;
        }

		//! @return pointer to DEVS component's parent component. It can be nullptr if the component has no parent.
        [[nodiscard]] const Coupled * getParent() const {
            return parent;
        }

		//! @return reference to the input port set.
		[[nodiscard]] const std::vector<std::shared_ptr<PortInterface>>& getInPorts() const {
			return serialInPorts;
		}

		//! @return reference to the output port set.
		[[nodiscard]] const std::vector<std::shared_ptr<PortInterface>>& getOutPorts() const {
			return serialOutPorts;
		}

		/**
		 * Sets the component's parent to the provided DEVS component.
		 * @param newParent pointer to the new parent.
		 */
        void setParent(Coupled * newParent) {
			parent = newParent;
        }

		/**
		 * Checks if a port is part of the input port set of the component.
		 * @param port shared pointer to the port under study.
		 * @return true if the input port set contains a shared pointer to the port under study.
		 */
		[[nodiscard]] bool containsInPort(const std::shared_ptr<PortInterface>& port) const {
            return inPorts.find(port->getId()) != inPorts.end() && inPorts.at(port->getId()) == port;
		}

		/**
		 * Checks if a port is part of the output port set of the component.
		 * @param port shared pointer to the port under study.
		 * @return true if the output port set contains a shared pointer to the port under study.
		 */
		[[nodiscard]] bool containsOutPort(const std::shared_ptr<PortInterface>& port) const {
            return outPorts.find(port->getId()) != outPorts.end() && outPorts.at(port->getId()) == port;
		}

		/**
		 * returns pointer an input port. The port is not casted to any type yet.
		 * @param id Identifier of the input port.
		 * @return pointer to the input port.
		 * @throws CadmiumModelException if there is no input port with the provided ID.
		 */
        [[nodiscard]] std::shared_ptr<PortInterface> getInPort(const std::string& id) const {
			if(inPorts.find(id)==inPorts.end()) throw CadmiumModelException("port not found");
			return inPorts.find(id)->second;
            // try {
            //     return inPorts.at(id);
            // } catch (...) {
            //     throw CadmiumModelException("port not found");
            // }
        }

		/**
		 * Returns pointer to an input port. The port is dynamically casted according to the desired message type.
		 * @tparam T expected type of the input port.
		 * @param id Identifier of the input port.
		 * @return pointer to the input port.
		 * @throws CadmiumModelException if there is no input port with the provided ID or if the port type is invalid.
		 */
		template <typename T>
		Port<T> getInPort(const std::string& id) const {
			auto port = std::dynamic_pointer_cast<_Port<T>>(getInPort(id));
			if (port == nullptr) {
				throw CadmiumModelException("invalid port type");
			}
			return port;
		}

		/**
		 * returns pointer an output port. The port is not casted to any type yet.
		 * @param id Identifier of the output port.
		 * @return pointer to the output port.
		 * @throws CadmiumModelException if there is no output port with the provided ID.
		 */
		[[nodiscard]] std::shared_ptr<PortInterface> getOutPort(const std::string& id) const {
			if(outPorts.find(id)==outPorts.end()) throw CadmiumModelException("port not found");
			return outPorts.find(id)->second;
            // try {
            //     return outPorts.at(id);
            // } catch (...) {
            //     throw CadmiumModelException("port not found");
            // }
		}

		/**
		 * Returns pointer to an output port. The port is dynamically casted according to the desired message type.
		 * @tparam T expected type of the output port.
		 * @param id Identifier of the output port.
		 * @return pointer to the output port.
		 * @throws CadmiumModelException if there is no input port with the provided ID or if the port type is invalid.
		 */
        template <typename T>
		Port<T> getOutPort(const std::string& id) const {
			auto port = std::dynamic_pointer_cast<_Port<T>>(getOutPort(id));
			if (port == nullptr) {
				throw CadmiumModelException("invalid port type");
			}
			return port;
        }

		/**
		 * Adds a new input port to the component.
		 * @param port pointer to the port interface to be added to the input interface of the component.
		 * @throws CadmiumModelException if port already belongs to other component or if there is already an input port with the same ID.
		 */
        void addInPort(const std::shared_ptr<PortInterface>& port) {
			if (port->getParent() != nullptr) {
				throw CadmiumModelException("port already belongs to other component");
			}
            if (inPorts.find(port->getId()) != inPorts.end()) {
                throw CadmiumModelException("port ID already defined");
            }
			port->setParent(this);
            serialInPorts.push_back(port);
            inPorts[port->getId()] = port;
        }

		/**
		 * Creates and adds a new input port to the component.
		 * @tparam T desired type of the input port.
		 * @param id Identifier of the new input port.
		 * @return pointer to the newly created port.
		 * @throws CadmiumModelException if there is already an input port with the same ID.
		 */
        template <typename T>
        [[maybe_unused]] Port<T> addInPort(const std::string id) {
			auto port = std::make_shared<_Port<T>>(id);
            addInPort(port);
			return port;
        }

		/**
		 * Creates and adds a new input big port to the component.
		 * @tparam T desired type of the input big port.
		 * @param id Identifier of the new input big port.
		 * @return pointer to the newly created big port.
		 * @throws CadmiumModelException if there is already an input port with the same ID.
		 */
		template <typename T>
		[[maybe_unused]] BigPort<T> addInBigPort(const std::string id) {
			auto port = std::make_shared<_BigPort<T>>(id);
			addInPort(port);
			return port;
		}

		/**
		 * Adds a new output port to the component.
		 * @param port pointer to the port interface to be added to the output interface of the component.
		 * @throws CadmiumModelException if port already belongs to other component or if there is already an output port with the same ID.
		 */
        void addOutPort(const std::shared_ptr<PortInterface>& port) {
			if (port->getParent() != nullptr) {
				throw CadmiumModelException("port already belongs to other component");
			}
            if (outPorts.find(port->getId()) != outPorts.end()) {
                throw CadmiumModelException("port ID already defined");
            }
			port->setParent(this);
            serialOutPorts.push_back(port);
            outPorts[port->getId()] = port;
        }

		/**
		 * Creates and adds a new output port to the component.
		 * @tparam T desired type of the output port.
		 * @param id Identifier of the new output port.
		 * @return pointer to the newly created port.
		 * @throws CadmiumModelException if there is already an output port with the same ID.
		 */
        template <typename T>
		[[maybe_unused]] Port<T> addOutPort(const std::string id) {
			auto port = std::make_shared<_Port<T>>(id);
            addOutPort(port);
			return port;
        }

		/**
		 * Creates and adds a new output big port to the component.
		 * @tparam T desired type of the output big port.
		 * @param id Identifier of the new output big port.
		 * @return pointer to the newly created big port.
		 * @throws CadmiumModelException if there is already an output port with the same ID.
		 */
		template <typename T>
		[[maybe_unused]] BigPort<T> addOutBigPort(const std::string id) {
			auto port = std::make_shared<_BigPort<T>>(id);
			addOutPort(port);
			return port;
		}

		//! @return true if all the input ports are empty.
        [[nodiscard]] bool inEmpty() const {
			return std::all_of(serialInPorts.begin(), serialInPorts.end(), [](auto const& port){ return port->empty(); });
        }

		//! @return true if all the output ports are empty.
		[[maybe_unused]] [[nodiscard]] bool outEmpty() const {
			return std::all_of(serialOutPorts.begin(), serialOutPorts.end(), [](auto const& port){ return port->empty(); });
        }

		//! It clears all the input/output ports of the DEVS component.
		void clearPorts() {
			std::for_each(serialInPorts.begin(), serialInPorts.end(), [](auto& port) { port->clear(); });
			std::for_each(serialOutPorts.begin(), serialOutPorts.end(), [](auto& port) { port->clear(); });
		}
    };
}

#endif //CADMIUM_MODELING_DEVS_COMPONENT_HPP_
