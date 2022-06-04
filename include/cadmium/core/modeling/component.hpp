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

#ifndef _CADMIUM_CORE_MODELING_COMPONENT_HPP_
#define _CADMIUM_CORE_MODELING_COMPONENT_HPP_

#include <exception>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
#include "port.hpp"
#include "../exception.hpp"

namespace cadmium {

	/// Abstract Base class of a DEVS component.
    class Component {
     protected:
		const std::string id;                       /// ID of the DEVS component
		std::shared_ptr<const Component *> parent;  /// Pointer to parent component.
		PortSet inPorts, outPorts;                  /// input and output ports of the component.
     public:
        explicit Component(std::string id): id(std::move(id)), parent(std::make_shared<const Component *>(nullptr)), inPorts(), outPorts() {}
        virtual ~Component() = default;

		/// @return ID of the DEVS component
        [[nodiscard]] const std::string& getId() const {
            return id;
        }

		/// @return shared pointer to DEVS component's parent component. It can be nullptr if the component has no parent.
        [[nodiscard]] const Component * getParent() const {
            return *parent;
        }

		[[nodiscard]] const PortSet& getInPorts() const {
			return inPorts;
		}

		[[nodiscard]] const PortSet& getOutPorts() const {
			return outPorts;
		}

		/**
		 * Sets the component's parent to the provided DEVS component.
		 * @param newParent new  component's parent.
		 */
        void setParent(const Component * newParent) {
			*parent = newParent;
        }

		/**
		 * returns pointer an input port. The port is not casted to any type yet.
		 * @param id Identifier of the input port.
		 * @return pointer to the input port. If nullptr, there is no input port with the provided ID.
		 */
        [[nodiscard]] std::shared_ptr<PortInterface> getInPort(const std::string& id) const {
            return inPorts.getPort(id);
        }

		[[nodiscard]] bool containsInPort(const std::shared_ptr<PortInterface>& port) const {
			return inPorts.containsPort(port);
		}

		[[nodiscard]] bool containsOutPort(const std::shared_ptr<PortInterface>& port) const {
			return outPorts.containsPort(port);
		}

		/**
		 * Returns pointer to an input port. The port is dynamically casted according to the desired message type.
		 * @tparam T expected type of the input port.
		 * @param id Identifier of the input port.
		 * @return pointer to the input port. If nullptr, there is no input port with the provided ID or dynamic cast failed.
		 */
        template <typename T>
        std::shared_ptr<Port<T>> getInPort(const std::string& id) const {
            return inPorts.getPort(id);
        }

		/**
		 * returns pointer an output port. The port is not casted to any type yet.
		 * @param id Identifier of the output port.
		 * @return pointer to the output port. If nullptr, there is no output port with the provided ID.
		 */
        [[nodiscard]] std::shared_ptr<PortInterface> getOutPort(const std::string& id) const {
            return outPorts.getPort(id);
        }

		/**
		 * Returns pointer to an output port. The port is dynamically casted according to the desired message type.
		 * @tparam T expected type of the output port.
		 * @param id Identifier of the output port.
		 * @return pointer to the output port. If nullptr, there is no output port with the provided ID or dynamic cast failed.
		 */
        template <typename T>
        std::shared_ptr<Port<T>> getOutPort(const std::string& id) const {
            return outPorts.getPort(id);
        }

		/**
		 * Adds a new input port to the component.
		 * @param port pointer to the port interface to be added to the input interface of the component.
		 */
        void addInPort(const std::shared_ptr<PortInterface>& port) {
			if (port->getParent() != nullptr) {
				throw CadmiumModelException("Port " + port->getId() + " already belongs to model " + port->getParent()->getId());
			}
			port->setParent(this);
            inPorts.addPort(port);
        }

		/**
		 * Adds a new input port to the component.
		 * @tparam T type of the input port.
		 * @param port typed port to be added to the input interface of the component.
		 */
		template <typename T>
		[[maybe_unused]] void addInPort(Port<T> port) {
			addInPort(std::make_shared<Port<T>>(std::move(port)));
		}

		/**
		 * Creates and adds a new input port to the component.
		 * @tparam T desired type of the input port.
		 * @param id Identifier of the new input port.
		 */
        template <typename T>
        [[maybe_unused]] void addInPort(const std::string id) {
            addInPort(std::make_shared<Port<T>>(id));
        }

		/**
		 * Adds a new output port to the component.
		 * @param port pointer to the port interface to be added to the output interface of the component.
		 */
        void addOutPort(const std::shared_ptr<PortInterface>& port) {
			if (port->getParent() != nullptr) {
				throw CadmiumModelException("Port " + port->getId() + " already belongs to model " + port->getParent()->getId());
			}
			port->setParent(this);
            outPorts.addPort(port);
        }

		/**
		 * Adds a new output port to the component.
		 * @tparam T type of the output port.
		 * @param port typed port to be added to the output interface of the component.
		 */
		template <typename T>
		[[maybe_unused]] void addOutPort(Port<T> port) {
			addOutPort(std::make_shared<Port<T>>(std::move(port)));
		}

		/**
		 * Creates and adds a new output port to the component.
		 * @tparam T desired type of the output port.
		 * @param id Identifier of the new output port.
		 */
        template <typename T>
        [[maybe_unused]] void addOutPort(const std::string id) {
            addOutPort(std::make_shared<Port<T>>(id));
        }

		/// @return true if all the input ports are empty.
        [[nodiscard]] bool inEmpty() const {
            return inPorts.empty();
        }

		/// @return true if all the output ports are empty.
		[[maybe_unused]] [[nodiscard]] bool outEmpty() const {
            return outPorts.empty();
        }

		/// It clears all the input/output ports of the DEVS component.
		void clearPorts() {
			inPorts.clear();
			outPorts.clear();
		}
    };
}

#endif //_CADMIUM_CORE_MODELING_COMPONENT_HPP_