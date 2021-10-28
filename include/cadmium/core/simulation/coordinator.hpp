#ifndef _CADMIUM_SIMULATION_COORDINATOR_HPP_
#define _CADMIUM_SIMULATION_COORDINATOR_HPP_

#include <cadmium/core/modeling/atomic.hpp>
#include <cadmium/core/modeling/coupled.hpp>
#include <memory>
#include <utility>
#include <vector>
#include "abstract_simulator.hpp"
#include "simulator.hpp"

namespace cadmium {
    class Coordinator: public AbstractSimulator {
     private:
        std::shared_ptr<Coupled> model;
        std::vector<std::shared_ptr<AbstractSimulator>> simulators;
     public:
		Coordinator() : AbstractSimulator(0), model() {}
		Coordinator(double time) : AbstractSimulator(time), model() {}
        Coordinator(std::shared_ptr<Coupled> model, double time): AbstractSimulator(time), model(std::move(model)) {
			if (this->model == nullptr) {
				throw std::bad_exception();  // TODO custom exceptions
			}
			for (auto& component: this->model->components) {
				std::shared_ptr<AbstractSimulator> simulator;
				auto coupled = std::dynamic_pointer_cast<Coupled>(component);
				if (coupled != nullptr) {
					simulator = std::make_shared<Coordinator>(coupled, time);
				} else {
					auto atomic = std::dynamic_pointer_cast<AbstractAtomic>(component);
					if (atomic == nullptr) {
						throw std::bad_exception();  // TODO custom exceptions
					}
					simulator = std::make_shared<Simulator>(atomic, time);
				}
				simulators.push_back(simulator);
				timeNext = std::min(timeNext, simulator->timeNext);
			}
		}

        std::shared_ptr<Component> getComponent() override {
            return model;
        }

        void collection(double time) override {
            if (time >= timeNext) {
                std::for_each(simulators.begin(), simulators.end(), [time](auto& s) { s->collection(time); });
                std::for_each(model->IC.begin(), model->IC.end(), [](auto& s) {std::get<1>(s)->propagate(std::get<0>(s)); });
                std::for_each(model->EOC.begin(), model->EOC.end(), [](auto& s) {std::get<1>(s)->propagate(std::get<0>(s)); });
            }
        }

        void transition(double time) override {
            std::for_each(model->EIC.begin(), model->EIC.end(), [](auto& s) {std::get<1>(s)->propagate(std::get<0>(s)); });
            timeNext = std::numeric_limits<double>::infinity();
            for (auto& simulator: simulators) {
                simulator->transition(time);
                timeNext = std::min(timeNext, simulator->timeNext);
            }
        }

        void clear() override {
            std::for_each(simulators.begin(), simulators.end(), [](auto& s) { s->clear(); });
            AbstractSimulator::clear();
        }

        void simulate(long nIterations) {
            while (nIterations-- > 0 && timeNext < std::numeric_limits<double>::infinity()) {
                timeLast = timeNext;
                collection(timeLast);
                transition(timeLast);
                clear();
            }
        }

        void simulate(double timeInterval) {
            double timeFinal = timeLast + timeInterval;
            while(timeNext < timeFinal) {
                timeLast = timeNext;
                collection(timeLast);
                transition(timeLast);
                clear();
            }
            timeLast = timeFinal;
        }
    };
}

#endif //_CADMIUM_SIMULATION_COORDINATOR_HPP_
