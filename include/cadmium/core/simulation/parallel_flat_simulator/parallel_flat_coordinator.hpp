/**
 * <one line to give the program's name and a brief idea of what it does.>
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

#ifndef _CADMIUM_CORE_SIMULATION_PARALLEL_FLAT_COORDINATOR_HPP_
#define _CADMIUM_CORE_SIMULATION_PARALLEL_FLAT_COORDINATOR_HPP_

#include <memory>
#include <utility>
#include <vector>
#include "parallel_abs_simulator.hpp"
#include "parallel_simulator.hpp"
#include "../../modeling/atomic.hpp"
#include "../../modeling/coupled.hpp"
#include "port.hpp"

namespace cadmium {

    using coupling = std::tuple<const std::shared_ptr<PortInterface>, const std::shared_ptr<PortInterface>>;

    class ParallelFlatCoordinator : public ParallelAbstractSimulator {
     private:
        std::shared_ptr<Coupled> model;
        std::vector<std::shared_ptr<ParallelAbstractSimulator>> simulators;

		long setModelId(long next) override {
			modelId = next++;
			for (auto& simulator: simulators) {
				next = simulator->setModelId(next);
			}
			return next;
		}

		void start(double time) {
			timeLast = time;
			std::for_each(simulators.begin(), simulators.end(), [time](auto& s) { s->start(time); });
		}

		void stop(double time) {
			timeLast = time;
			std::for_each(simulators.begin(), simulators.end(), [time](auto& s) { s->stop(time); });
		}

        void execute_output_functions(double time) {
            if (time >= timeNext) {
                std::for_each(simulators.begin(), simulators.end(), [time](auto& s) { s->execute_output_function(time); });
            }
        }

		void route_messages(double time) {
            std::for_each(model->IC.begin(), model->IC.end(), [](auto& s) {std::get<1>(s)->propagate(std::get<0>(s)); });
            std::for_each(model->EOC.begin(), model->EOC.end(), [](auto& s) {std::get<1>(s)->propagate(std::get<0>(s)); });
            std::for_each(model->EIC.begin(), model->EIC.end(), [](auto& s) {std::get<1>(s)->propagate(std::get<0>(s)); });
        }

		double execute_state_transitions(double time) {
		    double timeNext = std::numeric_limits<double>::infinity();
			for (auto& simulator: simulators) {
				simulator->transition(time);
				timeNext = std::min(timeNext, simulator->timeNext);
			}
			return timeNext;
		}

		void clear() override {
			std::for_each(simulators.begin(), simulators.end(), [](auto& s) { s->clear(); });
			getComponent()->clearPorts();
		}

     public:
        ParallelFlatCoordinator(std::shared_ptr<Coupled> model, double time): AbstractSimulator(time), model(std::move(model)) {
            if (this->model == nullptr) {
				throw std::bad_exception();  // TODO custom exceptions
			}
			//for (auto& component: this->model->components) {
            //#pragma omp parallel for schedule(static)
            //for (size_t i=0; i<model; i++) {
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

/*
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
*/

/*
        	std::vector<std::shared_ptr<AbstractSimulator>> flat_subcoordinators;
            //list of all coordinators and simulators in the structure
            flat_subcoordinators = get_flat_subcoordinators(model);

            //get alll couplings as internal ones
            flat_internal_couplings = get_internal_couplings(flat_subcoordinators);

			//remove intermediate levels
			_internal_coupligns = remove_intermediate_levels(_internal_coupligns);
*/
		}
/*

		template <typename T>
		explicit Coordinator(T model) : Coordinator(std::make_shared<T>(std::move(model)), 0) {}

		std::vector<std::shared_ptr<AbstractSimulator>> get_flat_subcoordinators(std::shared_ptr<Coupled> model) {
		    std::vector<std::shared_ptr<AbstractSimulator>> _subcoordinators;
		    std::vector<std::shared_ptr<AbstractSimulator>> partial_subcoordinators;

		    for(auto& m : model->components) {
		        partial_subcoordinators = get_subcoordinators(m);
		        _subcoordinators.insert(_subcoordinators.end(), std::make_move_iterator(partial_subcoordinators.begin()), std::make_move_iterator(partial_subcoordinators.end()));
		    }

		    return _subcoordinators;
        }

		std::vector<std::shared_ptr<AbstractSimulator>> get_subcoordinators(std::shared_ptr<Coupled> model) {
		    partial_subcoordinators;
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
		    			    //timeNext = std::min(timeNext, simulator->timeNext);
		    }
        }


        subcoordinators_type<TIME> get_subcoordinators(std::shared_ptr<cadmium::dynamic::modeling::model> model) {
        	subcoordinators_type<TIME> partial_subcoordinators;

        	std::shared_ptr<cadmium::dynamic::modeling::coupled<TIME>> m_coupled = std::dynamic_pointer_cast<cadmium::dynamic::modeling::coupled<TIME>>(model);
        	std::shared_ptr<cadmium::dynamic::modeling::atomic_abstract<TIME>> m_atomic = std::dynamic_pointer_cast<cadmium::dynamic::modeling::atomic_abstract<TIME>>(model);

        	if (m_coupled == nullptr) {
        		if (m_atomic == nullptr) {
        			throw std::domain_error("Invalid submodel is neither coupled nor atomic");
        		}
        		std::shared_ptr<cadmium::dynamic::engine::engine<TIME>> simulator = std::make_shared<cadmium::dynamic::engine::simulator<TIME, LOGGER>>(m_atomic);
        		_subcoordinators.push_back(simulator);
        	} else {
        		if (m_atomic != nullptr) {
        			throw std::domain_error("Invalid submodel is defined as both coupled and atomic");
        		}
        		std::shared_ptr<cadmium::dynamic::engine::engine<TIME>> coordinator = std::make_shared<cadmium::dynamic::engine::coordinator<TIME, LOGGER>>(m_coupled);
        		_subcoordinators.push_back(coordinator);
        		for(auto& model : m_coupled->_models) {
        			partial_subcoordinators = get_subcoordinators(model);
        			_subcoordinators.insert(_subcoordinators.end(), std::make_move_iterator(partial_subcoordinators.begin()), std::make_move_iterator(partial_subcoordinators.end()));
        		}
        	}
        	engines_by_id.insert(std::make_pair(_subcoordinators.back()->get_model_id(), _subcoordinators.back()));
        	return _subcoordinators;
        }


        internal_couplings<TIME> get_internal_couplings(subcoordinators_type<TIME> flat_subcoordinators) {
        	internal_couplings<TIME> _partial_internal_couplings;
        	internal_couplings<TIME> _internal_couplings;
        	external_couplings<TIME> _external_output_couplings;
        	external_couplings<TIME> _external_input_couplings;
        	cadmium::dynamic::engine::internal_coupling<TIME> new_ic;

        	for(auto& element : flat_subcoordinators) {

        		std::shared_ptr<cadmium::dynamic::engine::coordinator<TIME,LOGGER>> m_coupled = std::dynamic_pointer_cast<cadmium::dynamic::engine::coordinator<TIME, LOGGER>>(element);

    			if(m_coupled != nullptr) {
    				_partial_internal_couplings  = m_coupled->get_internal_couplings();
					_internal_couplings.insert(_internal_couplings.end(), std::make_move_iterator(_partial_internal_couplings.begin()), std::make_move_iterator(_partial_internal_couplings.end()));

					_external_output_couplings = m_coupled->get_external_output_couplings();

					for(auto& eoc : _external_output_couplings){
						new_ic.first.first = eoc.first;
						new_ic.first.second = engines_by_id.at(m_coupled->get_model_id());
						new_ic.second = eoc.second;
						_internal_coupligns.push_back(new_ic);
					}

					_external_input_couplings = m_coupled->get_external_input_couplings();

					for(auto& eic : _external_input_couplings){
						new_ic.first.first= engines_by_id.at(m_coupled->get_model_id());
						new_ic.first.second = eic.first;
						new_ic.second = eic.second;
						_internal_coupligns.push_back(new_ic);
					}
    			}
        	}
        	return _internal_couplings;
        }

        internal_couplings<TIME> remove_intermediate_levels(internal_couplings<TIME> internal_couplings) {
        	//std::type_index from_type, from_port_type, to_type, to_port_type;

        	for(auto& from : internal_couplings) {

        		std::shared_ptr<cadmium::dynamic::engine::engine<TIME>> engine;
        		engine = from.first.first;
        		std::shared_ptr<cadmium::dynamic::engine::coordinator<TIME,LOGGER>> m_coupled = std::dynamic_pointer_cast<cadmium::dynamic::engine::coordinator<TIME, LOGGER>>(from.first.first);

        		if(m_coupled != nullptr){
        			for(auto& to : internal_couplings){
        				if(from.first.first->get_model_id().compare(to.first.second->get_model_id()) == 0){
        					//for(std::vector<std::shared_ptr<cadmium::dynamic::engine::link_abstract>>::iterator link_from = from.second.begin() ; link_from != from.second.end(); ++link_from){
        					for(auto link_from = from.second.begin() ; link_from != from.second.end(); ++link_from){
        						//for(std::vector<std::shared_ptr<cadmium::dynamic::engine::link_abtract>>::iterator link_to = from.second.begin() ; link_to != from.second.end(); ++link_to){
        						for(auto link_to = from.second.begin() ; link_to != from.second.end(); ++link_to){
        							if((*link_from)->from_port_type_index() == (*link_to)->to_port_type_index()){
        								//std::type_index from_port_type, to_port_type;
        								//std::type_index  from_port_type = (*link_from)->from_type_index();
        								//std::type_index to_port_type = (*link_to)->to_type_index();

        								//using from_port=typename current_IC::from_model_output_port;

        								//std::type_index from_port = (*link_from)->from_port_type_index() ;
        								//std::type_index to_port = (*link_from)->to_port_type_index() ;

        								//std::type_index from_port = (*link_from)->from_port_type_index();
        								//std::type_index to_port = (*link_to)->to_port_type_index();

        								//std::type_index from_port = (*link_from)->from_port_type_index() ;
        								//std::type_index to_port = (*link_from)->to_port_type_index() ;

        								//using from_message_type = typename PORT_FROM::message_type;
        								//using from_message_bag_type = typename cadmium::message_bag<PORT_FROM>;
        								//using to_message_type = typename PORT_TO::message_type;
        								//using to_message_bag_type = typename cadmium::message_bag<PORT_TO>;

        								//std::shared_ptr<cadmium::dynamic::engine::link_abtract> m_coupled = std::dynamic_pointer_cast<cadmium::dynamic::modeling::coupled<TIME>>(m);

        								//std::type_index from_port_ = (*link_from)->from_port();
        								//std::type_index to_port_ = (*link_to)->to_port();

        								//decltype ((*link_from)->get_from_port();)

        								//typename from_port = ((*link_from)->from_port()).name();
        								//typename to_port = ((*link_to)->from_port_type_index()).name();

        								//std::string from_port = ((*link_from)->from_port());
        								//std::string to_port = ((*link_to)->to_port());

        								const std::type_info* from_port = ((*link_from)->from_port());
        								const std::type_info* to_port = ((*link_from)->to_port());

        								//boost::typeindex::type_id<T>().pretty_name();

        								//std::type_index(typeid(fruit::apple));

        								//struct from : public in_port<from_port_type>{};
        								//struct to : public out_port<to_port_type>{};



        								//std::shared_ptr<cadmium::dynamic::engine::link_abstract> ic_link = cadmium::dynamic::translate::make_link<from_port_type,to_port_type>();
        								//std::shared_ptr<cadmium::dynamic::engine::link_abstract> ic_link = cadmium::dynamic::translate::make_link<double, double>();

        								//if(from_port_type == to_port_type){
        									//cadmium::dynamic::modeling::IC ic_link = cadmium::dynamic::translate::make_IC<from_port,to_port>("sdfsdf","sdfsdff");
        								//}

        								//std::shared_ptr<cadmium::dynamic::engine::link_abstract> new_link = cadmium::dynamic::translate::make_link<from_port, to_port>();

        								//std::shared_ptr<cadmium::dynamic::engine::link_abstract> new_link = cadmium::dynamic::translate::make_link<decltype(from_port), decltype(to_port)>();
        								//boost::typeindex::type_id<PORT_FROM>()

        								Do(from_port, to_port);


        							}
        						}
        					}
        				}
        			}
        		}
        	}
        	return internal_couplings;
        }
*/

		void start() {
			setModelId(0);
			if (logger != nullptr) {
				logger->start();
			}
			start(timeLast);
		}

		void stop() {
			stop(timeLast);
			if (logger != nullptr) {
				logger->stop();
			}
		}

		void setLogger(const std::shared_ptr<Logger>& log) override {
			logger = log;
			std::for_each(simulators.begin(), simulators.end(), [log](auto& s) { s->setLogger(log); });
		}

		std::shared_ptr<Component> getModel() override {
			return model;
		}

        std::vector<std::shared_ptr<AbstractSimulator>> get_subcoordinators() {
            return simulators;
        }

        std::vector<coupling> get_internal_couplings() {
		    return model->IC;
        }


    };
}

#endif //_CADMIUM_CORE_SIMULATION_FLAT_COORDINATOR_HPP_
