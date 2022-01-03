//
// Created by Román Cárdenas Rodríguez on 3/1/22.
//

#ifndef CADMIUM_EXAMPLE_CELLDEVS_GRID_SIRD_COUPLED_HPP_
#define CADMIUM_EXAMPLE_CELLDEVS_GRID_SIRD_COUPLED_HPP_

#include <cadmium/celldevs/grid/coupled.hpp>
#include "cells/sird.hpp"
#include "state.hpp"
#include "vicinity.hpp"

class SIRDSCoupled: public cadmium::celldevs::GridCellDEVSCoupled<sird, mc> {
 public:
	SIRDSCoupled(const std::string& id, const std::string& configFilePath): cadmium::celldevs::GridCellDEVSCoupled<sird, mc>(id, configFilePath) {
		addOutPort("output");
	}

	void addCell(const std::vector<int>& cellId, const std::shared_ptr<cadmium::celldevs::GridCellConfig<sird, mc>>& cellConfig) override {
		auto cellModel = cellConfig->cellModel;
		if (cellModel == "default" || cellModel == "sirds") {
			addComponent(SIRDSCell(cellId, cellConfig));
		} else {
			throw std::bad_typeid();
		}
	}
};

#endif //CADMIUM_EXAMPLE_CELLDEVS_GRID_SIRD_COUPLED_HPP_
