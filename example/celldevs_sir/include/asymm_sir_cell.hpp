#ifndef CADMIUM_EXAMPLE_CELLDEVS_ASYMM_CELL_HPP_
#define CADMIUM_EXAMPLE_CELLDEVS_ASYMM_CELL_HPP_

#include <cmath>
#include <nlohmann/json.hpp>
#include <cadmium/modeling/celldevs/asymm/cell.hpp>
#include <cadmium/modeling/celldevs/asymm/config.hpp>
#include "state.hpp"

 namespace cadmium::celldevs::example::sir {
	 //! Asymmetric Susceptible-Infected-Recovered cell.
	 class AsymmSIRCell : public AsymmCell<SIRState, double> {
		 double rec;   //!< Recovery factor.
		 double susc;  //!< Susceptibility factor.
		 double vir;   //!< Virulence factor.
	  public:
		 AsymmSIRCell(const std::string& id, const std::shared_ptr<const AsymmCellConfig<SIRState, double>>& config):
		   AsymmCell<SIRState, double>(id, config) {
			 config->rawCellConfig.at("rec").get_to(rec);
			 config->rawCellConfig.at("susc").get_to(susc);
			 config->rawCellConfig.at("vir").get_to(vir);
		 }

		 [[nodiscard]] SIRState localComputation(SIRState state,
			 const std::unordered_map<std::string, NeighborData<SIRState, double>>& neighborhood) const override {
			 auto newI = newInfections(state, neighborhood);
			 auto newR = newRecoveries(state);

			 // We round the outcome to three decimals:
			 state.r = std::round((state.r + newR) * 1000) / 1000;
			 state.i = std::round((state.i + newI - newR) * 1000) / 1000;
			 state.s = 1 - state.i - state.r;
			 return state;
		 }

		 //! Output delay is always one day.
		 [[nodiscard]] double outputDelay(const SIRState& state) const override {
			 return 1.;
		 }

		 //! It computes the ratio of new infections in the cell.
		 [[nodiscard]] double newInfections(const SIRState& state,
			 const std::unordered_map<std::string, NeighborData<SIRState, double>>& neighborhood) const {
			 double aux = 0;
			 for (const auto& [neighborId, neighborData]: neighborhood) {
				 auto s = neighborData.state;
				 auto v = neighborData.vicinity;
				 aux += s->i * (double)s->p * v;
			 }
			 return state.s * susc * std::min(1., vir * aux / state.p);
		 }

		 //! It computes the ratio of new recoveries in the cell.
		 [[nodiscard]] double newRecoveries(const SIRState& state) const {
			 return state.i * rec;
		 }
	 };
 }  //namespace cadmium::celldevs::example::sir
#endif //CADMIUM_EXAMPLE_CELLDEVS_ASYMM_CELL_HPP_
