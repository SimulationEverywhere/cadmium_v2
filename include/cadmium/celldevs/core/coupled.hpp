/**
 * Abstract Cell-DEVS coupled model.
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

#ifndef _CADMIUM_CELLDEVS_CORE_COUPLED_HPP_
#define _CADMIUM_CELLDEVS_CORE_COUPLED_HPP_

#include <fstream>
#include <nlohmann/json.hpp>
#include <sstream>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>
#include "../../core/modeling/coupled.hpp"
#include "cell.hpp"
#include "config.hpp"

namespace cadmium::celldevs {
	template<typename C, typename S, typename V>
	class CellDEVSCoupled : public Coupled {
	 protected:
		nlohmann::json rawConfig;                                                             /// JSON configuration file.
		std::unordered_map<std::string, std::shared_ptr<CellConfig<C, S, V>>> cellConfigs;    /// unordered map containing all the different cell configurations
	 public:
		CellDEVSCoupled(const std::string& id, const std::string& configFilePath): Coupled(id), rawConfig(), cellConfigs() {
			std::ifstream i(configFilePath);
			i >> rawConfig;
		}

		/// It adds all the cells according to the provided JSON configuration file.
		virtual void addCells() = 0;

		/// It builds the Cell-DEVS model completely
		void buildModel() {
			this->addCells();
			addCouplings();
		}

		/// It adds all the couplings required in the scenario according to the configuration file.
		void addCouplings() {
			for (const auto& cell: Coupled::components) {
				auto cellModel = std::dynamic_pointer_cast<Cell<C, S, V>>(cell);
				if (cellModel == nullptr) {
					throw std::bad_exception();  // TODO custom exception: unable to treat component as a cell
				}
				for (const auto& neighbor: cellModel->getNeighborhood()) {
					addInternalCoupling(cellId(neighbor.first), "neighborhoodOutput",
						cellModel->getId(), "neighborhoodInput");
				}
				auto cellConfig = cellModel->getCellConfig();
				for (const auto& eic: cellConfig->EIC) {
					addExternalInputCoupling(eic.first, cellModel->getId(), eic.second);
				}
				for (const auto& eoc: cellConfig->EOC) {
					addExternalOutputCoupling(cellModel->getId(), eoc.first, eoc.second);
				}
			}
		}

		/**
		 * Returns a string representation of a cell.
		 * @param id ID of a cell.
		 * @return string representation of the cell ID.
		 */
		static std::string cellId(const C& id) {
			std::stringstream ss;
			ss << id;
			return ss.str();
		}
	};
}

#endif //_CADMIUM_CELLDEVS_CORE_COUPLED_HPP_
