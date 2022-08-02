#ifndef CADMIUM_EXAMPLE_CELLDEVS_BENCHMARK_CELL_HPP_
#define CADMIUM_EXAMPLE_CELLDEVS_BENCHMARK_CELL_HPP_

#include <cmath>
#include <nlohmann/json.hpp>
#include <cadmium/celldevs/grid/cell.hpp>
#include <cadmium/celldevs/grid/config.hpp>
#include "state.hpp"

namespace cadmium::celldevs::example::benchmark {
	//! Synthetic benchmark cell.
	class GridBenchmarkCell : public GridCell<benchmarkState, double> {
		double digit;
	 public:
		GridBenchmarkCell(const std::vector<int>& id, const std::shared_ptr<const GridCellConfig<benchmarkState, double>>& config):
		  GridCell<benchmarkState, double>(id, config), digit() {}

		[[nodiscard]] benchmarkState localComputation(benchmarkState state,
		  const std::unordered_map<std::vector<int>, NeighborData<benchmarkState, double>>& neighborhood) const override {
			if(state.digit == 0){
				state.digit = 1;
			} else {
				state.digit = 0;
			}
			return state;
		}

		[[nodiscard]] double outputDelay(const benchmarkState& state) const override {
			return 1.;
		}
	};
}  //namespace admium::celldevs::example::sir

#endif //CADMIUM_EXAMPLE_CELLDEVS_BENCHMARK_CELL_HPP_
