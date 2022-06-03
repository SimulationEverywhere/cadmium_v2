#ifndef CADMIUM_EXAMPLE_CELLDEVS_SYMM_SIR_CELLS_SIR_HPP_
#define CADMIUM_EXAMPLE_CELLDEVS_SYMM_SIR_CELLS_SIR_HPP_

#include <cmath>
#include <nlohmann/json.hpp>
#include <cadmium/celldevs/grid/cell.hpp>
#include <cadmium/celldevs/grid/config.hpp>
#include "../state.hpp"

 class SIRCell : public cadmium::celldevs::GridCell<SIR, double> {
	 double rec;   // recovery factor
	 double susc;  // susceptibility factor
	 double vir;   // virulence factor
 public:
	 SIRCell(const std::vector<int>& id, const std::shared_ptr<const cadmium::celldevs::GridCellConfig<SIR, double>>& config):
		 cadmium::celldevs::GridCell<SIR, double>(id, config) {
		 config->rawCellConfig.at("rec").get_to(rec);
		 config->rawCellConfig.at("susc").get_to(susc);
		 config->rawCellConfig.at("vir").get_to(vir);
	 }

	[[nodiscard]] SIR localComputation(SIR state, const std::unordered_map<std::vector<int>, cadmium::celldevs::NeighborData<SIR, double>>& neighborhood, const cadmium::PortSet& x) const override {
		auto newI = newInfections(state, neighborhood);
		auto newR = newRecoveries(state);

		// We round the outcome to three decimals:
		state.r = std::round((state.r + newR) * 1000) / 1000;
		state.i = std::round((state.i + newI - newR) * 1000) / 1000;
		state.s = 1 - state.i - state.r;
		return state;
	}

	 [[nodiscard]] double outputDelay(const SIR& state) const override {
		 return 1.;
	 }

	[[nodiscard]] double newInfections(const SIR& state, const std::unordered_map<std::vector<int>, cadmium::celldevs::NeighborData<SIR, double>>& neighborhood) const {
		double aux = 0;
		for(const auto& [neighborId, neighborData]: neighborhood) {
			auto s = neighborData.state;
			auto v = neighborData.vicinity;
			aux += s->i * (double) s->p * v;
		}
		return state.s * susc * std::min(1., vir * aux / state.p);
	}

	[[nodiscard]] double newRecoveries(const SIR& state) const {
		return state.i * rec;
	}
};

#endif //CADMIUM_EXAMPLE_CELLDEVS_SYMM_SIR_CELLS_SIR_HPP_
