/**
 * Abstract base class for grid Cell-DEVS coupled model.
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2022-present Román Cárdenas Rodríguez
 * ARSLab - Carleton University
 * GreenLSI - Polytechnic University of Madrid
 */

#ifndef CADMIUM_MODELING_CELLDEVS_GRID_COUPLED_HPP_
#define CADMIUM_MODELING_CELLDEVS_GRID_COUPLED_HPP_

#include <nlohmann/json.hpp>
#include <memory>
#include <vector>
#include "cell.hpp"
#include "scenario.hpp"
#include "utility.hpp"
#include "../core/coupled.hpp"
#include "../../../exception.hpp"

namespace cadmium::celldevs {

	template <typename S, typename V>
	using gridCellFactory = std::shared_ptr<GridCell<S, V>>(*)(const coordinates & cellId, const std::shared_ptr<const GridCellConfig<S, V>>& cellConfig);

	/**
	 * @brief Coupled grid Cell-DEVS model.
	 * @tparam S the type used for representing a cell state.
	 * @tparam V the type used for representing a neighboring cell's vicinities.
	 */
	template <typename S, typename V>
	class GridCellDEVSCoupled: public CellDEVSCoupled<coordinates, S, V> {
	 private:
		using CellDEVSCoupled<coordinates, S, V>::rawConfig;
		std::shared_ptr<GridScenario> scenario;  //!< Pointer to the scenario configuration.
		gridCellFactory<S, V> factory;           //!< Pointer to grid cell factory function.
	 public:
		/**
		 * Constructor function.
		 * @param id ID of the coupled grid Cell-DEVS model.
		 * @param factory pointer to grid cell factory function.
		 * @param configFilePath path to the scenario configuration file.
		 */
		GridCellDEVSCoupled(const std::string& id, gridCellFactory<S, V> factory, const std::string& configFilePath):
		  CellDEVSCoupled<coordinates, S, V>(id, configFilePath), factory(factory) {
			nlohmann::json rawScenario = rawConfig.at("scenario");
			auto shape = rawScenario.at("shape").get<coordinates>();
			auto origin = rawScenario.contains("origin")? rawScenario["origin"].get<coordinates>() : coordinates(shape.size(), 0);
			auto wrapped = rawScenario.contains("wrapped") && rawScenario["wrapped"].get<bool>();
			scenario = std::make_shared<GridScenario>(shape, origin, wrapped);
		}

		/**
	     * Generates a cell configuration struct from a JSON object.
		 * @param configId ID of the configuration to be loaded.
		 * @param cellConfig cell configuration parameters (JSON object).
		 * @return pointer to the cell configuration created.
		 */
		std::shared_ptr<CellConfig<coordinates, S, V>> loadCellConfig(const std::string& configId, const nlohmann::json& cellConfig) const override {
			return std::make_shared<GridCellConfig<S, V>>(configId, cellConfig, scenario);
		}

		/**
		 * From a cell configuration config, it adds all the required cells to the model.
		 * @param cellConfig target cell configuration struct.
		 */
		void addCells(const std::shared_ptr<CellConfig<coordinates, S, V>>& cellConfig) override {
			auto config = std::dynamic_pointer_cast<GridCellConfig<S, V>>(cellConfig);
			if (config == nullptr) {
				throw CadmiumModelException("invalid cell configuration data type");
			}
			for (const auto& cellId: config->cellMap) {
				this->addComponent(factory(cellId, config));
			}
		}

		/**
		 * After adding all the cells with special configuration, it adds the remaining default cells.
		 * @param defaultConfig default cell configuration struct.
		 */
		void addDefaultCells(const std::shared_ptr<CellConfig<coordinates, S, V>>& defaultConfig) override {
			auto config = std::dynamic_pointer_cast<GridCellConfig<S, V>>(defaultConfig);
			if (config == nullptr) {
				throw CadmiumModelException("invalid cell configuration data type");
			}
			for (const auto& cellId: *scenario) {
				try {
					auto _cell = Coupled::getComponent(CellDEVSCoupled<coordinates , S, V>::cellId(cellId));
				} catch (const CadmiumModelException& ex) {
					this->addComponent(factory(cellId, config));
				}
			}
		}
	};
}

#endif //CADMIUM_MODELING_CELLDEVS_GRID_COUPLED_HPP_
