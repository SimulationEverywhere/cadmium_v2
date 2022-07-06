#ifndef CADMIUM_EXAMPLE_CELLDEVS_ASYMM_COUPLED_HPP_
#define CADMIUM_EXAMPLE_CELLDEVS_ASYMM_COUPLED_HPP_

#include <cadmium/celldevs/asymm/coupled.hpp>
#include "sir_cell.hpp"

namespace cadmium::celldevs::example::sir {
	class AsymmSIRCoupled : public AsymmCellDEVSCoupled<SIRState, double> {
	 public:
		AsymmSIRCoupled(const std::string& id, const std::string& configFilePath) : AsymmCellDEVSCoupled<SIRState, double>(id, configFilePath) {}

		void addCell(const std::string& cellId, const std::shared_ptr<const AsymmCellConfig<SIRState, double>>& cellConfig) override {
			auto cellModel = cellConfig->cellModel;
			if (cellModel == "default" || cellModel == "SIR") {
				addComponent(std::make_shared<AsymmSIRCell>(cellId, cellConfig));
			}
			else {
				throw std::bad_typeid();
			}
		}
	};
}  //namespace cadmium::celldevs::example::sir

#endif //CADMIUM_EXAMPLE_CELLDEVS_ASYMM_COUPLED_HPP_
