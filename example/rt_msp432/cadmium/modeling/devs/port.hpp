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

#ifndef CADMIUM_MODELING_DEVS_PORT_HPP_
#define CADMIUM_MODELING_DEVS_PORT_HPP_

#include <algorithm>
#include <cstddef>
#include <memory>
#include <cstring>
#include <string>
#include <typeinfo>
#include <vector>
#include "component.hpp"
#include "../../exception.hpp"

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
         * @throw CadmiumModelException if ports are not compatible (i.e., they contain different types of message).
         */
        virtual void propagate(const std::shared_ptr<const PortInterface>& portFrom) = 0;

    #ifndef NO_LOGGING
        /**
         * It logs a single message of the port bag.
         * @param i index in the bag of the message to be logged.
         * @return a string representation of the ith message in the port bag.
         */
        [[nodiscard]] virtual std::string logMessage(std::size_t i) const = 0;  // TODO change to lazy iterator
    #endif
    };

    /**
     * @brief DEVS port with typed messages class.
     *
     * Typed ports can only hold messages of a given data type.
     * NOTE: modelers don't have to deal with objects of the _Port<T> class. They always interface with Port<T> objects.
     *
     * @tparam T data type of the messages that the port can hold.
     */
    template <typename T>
    class _Port: public PortInterface {
     protected:
        std::vector<T> bag;  //!< message bag of the port.
     public:
        /**
         * Constructor function of the Port<T> class.
         * @param id ID of the port to be created.
         */
        explicit _Port(std::string id) : PortInterface(std::move(id)), bag() {}

        //! @return a reference to the port message bag.
        [[nodiscard]] const std::vector<T>& getBag() const {
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
            bag.push_back(std::move(message));
        }

        /**
         * Checks if the port can hold messages of the same type as other port.
         * @param other pointer to the other port under study.
         * @return true if both ports can hold messages of the same type.
         */
        [[nodiscard]] bool compatible(const std::shared_ptr<const PortInterface>& other) const override {
            return std::dynamic_pointer_cast<const _Port<T>>(other) != nullptr;
        }

        /**
         * It creates a new port that can hold messages of the same type as the port that invoked the method.
         * @param portId ID of the new port.
         * @return shared pointer to the new port.
         */
        [[nodiscard]] std::shared_ptr<PortInterface> newCompatiblePort(std::string portId) const override {
            return std::make_shared<_Port<T>>(std::move(portId));
        }

        /**
         * It propagates all the messages from one port to the port that invoked this method.
         * @param portFrom pointer to the port that holds the messages to be propagated.
         * @throw CadmiumModelException if ports are not compatible (i.e., they contain messages of different data types).
         */
        void propagate(const std::shared_ptr<const PortInterface>& portFrom) override {
            auto typedPort = std::dynamic_pointer_cast<const _Port<T>>(portFrom);
            if (typedPort == nullptr) {
                throw CadmiumModelException("invalid port type");
            }
            bag.insert(bag.end(), typedPort->bag.begin(), typedPort->bag.end());
        }

    #ifndef NO_LOGGING
        /**
         * It logs a given message of the bag.
         * @param i index in the bag of the message to be logged.
         * @return a string representation of the ith message in the port bag.
         */
        [[nodiscard]] std::string logMessage(std::size_t i) const override {
            std::stringstream ss;
            ss << bag.at(i);
            return ss.str();
        }
    #endif
    };

    //! Type alias to work with shared pointers pointing to _Port<T> objects with less boilerplate code.
    template <typename T>
    using Port = std::shared_ptr<_Port<T>>;

    /**
     * @brief typed port for big messages.
     *
     * Messages are stored and passed as shared pointers to constant messages to save memory.
     * NOTE: modelers don't have to deal with the _BigPort<T> class. They always interface with BigPort<T> objects.
     *
     * @tparam T Data type of the big messages stored by the port.
     */
    template <typename T>
    class _BigPort: public _Port<std::shared_ptr<const T>> {
        using _Port<std::shared_ptr<const T>>::bag;
     public:
        /**
         * Constructor function of the BigPort<T> class.
         * @param id ID of the port to be created.
         */
        explicit _BigPort(std::string id): _Port<std::shared_ptr<const T>>(std::move(id)){}

        /**
         * Adds a new message to the big port bag. It hides the complexity of creating a shared pointer.
         * @param message new message to be added to the bag.
         */
        void addMessage(const T message) {
            bag.push_back(std::make_shared<const T>(std::move(message)));
        }

        /**
         * Creates and adds a new message. It hides the complexity of creating a shared pointer.
         * @tparam Args data types of all the constructor fields of the new message.
         * @param args extra parameters required to generate the new message.
         */
        template <typename... Args>
        void addMessage(Args&&... args) {
            bag.push_back(std::make_shared<const T>(std::forward<Args>(args)...));
        }

    #ifndef NO_LOGGING
        /**
         * It logs a single message of the bag.
         * @param i index in the bag of the message to be logged.
         * @return a string representation of the ith message in the port bag.
         */
        [[nodiscard]] std::string logMessage(std::size_t i) const override {
            std::stringstream ss;
            ss << *bag.at(i);
            return ss.str();
        }
    #endif
    };

    //! Type alias to work with shared pointers pointing to _BigPort<T> objects with less boilerplate code.
    template <typename T>
    using BigPort = std::shared_ptr<_BigPort<T>>;
}

#endif //CADMIUM_MODELING_DEVS_PORT_HPP_
