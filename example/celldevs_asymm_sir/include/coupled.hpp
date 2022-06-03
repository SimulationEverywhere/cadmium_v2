#ifndef CADMIUM_EXAMPLE_CELLDEVS_ASYMM_SIR_COUPLED_HPP_
#define CADMIUM_EXAMPLE_CELLDEVS_ASYMM_SIR_COUPLED_HPP_

#include <cadmium/celldevs/asymm/coupled.hpp>
#include "cells/sir.hpp"
#include "state.hpp"

class SIRCoupled: public cadmium::celldevs::AsymmCellDEVSCoupled<SIR, double> {
 public:
	SIRCoupled(const std::string& id, const std::string& configFilePath): cadmium::celldevs::AsymmCellDEVSCoupled<SIR, double>(id, configFilePath) {}

	void addCell(const std::string& cellId, const std::shared_ptr<const cadmium::celldevs::AsymmCellConfig<SIR, double>>& cellConfig) override {
		auto cellModel = cellConfig->cellModel;
		if (cellModel == "default" || cellModel == "SIR") {
			addComponent(SIRCell(cellId, cellConfig));
		} else {
			throw std::bad_typeid();
		}
	}
};

#endif //CADMIUM_EXAMPLE_CELLDEVS_ASYMM_SIR_COUPLED_HPP_
