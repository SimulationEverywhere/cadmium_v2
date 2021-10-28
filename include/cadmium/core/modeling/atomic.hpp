#ifndef _CADMIUM_CORE_MODELING_ATOMIC_HPP_
#define _CADMIUM_CORE_MODELING_ATOMIC_HPP_

#include <memory>
#include <utility>
#include <vector>
#include "component.hpp"
#include "port.hpp"

// TODO change time type

namespace cadmium {

    class AbstractAtomic: public Component {
     public:
        explicit AbstractAtomic(std::string id) : Component(std::move(id)) {}
        virtual void internalTransition() = 0;
        virtual void externalTransition(double e) = 0;
        virtual void confluentTransition(double e) = 0;
        virtual void output() = 0;
        [[nodiscard]] virtual double timeAdvance() const = 0;  // TODO change time type
    };

    template <typename S>
    class Atomic: public AbstractAtomic {
     protected:
        S state;
     public:
        explicit Atomic(std::string id, S initialState) : state(initialState), AbstractAtomic(std::move(id)) {};

        virtual void internalTransition(S& s) const = 0;
        virtual void externalTransition(S& s, double e, const PortSet& x) const = 0;
        virtual void output(const S& s, const PortSet& y) const = 0;
        virtual double timeAdvance(const S& s) const = 0;
        virtual void confluentTransition(S& s, double e, const PortSet& x) const {
            this->internalTransition(s);
            this->externalTransition(s, 0., x);
        }

        void internalTransition() override {
            this->internalTransition(state);
        }

        void externalTransition(double e) override {
            this->externalTransition(state, e, interface->inPorts);
        }

        void confluentTransition(double e) override {
            this->confluentTransition(state, e, interface->inPorts);
        }

        void output() override {
            this->output(state, interface->outPorts);
        }

        [[nodiscard]] double timeAdvance() const override {
            return this->timeAdvance(state);
        }
    };
}

#endif //_CADMIUM_CORE_MODELING_ATOMIC_HPP_
