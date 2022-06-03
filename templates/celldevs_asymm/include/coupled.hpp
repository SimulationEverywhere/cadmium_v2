#ifndef CADMIUM_EXAMPLE_CELLDEVS_ASYMM_SIR_COUPLED_HPP_
#define CADMIUM_EXAMPLE_CELLDEVS_ASYMM_SIR_COUPLED_HPP_

#include "cadmium/celldevs/asymm/coupled.hpp"
#include "cells/cell.hpp"
#include "state.hpp"
#include "vicinity.hpp"

class CoupledModel: public cadmium::celldevs::AsymmCellDEVSCoupled<State, Vicinity> {
 public:
	CoupledModel(const std::string& id, const std::string& configFilePath): cadmium::celldevs::AsymmCellDEVSCoupled<State, Vicinity>(id, configFilePath) {}

	void addCell(const std::string& cellId, const std::shared_ptr<const cadmium::celldevs::AsymmCellConfig<State, Vicinity>>& cellConfig) override {
		auto cellModel = cellConfig->cellModel;
		if (cellModel == "default") {
			addComponent(CellTemplate(cellId, cellConfig));
		} else {
			throw std::bad_typeid();
		}
	}
};

#endif //CADMIUM_EXAMPLE_CELLDEVS_ASYMM_SIR_COUPLED_HPP_
