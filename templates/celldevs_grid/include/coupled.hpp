#ifndef CADMIUM_EXAMPLE_CELLDEVS_ASYMM_SIR_COUPLED_HPP_
#define CADMIUM_EXAMPLE_CELLDEVS_ASYMM_SIR_COUPLED_HPP_

#include "cadmium/celldevs/grid/coupled.hpp"
#include "cells/cell.hpp"
#include "state.hpp"
#include "vicinity.hpp"

class CoupledModel: public cadmium::celldevs::GridCellDEVSCoupled<State, Vicinity> {
 public:
	CoupledModel(const std::string& id, const std::string& configFilePath): cadmium::celldevs::GridCellDEVSCoupled<State, Vicinity>(id, configFilePath) {}

	void addCell(const std::vector<int>& cellId, const std::shared_ptr<const cadmium::celldevs::GridCellConfig<State, Vicinity>>& cellConfig) override {
		auto cellModel = cellConfig->cellModel;
		if (cellModel == "default") {
			addComponent(CellTemplate(cellId, cellConfig));
		} else {
			throw std::bad_typeid();
		}
	}
};

#endif //CADMIUM_EXAMPLE_CELLDEVS_ASYMM_SIR_COUPLED_HPP_
