#ifndef CADMIUM_EXAMPLE_DEVSTONE_COUPLED_HPP_
#define CADMIUM_EXAMPLE_DEVSTONE_COUPLED_HPP_

#include <cadmium/core/modeling/coupled.hpp>
#include <cadmium/core/simulation/root_coordinator.hpp>
#include <memory>

namespace cadmium::example::devstone {
	//! Base class for DEVStone coupled models and DEVStone model factory function.
	class DEVStoneCoupled : public Coupled {
	 protected:
		//! pointer to child DEVStoneCoupled model. By default, it is set to nullptr (i.e., no child model).
		std::shared_ptr<DEVStoneCoupled> childCoupled;
	 public:
		/**
		 * DEVStone coupled model constructor function.
		 * @param width width of the DEVStone coupled model.
		 * @param depth depth of the DEVStone coupled model.
		 * @param intDelay internal delay for the DEVStone atomic models.
		 * @param extDelay external delay for the DEVStone atomic models.
		 */
		DEVStoneCoupled(int width, int depth, int intDelay, int extDelay);

		/**
		 * Static method to create a new DEVStone model with the desired configuration.
		 * @param type DEVStone model type. It must be "LI", "HI", "HO", or "HOmod".
		 * @param width width of the DEVStone coupled model.
		 * @param depth depth of the DEVStone coupled model.
		 * @param intDelay internal delay for the DEVStone atomic models.
		 * @param extDelay external delay for the DEVStone atomic models.
		 * @return pointer to newly created DEVStone coupled model.
		 */
		static std::shared_ptr<DEVStoneCoupled> newDEVStoneCoupled(const std::string& type, int width, int depth, int intDelay, int extDelay);

		//! @return the total number of DEVStone atomic models.
		[[nodiscard]] int nAtomics() const;

		//! @return the total number of internal couplings in the DEVStone coupled model.
		[[nodiscard]] int nICs() const;

		//! @return the total number of external internal couplings in the DEVStone coupled model.
		[[nodiscard]] int nEICs() const;

		//! @return the total number of external output couplings in the DEVStone coupled model.
		[[nodiscard]] int nEOCs() const;

		//! @return the total number of internal transitions triggered by all the DEVStone atomic models.
		[[nodiscard]] int nInternals() const;

        //! @return the total number of external transitions triggered by all the DEVStone atomic models.
        [[nodiscard]] int nExternals() const;

        //! @return the total number of events received by all the DEVStone atomic models.
        [[nodiscard]] int nEvents() const;
	};
}  //namespace cadmium::example::devstone

#endif //CADMIUM_EXAMPLE_DEVSTONE_COUPLED_HPP_
