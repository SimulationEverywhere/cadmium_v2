/**
 * Implementation of the main building blocks for the Asymmetric Cell-DEVS formalism.
 * Copyright (C) 2021  Román Cárdenas Rodríguez
 * ARSLab - Carleton University
 * GreenLSI - Polytechnic University of Madrid
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef _CADMIUM_CELLDEVS_ASYMM_HPP_
#define _CADMIUM_CELLDEVS_ASYMM_HPP_

#include <string>
#include "core/cell.hpp"
#include "core/config.hpp"
#include "core/coupled.hpp"

namespace cadmium::celldevs {
	template <typename S, typename V>
	struct AsymmCellConfig: public CellConfig<std::string, S, V> {
		AsymmCellConfig(const std::string& configId, const nlohmann::json& configParams): CellConfig<std::string, S, V>(configId, configParams) {}

		std::unordered_map<std::string, NeighborData<S, V>> buildNeighborhood(const std::string& cellId) const override {
			return CellConfig<std::string, S, V>::rawNeighborhood.template get<std::unordered_map<std::string, NeighborData<S, V>>>();
		}
	};

	template <typename S, typename V>
	class AsymmCell: public Cell<std::string, S, V> {
	  public:
		 AsymmCell(const std::string& id, const std::shared_ptr<AsymmCellConfig<S, V>>& config): Cell<std::string, S, V>(id, config) {}
 	};

	template <typename S, typename V>
	class AsymmCellDEVSCoupled: public CellDEVSCoupled<std::string, S, V> {
		using CellDEVSCoupled<std::string, S, V>::rawConfig;
		using CellDEVSCoupled<std::string, S, V>::cellConfigs;
	  public:
		AsymmCellDEVSCoupled(const std::string& id, const std::string& configFilePath): CellDEVSCoupled<std::string, S, V>(id, configFilePath) {}

		/**
		 * Adds a new cell to the coupled model. Modelers must implement it to use their custom cell models.
		 * @param cellId ID of the new cell.
		 * @param cellConfig all the configuration parameters related to the cell to be added.
		 */
		virtual void addCell(const std::string& cellId, const std::shared_ptr<AsymmCellConfig<S, V>>& cellConfig) = 0;

		/**
	     * Generates a cell configuration struct from a JSON object.
		 * @param configId ID of the configuration to be loaded.
		 * @param cellConfig cell configuration parameters (JSON object).
		 * @return pointer to the cell configuration created.
		 */
		std::shared_ptr<CellConfig<std::string, S, V>> loadCellConfig(const std::string& configId, const nlohmann::json& cellConfig) const override {
			return std::make_shared<AsymmCellConfig<S, V>>(configId, cellConfig);
		}

		 /**
		  * From a cell configuration config, it adds all the required cells to the model.
		  * @param cellConfig target cell configuration struct.
		  */
		 void addCells(const std::shared_ptr<CellConfig<std::string, S, V>>& cellConfig) override {
			 auto config = std::dynamic_pointer_cast<AsymmCellConfig<S, V>>(cellConfig);
			 if (config == nullptr) {
				 throw std::bad_exception();  // TODO custom exception: unable to do the appropriate cast
			 }
			 auto cellId = config->configId;
			 this->addCell(cellId, config);
		 }
	 };
}

#endif //_CADMIUM_CELLDEVS_ASYMM_HPP_
