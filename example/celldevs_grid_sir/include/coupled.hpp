#ifndef CADMIUM_EXAMPLE_CELLDEVS_SYMM_SIR_COUPLED_HPP_
#define CADMIUM_EXAMPLE_CELLDEVS_SYMM_SIR_COUPLED_HPP_

#include <cadmium/celldevs/grid/coupled.hpp>
#include "cells/sir.hpp"
#include "state.hpp"

class SIRCoupled: public cadmium::celldevs::GridCellDEVSCoupled<SIR, double> {
 public:
	SIRCoupled(const std::string& id, const std::string& configFilePath): cadmium::celldevs::GridCellDEVSCoupled<SIR, double>(id, configFilePath) {}

	void addCell(const std::vector<int>& cellId, const std::shared_ptr<const cadmium::celldevs::GridCellConfig<SIR, double>>& cellConfig) override {
		auto cellModel = cellConfig->cellModel;
		if (cellModel == "default" || cellModel == "SIR") {
			addComponent<SIRCell>(cellId, cellConfig);
		} else {
			throw std::bad_typeid();
		}
	}
};

#endif //CADMIUM_EXAMPLE_CELLDEVS_SYMM_SIR_COUPLED_HPP_
