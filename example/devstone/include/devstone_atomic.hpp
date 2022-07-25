#ifndef CADMIUM_EXAMPLE_DEVSTONE_ATOMIC_HPP_
#define CADMIUM_EXAMPLE_DEVSTONE_ATOMIC_HPP_

#include <cadmium/core/modeling/atomic.hpp>
#include <iostream>

namespace cadmium::example::devstone {
	//! State of DEVStone atomic models.
	struct DEVStoneState {
		double sigma;                //!< time to wait before triggering the next internal transition function.
		unsigned long nTransitions;  //!< number of external transitions triggered so far.
		DEVStoneState();             //!< Constructor function sets sigma to infinity and nTransitions to 0.
	};

	/**
	 * Insertion operator for DEVStoneState objects. It only displays DEVStoneState::nTransitions.
	 * @param out output stream.
	 * @param s state to be represented in the output stream.
	 * @return output stream with the state already inserted.
	 */
	std::ostream &operator << (std::ostream &os, const DEVStoneState& x);

	//! DEVStone atomic DEVS model
	class DEVStoneAtomic: public Atomic<DEVStoneState> {
	 private:
		const int intDelay;   //!< Dhrystone execution time (in ms) when triggering the internal transition function.
		const int  extDelay;  //!< Dhrystone execution time (in ms) when triggering the external transition function.
	 public:
		std::shared_ptr<cadmium::Port<int>> in;   //!< Input Port.
		std::shared_ptr<cadmium::Port<int>> out;  //!< Output Port.

		/**
		 * Constructor function of the atomic DEVStone model.
		 * @param id ID of the atomic DEVStone model.
		 * @param intDelay internal delay.
		 * @param extDelay external delay.
		 */
		DEVStoneAtomic(const std::string& id, int intDelay, int extDelay);

		//! It returns the number of external transitions triggered so far by the atomic DEVStone model.
		[[nodiscard]] unsigned long nTransitions() const;

		/**
		 * It runs the Dhrystone benchmark during DEVStoneAtomic::intDelay milliseconds and passivates.
		 * @param s reference to the model state.
		 */
		void internalTransition(DEVStoneState& s) const override;

		/**
		 * It runs the Dhrystone benchmark during DEVStoneAtomic::extDelay milliseconds and activates.
		 * @param s reference to the current model state.
		 * @param e time elapsed since the last state transition function was triggered.
		 * @param x reference to the atomic model input port set.
		 */
		void externalTransition(DEVStoneState& s, double e) const override;

		/**
		 * It outputs a new event via the DEVStoneAtomic::out Port.
		 * @param s reference to the current model state.
		 * @param y reference to the model output port set.
		 */
		void output(const DEVStoneState& s) const override;

		/**
		 * It returns DEVStoneState::sigma.
		 * @param s reference to the current model state.
		 * @return the value of the state sigma.
		 */
		[[nodiscard]] double timeAdvance(const DEVStoneState& s) const override;
	};
}  //namespace cadmium::example::devstone

#endif //CADMIUM_EXAMPLE_DEVSTONE_ATOMIC_HPP_
