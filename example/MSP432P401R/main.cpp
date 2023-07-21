/*****************************************************************************
*
* Copyright (C) 2013 - 2017 Texas Instruments Incorporated - http://www.ti.com/
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
*
* * Redistributions of source code must retain the above copyright
*   notice, this list of conditions and the following disclaimer.
*
* * Redistributions in binary form must reproduce the above copyright
*   notice, this list of conditions and the following disclaimer in the
*   documentation and/or other materials provided with the
*   distribution.
*
* * Neither the name of Texas Instruments Incorporated nor the names of
*   its contributors may be used to endorse or promote products derived
*   from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
* A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
* OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
* LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
* THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
******************************************************************************
*
* MSP432P401R cadmium example
*
******************************************************************************/



#include <cadmium/simulation/rt_root_coordinator.hpp>
#include "blinkySystem.hpp"
#ifdef EMBED
    #include <cadmium/simulation/rt_clock/ti_clock.hpp>
    #include <limits>
    #ifndef NO_LOGGING
        #include <cadmium/simulation/logger/stdout.hpp>
    #endif
#else
    #include <cadmium/simulation/rt_clock/rt_clock.hpp>
    #ifndef NO_LOGGING
        #include <cadmium/simulation/logger/csv.hpp>
    #endif
#endif

using namespace cadmium::blinkySystem;

int main(int argc,char* argv[]){
    auto model = std::make_shared<blinkySystem>("blinkySystem");
#ifdef EMBED
    auto clock = cadmium::TIClock();
    auto rootCoordinator = cadmium::RealTimeRootCoordinator(model,clock);
    #ifndef NO_LOGGING
        rootCoordinator.setLogger<cadmium::STDOUTLogger>(";");
    #endif
#else
    auto clock = cadmium::RealTimeClock();
    auto rootCoordinator = cadmium::RealTimeRootCoordinator(model, clock);
    #ifndef NO_LOGGING
        rootCoordinator.setLogger<cadmium::CSVLogger>("blinkyLog.csv",";");
    #endif
#endif
    rootCoordinator.start();
#ifdef EMBED
    rootCoordinator.simulate(std::numeric_limits<double>::infinity());
#else
    rootCoordinator.simulate(100.0);
#endif
    rootCoordinator.stop();
    return 0;
}
