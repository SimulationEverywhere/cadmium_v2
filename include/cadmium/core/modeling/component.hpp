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

#ifndef CADMIUM_CORE_MODELING_COMPONENT_HPP_
#define CADMIUM_CORE_MODELING_COMPONENT_HPP_

#include <exception>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
#include "port.hpp"
#include "../exception.hpp"

namespace cadmium {
	//! Abstract Base class of a DEVS component.
    class Component {
     protected:
		const std::string id;                       //!< ID of the DEVS component.
		std::shared_ptr<const Component *> parent;  //!< Pointer to parent component.
		PortSet inPorts;							//!< Input ports of the component.
		PortSet outPorts;                  			//!< Output ports of the component.
     public:
		/**
		 * Constructor function.
		 * @param id ID of the new DEVS components.
		 */
        explicit Component(std::string id): id(std::move(id)), parent(std::make_shared<const Component *>(nullptr)), inPorts(), outPorts() {}

		//! Destructor function.
		virtual ~Component() = default;

		//! @return ID of the DEVS component.
        [[nodiscard]] const std::string& getId() const {
            return id;
        }

		//! @return pointer to DEVS component's parent component. It can be nullptr if the component has no parent.
        [[nodiscard]] const Component * getParent() const {
            return *parent;
        }

		//! @return reference to the input port set.
		[[nodiscard]] const PortSet& getInPorts() const {
			return inPorts;
		}

		//! @return reference to the output port set.
		[[nodiscard]] const PortSet& getOutPorts() const {
			return outPorts;
		}

		/**
		 * Sets the component's parent to the provided DEVS component.
		 * @param newParent pointer to the new parent.
		 */
        void setParent(const Component * newParent) {
			*parent = newParent;
        }

		/**
		 * Checks if a port is part of the input port set of the component.
		 * @param port shared pointer to the port under study.
		 * @return true if the input port set contains a shared pointer to the port under study.
		 */
		[[nodiscard]] bool containsInPort(const std::shared_ptr<PortInterface>& port) const {
			return inPorts.containsPort(port);
		}

		/**
		 * Checks if a port is part of the output port set of the component.
		 * @param port shared pointer to the port under study.
		 * @return true if the output port set contains a shared pointer to the port under study.
		 */
		[[nodiscard]] bool containsOutPort(const std::shared_ptr<PortInterface>& port) const {
			return outPorts.containsPort(port);
		}

		/**
		 * returns pointer an input port. The port is not casted to any type yet.
		 * @param id Identifier of the input port.
		 * @return pointer to the input port.
		 * @throws CadmiumModelException if there is no input port with the provided ID.
		 */
        [[nodiscard]] std::shared_ptr<PortInterface> getInPort(const std::string& id) const {
            return inPorts.getPort(id);
        }

		/**
		 * Returns pointer to an input port. The port is dynamically casted according to the desired message type.
		 * @tparam T expected type of the input port.
		 * @param id Identifier of the input port.
		 * @return pointer to the input port.
		 * @throws CadmiumModelException if there is no input port with the provided ID or if the port type is invalid.
		 */
		template <typename T>
		std::shared_ptr<Port<T>> getInPort(const std::string& id) const {
			return inPorts.getPort<T>(id);
		}

		/**
		 * returns pointer an output port. The port is not casted to any type yet.
		 * @param id Identifier of the output port.
		 * @return pointer to the output port.
		 * @throws CadmiumModelException if there is no output port with the provided ID.
		 */
		[[nodiscard]] std::shared_ptr<PortInterface> getOutPort(const std::string& id) const {
			return outPorts.getPort(id);
		}

		/**
		 * Returns pointer to an output port. The port is dynamically casted according to the desired message type.
		 * @tparam T expected type of the output port.
		 * @param id Identifier of the output port.
		 * @return pointer to the output port.
		 * @throws CadmiumModelException if there is no input port with the provided ID or if the port type is invalid.
		 */
        template <typename T>
        std::shared_ptr<Port<T>> getOutPort(const std::string& id) const {
            return outPorts.getPort<T>(id);
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
			inPorts.addPort(port);
			port->setParent(this);
        }

		/**
		 * Creates and adds a new input port to the component.
		 * @tparam T desired type of the input port.
		 * @param id Identifier of the new input port.
		 * @return pointer to the newly created port.
		 * @throws CadmiumModelException if there is already an input port with the same ID.
		 */
        template <typename T>
        std::shared_ptr<Port<T>> addInPort(const std::string id) {
			auto port = std::make_shared<Port<T>>(id);
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
			outPorts.addPort(port);
			port->setParent(this);
        }

		/**
		 * Creates and adds a new output port to the component.
		 * @tparam T desired type of the output port.
		 * @param id Identifier of the new output port.
		 * @return pointer to the newly created port.
		 * @throws CadmiumModelException if there is already an output port with the same ID.
		 */
        template <typename T>
        std::shared_ptr<Port<T>> addOutPort(const std::string id) {
			auto port = std::make_shared<Port<T>>(id);
            addOutPort(port);
			return port;
        }

		//! @return true if all the input ports are empty.
        [[nodiscard]] bool inEmpty() const {
            return inPorts.empty();
        }

		//! @return true if all the output ports are empty.
		[[maybe_unused]] [[nodiscard]] bool outEmpty() const {
            return outPorts.empty();
        }

		//! It clears all the input/output ports of the DEVS component.
		void clearPorts() {
			inPorts.clear();
			outPorts.clear();
		}
    };
}

#endif //CADMIUM_CORE_MODELING_COMPONENT_HPP_
