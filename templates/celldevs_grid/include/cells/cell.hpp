#ifndef CADMIUM_TEMPLATE_CELLDEVS_GRID_CELL_HPP_
#define CADMIUM_TEMPLATE_CELLDEVS_GRID_CELL_HPP_

#include "cadmium/celldevs/grid/cell.hpp"
#include "../state.hpp"
#include "../vicinity.hpp"

/// Extra configuration for cell model (remove this if you don't need it)
struct CellConfig {
	CellConfig() {}
};

/**
 * We need to implement the from_json method for the desired cell configuration struct.
 * Otherwise, Cadmium will not be able to understand the JSON configuration file.
 * You can remove this if you don't need extra configuration parameters.
 * @param j Chunk of JSON file that represents a cell configuration.
 * @param c cell configuration struct to be filled with the configuration shown in the JSON file.
 */
void from_json(const nlohmann::json& j, CellConfig& c) {}

 class CellTemplate : public cadmium::celldevs::GridCell<State, Vicinity> {
	 CellConfig cellConfig;   /// additional cell configuration (remove it if you don't need it)
 public:
	 CellTemplate(const std::vector<int>& id, const std::shared_ptr<const cadmium::celldevs::GridCellConfig<State, Vicinity>>& config):
	   cadmium::celldevs::GridCell<State, Vicinity>(id, config), cellConfig() {
		 //  addInPort<int>("inPort1");  // You can add input ports if needed
		 config->rawCellConfig.get_to(cellConfig);  // remove this if you don't need additional cell configuration
	 }

	 /**
	  * Implement the local computation function of the cell.
	  * @param state previous cell state. You can modify this variable and return it.
	  * @param neighborhood information about the cell neighborhood (neighboring cells, vicinity factor, states...).
	  * @param x input port set with any potential input messages.
	  * @return new cell state
	  */
	[[nodiscard]] State localComputation(State state, const std::unordered_map<std::vector<int>, cadmium::celldevs::NeighborData<State, Vicinity>>& neighborhood, const cadmium::PortSet& x) const override {
		 // This loop iterates over all the neighboring cells. neighborId is the neighboring cell ID
		for(const auto& [neighborId, neighborData]: neighborhood) {
			 auto neighborState = neighborData.state;  // neighbor cell state
			 auto neighborVicinity = neighborData.vicinity;  // vicinity factor of neighboring cell
		 }
		return state;
	}

	/**
	 * Implement the output delay function of the cell.
	 * @param state new cell state.
	 * @return time to wait before notifying influenced cells about this new state.
	 */
	 [[nodiscard]] double outputDelay(const State& state) const override {
		 return 1.;
	 }
};

#endif //CADMIUM_TEMPLATE_CELLDEVS_GRID_CELL_HPP_
