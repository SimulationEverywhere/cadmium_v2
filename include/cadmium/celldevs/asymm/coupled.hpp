/**
 * Abstract implementation of a coupled asymmetric Cell-DEVS model.
 * Copyright (C) 2022  Román Cárdenas Rodríguez
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

#ifndef CADMIUM_CELLDEVS_ASYMM_COUPLED_HPP_
#define CADMIUM_CELLDEVS_ASYMM_COUPLED_HPP_

#include <string>
#include <memory>
#include <nlohmann/json.hpp>
#include "config.hpp"
#include "../core/coupled.hpp"
#include "../../core/exception.hpp"

namespace cadmium::celldevs {
	/**
	 * @brief Abstract implementation of a coupled asymmetric Cell-DEVS model.
	 * @tparam S the type used for representing a cell state.
	 * @tparam V the type used for representing a neighboring cell's vicinities.
	 */
	template <typename S, typename V>
	class AsymmCellDEVSCoupled: public CellDEVSCoupled<std::string, S, V> {  // TODO meter puntero a función aquí.
	 public:
		AsymmCellDEVSCoupled(const std::string& id, const std::string& configFilePath): CellDEVSCoupled<std::string, S, V>(id, configFilePath) {}

		/**
		 * Adds a new cell to the coupled model. Modelers must implement it to use their custom cell models.
		 * @param cellId ID of the new cell.
		 * @param cellConfig all the configuration parameters related to the cell to be added.
		 */
		virtual void addCell(const std::string& cellId, const std::shared_ptr<const AsymmCellConfig<S, V>>& cellConfig) = 0;

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
				throw CadmiumModelException("Invalid cell configuration data type");
			}
			auto cellId = config->configId;
			this->addCell(cellId, config);
		}
	};
} // namespace cadmium::celldevs

#endif // CADMIUM_CELLDEVS_ASYMM_COUPLED_HPP_
