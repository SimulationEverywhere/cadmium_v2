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

#ifndef _CADMIUM_CORE_MODELING_PORT_HPP_
#define _CADMIUM_CORE_MODELING_PORT_HPP_

#include <algorithm>
#include <memory>
#include <string>
#include <typeinfo>
#include <vector>
#include "component.hpp"
#include "../exception.hpp"

namespace cadmium {

    class Component;

    class PortInterface {
     private:
        friend struct PortSet;
        std::string id;
		std::shared_ptr<const Component *> parent;
     public:
        explicit PortInterface(std::string id): id(std::move(id)), parent(std::make_shared<const Component *>(nullptr)) {}
        virtual ~PortInterface() = default;

        [[maybe_unused]] [[nodiscard]] const std::string& getId() const {
            return id;
        }

        [[nodiscard]] const Component * getParent() const {
            return *parent;
        }

        void setParent(const Component * newParent) {
			*parent = newParent;
        }

        virtual void clear() = 0;
        [[nodiscard]] virtual bool empty() const = 0;
        [[nodiscard]]virtual bool compatible(const std::shared_ptr<const PortInterface>& other) const = 0;
        virtual void propagate(const std::shared_ptr<const PortInterface>& port_from) = 0;
		[[nodiscard]] virtual std::vector<std::string> logMessages() const = 0;
    };

    template <typename T>
    class Port: public PortInterface {
     private:
        std::vector<std::shared_ptr<const T>> bag;
     public:
        explicit Port(std::string id) : PortInterface(std::move(id)), bag() {}
		~Port() override = default;

        static std::shared_ptr<Port<T>> newPort(std::string id) {
            return std::make_shared<Port<T>>(std::move(id));
        }

        [[nodiscard]] const std::vector<std::shared_ptr<const T>>& getBag() const {
            return bag;
        }

        void clear() override {
            bag.clear();
        }

        [[nodiscard]] bool empty() const override {
            return bag.empty();
        }

        void addMessage(const T message) {
            bag.push_back(std::make_shared<const T>(std::move(message)));
        }

        static void addMessage(const std::shared_ptr<PortInterface>& port, const T message) {
            auto typedPort = std::dynamic_pointer_cast<Port<T>>(port);
            if (typedPort == nullptr) {
				throw CadmiumModelException("Port " + port->getId() + " does not support this message type");
            }
            typedPort->addMessage(std::move(message));
        }

        [[nodiscard]] bool compatible(const std::shared_ptr<const PortInterface>& other) const override {
            return std::dynamic_pointer_cast<const Port<T>>(other) != nullptr;
        }

        void propagate(const std::shared_ptr<const PortInterface>& portFrom) override {
            auto typedPort = std::dynamic_pointer_cast<const Port<T>>(portFrom);
            if (typedPort == nullptr) {
				throw CadmiumModelException("Origin port " + portFrom->getId() + " is not compatible with destination port " + getId());
            }
            bag.insert(bag.end(), typedPort->bag.begin(), typedPort->bag.end());
        }

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

    class PortSet {
     private:
        std::vector<std::shared_ptr<PortInterface>> ports;
     public:
        PortSet(): ports() {}
        ~PortSet() = default;

		[[nodiscard]] const std::vector<std::shared_ptr<PortInterface>>& getPorts() const {
			return ports;
		}

        [[nodiscard]] std::shared_ptr<PortInterface> getPort(const std::string& id) const {
            for (auto const& port: ports) {
                if (port->id == id) {
                    return port;
                }
            }
            return nullptr;
        }

        template <typename T>
        std::shared_ptr<Port<T>> getPort(const std::string& id) const {
            return std::dynamic_pointer_cast<Port<T>>(getPort(id));
        }

		template <typename T>
		[[maybe_unused]] const std::vector<std::shared_ptr<const T>>& getBag(const std::string& id) const {
			auto port = getPort<T>(id);
			if (port == nullptr) {
				throw CadmiumModelException("Port " + port->getId() + " is incompatible with this type");
			}
			return port->getBag();
		}

        void addPort(const std::shared_ptr<PortInterface>& port) {
			if (getPort(port->id) != nullptr) {
				throw CadmiumModelException("Another port with ID " + port->getId() + " is already defined");
            }
            ports.push_back(port);
        }

        template <typename T>
        void addMessage(const std::string& portId, const T message) const {
            Port<T>::addMessage(getPort(portId), std::move(message));
        }

        [[nodiscard]] bool empty() const {
            return std::all_of(ports.begin(), ports.end(), [](auto const& port){ return port->empty(); });
        }

        [[nodiscard]] bool containsPort(const std::shared_ptr<PortInterface>& port) const {
            return std::any_of(ports.begin(), ports.end(), [port](auto const& p){ return p == port; });
        }

        void clear() {
            std::for_each(ports.begin(), ports.end(), [](auto& port) { port->clear(); });
        }
    };
}

#endif //_CADMIUM_CORE_MODELING_PORT_HPP_
