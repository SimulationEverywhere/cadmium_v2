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
        std::string id;            //!< ID of the DEVS port.
		const Component * parent;  //!< Pointer to parent component.
     public:
		/**
		 * Constructor function.
		 * @param id ID of the port to be created.
		 */
        explicit PortInterface(std::string id): id(std::move(id)), parent(nullptr) {}

		//! Default virtual destructor function.
		virtual ~PortInterface() = default;

		//! @return ID of the port.
        [[nodiscard]] const std::string& getId() const {
            return id;
        }

		//! @return pointer to the parent component of the port. It can be nullptr if the component has no parent.
        [[nodiscard]] const Component * getParent() const {
            return parent;
        }

		/**
		 * Sets the port parent to the provided DEVS component.
		 * @param newParent pointer to new parent.
		 */
        void setParent(const Component * newParent) {
			parent = newParent;
        }

		//! It clears all the messages in the port bag.
        virtual void clear() = 0;

		//! @return true if the port bag does not contain any message.
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
		 * Creates and adds a new subcomponent. Then, it returns a pointer to the new component.
		 * @tparam Args data types of all the constructor fields of the new component.
		 * @param args extra parameters required to generate the new component.
		 * @return pointer to the new component.
		 */
		template <typename... Args>
		void addMessage(Args&&... args) {
			addMessage(T(std::forward<Args>(args)...));
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
}

#endif //CADMIUM_CORE_MODELING_PORT_HPP_
