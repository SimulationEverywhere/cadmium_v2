#include <cadmium/core/logger/csv.hpp>
#include <cadmium/core/simulation/root_coordinator.hpp>
#include <limits>
#include "efp.hpp"

 using namespace cadmium::example::gpt;

 int main() {
	 auto model = std::make_shared<EFP>("efp", 3, 1, 100);
	 auto rootCoordinator = cadmium::RootCoordinator(model);
	 auto logger = std::make_shared<cadmium::CSVLogger>("log_efp.csv", ";");
	 rootCoordinator.setLogger(logger);
	 rootCoordinator.start();
	 rootCoordinator.simulate(std::numeric_limits<double>::infinity());
	 rootCoordinator.stop();
	 return 0;
 }