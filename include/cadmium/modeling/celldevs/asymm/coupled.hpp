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

#ifndef CADMIUM_MODELING_CELLDEVS_ASYMM_COUPLED_HPP_
#define CADMIUM_MODELING_CELLDEVS_ASYMM_COUPLED_HPP_

#include <string>
#include <memory>
#include <nlohmann/json.hpp>
#include "cell.hpp"
#include "../core/coupled.hpp"
#include "../../../exception.hpp"

namespace cadmium::celldevs {

	template <typename S, typename V>
	using asymmCellFactory = std::shared_ptr<AsymmCell<S, V>>(*)(const std::string& cellId, const std::shared_ptr<const AsymmCellConfig<S, V>>& cellConfig);

	/**
	 * @brief Coupled asymmetric Cell-DEVS model.
	 * @tparam S the type used for representing a cell state.
	 * @tparam V the type used for representing a neighboring cell's vicinities.
	 */
	template <typename S, typename V>
	class AsymmCellDEVSCoupled: public CellDEVSCoupled<std::string, S, V> {
	 private:
		asymmCellFactory<S, V> factory;  //!< Pointer to asymmetric cell factory function
	 public:
		/**
		 * Constructor function.
		 * @param id ID of the coupled asymmetric Cell-DEVS model.
		 * @param factory pointer to asymmetric cell factory function.
		 * @param configFilePath path to the scenario configuration file.
		 */
		AsymmCellDEVSCoupled(const std::string& id, asymmCellFactory<S, V> factory, const std::string& configFilePath):
		  CellDEVSCoupled<std::string, S, V>(id, configFilePath), factory(factory) {}

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
			this->addComponent(factory(cellId, config));
		}
	};
} // namespace cadmium::celldevs

#endif // CADMIUM_MODELING_CELLDEVS_ASYMM_COUPLED_HPP_
