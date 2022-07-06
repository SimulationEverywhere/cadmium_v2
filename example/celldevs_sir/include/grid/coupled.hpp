#ifndef CADMIUM_EXAMPLE_CELLDEVS_GRID_COUPLED_HPP_
#define CADMIUM_EXAMPLE_CELLDEVS_GRID_COUPLED_HPP_

#include <cadmium/celldevs/grid/coupled.hpp>
#include "sir_cell.hpp"

namespace cadmium::celldevs::example::sir {
	class GridSIRCoupled : public GridCellDEVSCoupled<SIRState, double> {
	 public:
		GridSIRCoupled(const std::string& id, const std::string& configFilePath) : GridCellDEVSCoupled<SIRState, double>(id, configFilePath) {}

		void addCell(const std::vector<int>& cellId, const std::shared_ptr<const GridCellConfig<SIRState, double>>& cellConfig) override {
			auto cellModel = cellConfig->cellModel;
			if (cellModel == "default" || cellModel == "SIR") {
				addComponent<GridSIRCell>(cellId, cellConfig);
			}
			else {
				throw std::bad_typeid();
			}
		}
	};
}  //namespace cadmium::celldevs::example::sir

#endif //CADMIUM_EXAMPLE_CELLDEVS_SYMM_SIR_COUPLED_HPP_
