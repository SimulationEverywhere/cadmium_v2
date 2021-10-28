#ifndef _CADMIUM_CORE_MODELING_COMPONENT_HPP_
#define _CADMIUM_CORE_MODELING_COMPONENT_HPP_

#include <exception>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include "port.hpp"

namespace cadmium {

    struct ComponentInterface {
        std::string id;
        std::weak_ptr<ComponentInterface> parent;
        PortSet inPorts, outPorts;

        explicit ComponentInterface(std::string id) : id(std::move(id)), parent(), inPorts(), outPorts() {};
        ~ComponentInterface() = default;
    };

    class Component {
     protected:
        friend class AbstractSimulator;
        std::shared_ptr<ComponentInterface> interface;

        void clearPorts() {
            interface->inPorts.clear();
            interface->outPorts.clear();
        }
     public:
        explicit Component(std::string id) : interface(std::make_shared<ComponentInterface>(ComponentInterface(std::move(id)))) {};
        virtual ~Component() = default;

        const std::string& getId() {
            return interface->id;
        }

        [[nodiscard]] std::shared_ptr<ComponentInterface> getParent() const {
            return interface->parent.lock();
        }

        void setParent(const std::shared_ptr<ComponentInterface>& newParent) {
            interface->parent = newParent;
        }

        std::shared_ptr<PortInterface> getInPort(const std::string& id) const {
            return interface->inPorts.getPort(id);
        }

        template <typename T>
        std::shared_ptr<Port<T>> getInPort(const std::string& id) const {
            return interface->inPorts.getPort(id);
        }

        std::shared_ptr<PortInterface> getOutPort(const std::string& id) const {
            return interface->outPorts.getPort(id);
        }

        template <typename T>
        std::shared_ptr<Port<T>> getOutPort(const std::string& id) const {
            return interface->outPorts.getPort(id);
        }

        void addInPort(const std::shared_ptr<PortInterface>& port) {
            interface->inPorts.addPort(port);
            port->setParent(interface);
        }

        template <typename T>
        [[maybe_unused]] void addInPort(const std::string id) {
            addInPort(std::make_shared<Port<T>>(id));
        }

        void addOutPort(const std::shared_ptr<PortInterface>& port) {
            interface->outPorts.addPort(port);
            port->setParent(interface);
        }

        template <typename T>
        [[maybe_unused]] void addOutPort(const std::string id) {
            addOutPort(std::make_shared<Port<T>>(id));
        }

        bool inEmpty() const {
            return interface->inPorts.empty();
        }

        bool outEmpty() const {
            return interface->outPorts.empty();
        }
    };
}

#endif //_CADMIUM_CORE_MODELING_COMPONENT_HPP_
