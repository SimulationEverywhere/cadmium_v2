/**
 * Ports are used by PDEVS components to send messages to other PDEVS components.
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

#ifndef CADMIUM_CORE_MODELING_PORT_HPP_
#define CADMIUM_CORE_MODELING_PORT_HPP_

#include <algorithm>
#include <cstddef>
#include <memory>
#include <cstring>
#include <string>
#include <typeinfo>
#include <vector>
#include "component.hpp"
#include "../exception.hpp"

namespace cadmium {
    class Component;

	//! Abstract class to treat ports that holds messages of different data types equally.
    class PortInterface {
     private:
        std::string id;                             //!< ID of the DEVS port.
		std::shared_ptr<const Component *> parent;  //!< Pointer to parent component.
     public:
		/**
		 * Constructor function.
		 * @param id ID of the port to be created.
		 */
        explicit PortInterface(std::string id): id(std::move(id)), parent(std::make_shared<const Component *>(nullptr)) {}

		//! Default virtual destructor function.
		virtual ~PortInterface() = default;

		//! @return ID of the port.
        [[nodiscard]] const std::string& getId() const {
            return id;
        }

		//! @return pointer to the parent component of the port. It can be nullptr if the component has no parent.
        [[nodiscard]] const Component * getParent() const {
            return *parent;
        }

		/**
		 * Sets the port parent to the provided DEVS component.
		 * @param newParent pointer to new parent.
		 */
        void setParent(const Component * newParent) {
			*parent = newParent;
        }

		//! It clears all the messages in the port bag.
        virtual void clear() = 0;

		//! @return true if the port bag contains one or more messages.
        [[nodiscard]] virtual bool empty() const = 0;

		//! @return the number of messages within the port bag.
		[[nodiscard]] virtual std::size_t size() const = 0;

		/**
		 * Checks if the port can hold messages of the same type as other port.
		 * @param other pointer to the other port under study.
		 * @return true if both ports can hold messages of the same type.
		 */
		[[nodiscard]] virtual bool compatible(const std::shared_ptr<const PortInterface>& other) const = 0;

		/**
		 * It creates a new port that can hold messages of the same type as the port that invoked the method.
		 * @param portId ID of the new port.
		 * @return shared pointer to the new port.
		 */
		[[nodiscard]] virtual std::shared_ptr<PortInterface> newCompatiblePort(std::string portId) const = 0;

		/**
		 * It propagates all the messages from one port to the port that invoked this method.
		 * @param portFrom pointer to the port that holds the messages to be propagated.
		 * @throw CadmiumModelException if ports are not compatible (i.e., they contain messages of different data types).
		 */
        virtual void propagate(const std::shared_ptr<const PortInterface>& portFrom) = 0;

		//! @return a vector with string representations of each message in the port bag.
		[[nodiscard]] virtual std::vector<std::string> logMessages() const = 0;
    };

	/**
	 * @brief DEVS port with typed messages class.
	 *
	 * Typed ports can only hold messages of a given data type.
	 * @tparam T data type of the messages that the port can hold.
	 */
    template <typename T>
    class Port: public PortInterface {
     private:
        std::vector<std::shared_ptr<const T>> bag;  //!< message bag of the port.
     public:
		/**
		 * Constructor function of the Port<T> class.
		 * @param id ID of the port to be created.
		 */
        explicit Port(std::string id) : PortInterface(std::move(id)), bag() {}

		/**
		 * Static method to ease the creation of a shared pointer to a typed port.
		 * @param id ID of the port to be created.
		 * @return shared pointer to the newly created port.
		 */
        static std::shared_ptr<Port<T>> newPort(std::string id) {
            return std::make_shared<Port<T>>(std::move(id));
        }

		//! @return a reference to the port message bag.
        [[nodiscard]] const std::vector<std::shared_ptr<const T>>& getBag() const {
            return bag;
        }

		//! It clears all the messages inside the port bag.
        void clear() override {
            bag.clear();
        }

		//! @return true if the port bag is empty.
        [[nodiscard]] bool empty() const override {
            return bag.empty();
        }

		//! @return the number of messages within the port bag.
		[[nodiscard]] std::size_t size() const override {
			return bag.size();
		}

		/**
		 * adds a new message to the port bag.
		 * @param message new message to be added to the bag.
		 */
        void addMessage(const T message) {
            bag.push_back(std::make_shared<const T>(std::move(message)));
        }

		/**
		 * Static method that adds a message to a non-typed port. This method helps users to deal with dynamic pointer casts.
		 * @param port pointer to the non-typed port.
		 * @param message message to be added to the bag.
		 * @throw CadmiumModelException if the non-typed port cannot be casted to the corresponding typed port.
		 */
        static void addMessage(const std::shared_ptr<PortInterface>& port, const T message) {
            auto typedPort = std::dynamic_pointer_cast<Port<T>>(port);
            if (typedPort == nullptr) {
				throw CadmiumModelException("invalid port type");
            }
            typedPort->addMessage(std::move(message));
        }

		/**
		 * Checks if the port can hold messages of the same type as other port.
		 * @param other pointer to the other port under study.
		 * @return true if both ports can hold messages of the same type.
		 */
		[[nodiscard]] bool compatible(const std::shared_ptr<const PortInterface>& other) const override {
			return std::dynamic_pointer_cast<const Port<T>>(other) != nullptr;
		}

		/**
		 * It creates a new port that can hold messages of the same type as the port that invoked the method.
		 * @param portId ID of the new port.
		 * @return shared pointer to the new port.
		 */
		[[nodiscard]] std::shared_ptr<PortInterface> newCompatiblePort(std::string portId) const override {
			return std::make_shared<Port<T>>(std::move(portId));
		}

		/**
		 * It propagates all the messages from one port to the port that invoked this method.
		 * @param portFrom pointer to the port that holds the messages to be propagated.
		 * @throw CadmiumModelException if ports are not compatible (i.e., they contain messages of different data types).
		 */
        void propagate(const std::shared_ptr<const PortInterface>& portFrom) override {
            auto typedPort = std::dynamic_pointer_cast<const Port<T>>(portFrom);
            if (typedPort == nullptr) {
				throw CadmiumModelException("invalid port type");
            }
            bag.insert(bag.end(), typedPort->bag.begin(), typedPort->bag.end());
        }

		//! @return a vector with string representations of each message in the port bag.
		[[nodiscard]] std::vector<std::string> logMessages() const override {
			std::vector<std::string> logs;
			for (auto& msg: bag) {
				std::stringstream ss;
				ss << *msg;
				logs.push_back(ss.str());
			}
			return logs;
		}
    };

	//! Port set class. It is an interface to deal with more than one port.
    class PortSet {
     private:
        std::vector<std::shared_ptr<PortInterface>> ports;  //!< Pointers to the ports that comprise the port set.
     public:
		//! Constructor function.
        PortSet(): ports() {}

		//! Destructor function.
        ~PortSet() = default;

		//! @return reference to the ports comprising the port set.
		[[nodiscard]] const std::vector<std::shared_ptr<PortInterface>>& getPorts() const {
			return ports;
		}

		/**
		 * It returns a pointer to a non-typed port of the port set with a given ID.
		 * @param id ID of the requested port.
		 * @return pointer to the requested port.
		 * @throw CadmiumModelException if there is no port with the requested ID.
		 */
        [[nodiscard]] std::shared_ptr<PortInterface> getPort(const std::string& id) const {
            for (auto const& port: ports) {
                if (port->getId() == id) {
                    return port;
                }
            }
			throw CadmiumModelException("port not found");
        }

		/**
		 * It returns a pointer to a typed port of the port set with a given ID.
		 * @tparam T data type of the messages accepted by the requested port.
		 * @param id ID of the requested port.
		 * @return pointer to the requested port.
		 * @throw CadmiumModelException if there is no port with the requested ID or if the port type is invalid.
		 */
        template <typename T>
        std::shared_ptr<Port<T>> getPort(const std::string& id) const {
			auto port = std::dynamic_pointer_cast<Port<T>>(getPort(id));
			if (port == nullptr) {
				throw CadmiumModelException("invalid port type");
			}
			return port;
        }

		/**
		 * It returns a bag of a typed port of the port set.
		 * @tparam T data type of the messages within the bag.
		 * @param id ID of the requested port.
		 * @return reference to the requested port bag.
		 * @throw CadmiumModelException if there is no port with the requested ID or if the port type is invalid.
		 */
		template <typename T>
		[[maybe_unused]] const std::vector<std::shared_ptr<const T>>& getBag(const std::string& id) const {
			return getPort<T>(id)->getBag();
		}

		/**
		 * It adds a port to the port set.
		 * @param port pointer to the port to be added.
		 * @throw CadmiumModelException if there is already a port with this ID.
		 */
        void addPort(const std::shared_ptr<PortInterface>& port) {
			bool portIdAlreadyDefined = false;
			try {
				auto _port = getPort(port->getId());
			} catch (CadmiumModelException& ex) {
				if (std::strcmp(ex.what(), "port not found") != 0) {
					throw CadmiumModelException(ex.what());
				}
				portIdAlreadyDefined = true;
			}
			if (!portIdAlreadyDefined) {
				throw CadmiumModelException("port ID already defined");
			}
			ports.push_back(port);
        }

		/**
		 * Adds a new message to a port of the port set.
		 * @tparam T data type of the message.
		 * @param portId ID of the target port.
		 * @param message message to be added to the port.
		 * @throw CadmiumModelException if there is no port with the requested ID or if the port type is invalid.
		 */
        template <typename T>
        void addMessage(const std::string& portId, const T message) const {
			getPort<T>(portId)->addMessage(std::move(message));
        }

		//! @return true if any of the ports in the port set has one or more messages.
        [[nodiscard]] bool empty() const {
            return std::all_of(ports.begin(), ports.end(), [](auto const& port){ return port->empty(); });
        }

		/**
		 * Checks if a port is part of the port set.
		 * @param port shared pointer to the port under study.
		 * @return true if the port set contains a shared pointer to the port under study.
		 */
        [[nodiscard]] bool containsPort(const std::shared_ptr<PortInterface>& port) const {
            return std::any_of(ports.begin(), ports.end(), [port](auto const& p){ return p == port; });
        }

		//! It clears all the ports in the port set.
        void clear() {
            std::for_each(ports.begin(), ports.end(), [](auto& port) { port->clear(); });
        }
    };
}

#endif //CADMIUM_CORE_MODELING_PORT_HPP_
