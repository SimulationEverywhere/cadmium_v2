/**
 * Experimental Frame-Processor DEVS model.
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

#include <cadmium/core/logger/csv.hpp>
#include <cadmium/core/modeling/atomic.hpp>
#include <cadmium/core/modeling/coupled.hpp>
#include <cadmium/core/simulation/root_coordinator.hpp>
#include <iostream>
#include <limits>
#include <string>

struct Job {
    int id;
    double timeGenerated, timeProcessed;
    explicit Job(int id, double timeGenerated) : id(id), timeGenerated(timeGenerated), timeProcessed(std::numeric_limits<double>::infinity()) {}
};

std::ostream& operator<<(std::ostream &out, Job j) {
	out << "{" << j.id << "," << j.timeGenerated << "," << j.timeProcessed << "}";
	return out;
}

struct GeneratorState {
    double clock, sigma;
    int jobCount;
    GeneratorState() : clock(), sigma(), jobCount() {}
};

std::ostream& operator<<(std::ostream &out, const GeneratorState& s) {
	out << s.jobCount;
	return out;
}

class Generator: public cadmium::Atomic<GeneratorState> {
  private:
    double period;
    std::shared_ptr<cadmium::Port<bool>> stop;
    std::shared_ptr<cadmium::Port<Job>> out;
  public:
    Generator(const std::string& id, double period): cadmium::Atomic<GeneratorState>(id, GeneratorState()),
        period(period), stop(std::make_shared<cadmium::Port<bool>>("stop")) {
        addInPort(stop);
		out = addOutPort<Job>("out");
    }

    void internalTransition(GeneratorState& s) const override {
        s.clock += s.sigma;
        s.sigma = period;
        s.jobCount += 1;
    }
    void externalTransition(GeneratorState& s, double e, const cadmium::PortSet& x) const override {
        s.clock += e;
        s.sigma = (stop->getBag().back())? std::numeric_limits<double>::infinity() : std::max(s.sigma - e, 0.);
    }
    void output(const GeneratorState& s, const cadmium::PortSet& y) const override {
        out->addMessage(Job(s.jobCount, s.clock + s.sigma));
    }
    [[nodiscard]] double timeAdvance(const GeneratorState& s) const override {
        return s.sigma;
    }
};

struct ProcessorState {
    double clock, sigma;
    std::shared_ptr<Job> currentJob;
    explicit ProcessorState() : clock(), sigma(std::numeric_limits<double>::infinity()), currentJob() {}
};

std::ostream& operator<<(std::ostream &out, const ProcessorState& s) {
	out << "{" << s.clock << "," << s.sigma << "}";
	return out;
}

class Processor: public cadmium::Atomic<ProcessorState> {
 private:
    double processingTime;
 public:
    Processor(const std::string& id, double processingTime): cadmium::Atomic<ProcessorState>(id, ProcessorState()),
        processingTime(processingTime) {
        addInPort<Job>("in");
        addOutPort<Job>("out");
    }

    void internalTransition(ProcessorState& s) const override {
		s.clock += s.sigma;
        s.sigma = std::numeric_limits<double>::infinity();
        s.currentJob = nullptr;
    }
    void externalTransition(ProcessorState& s, double e, const cadmium::PortSet& x) const override {
		s.clock += e;
        s.sigma -= e;
        if (s.currentJob == nullptr) {
			auto bag = x.getBag<Job>("in");
            if (!bag.empty()) {
				auto job = bag.back();
                s.currentJob = std::make_shared<Job>(job->id, job->timeGenerated);
                s.sigma = processingTime;
				s.currentJob->timeProcessed = s.clock + s.sigma;
            }
        }
    }
    void output(const ProcessorState& s, const cadmium::PortSet& y) const override {
        y.addMessage("out", *(s.currentJob));
    }
    [[nodiscard]] double timeAdvance(const ProcessorState& s) const override {
        return s.sigma;
    }
};


struct TransducerState {
    double clock;
    double sigma;
    double totalTA;
    int nJobsGenerated;
    int nJobsProcessed;
    explicit TransducerState(double obsTime) : clock(), sigma(obsTime), totalTA(), nJobsGenerated(), nJobsProcessed() {}
};

std::ostream& operator<<(std::ostream &out, const TransducerState& s) {
	out << "{" << s.totalTA << "," << s.nJobsGenerated << "," << s.nJobsProcessed << "}";
	return out;
}

class Transducer: public cadmium::Atomic<TransducerState> {
 public:
    Transducer(const std::string& id, double obsTime):
    cadmium::Atomic<TransducerState>(id, TransducerState(obsTime)) {
        addInPort<Job>("generated");
        addInPort<Job>("processed");
        addOutPort<bool>("stop");
    }

    void internalTransition(TransducerState& s) const override {
        s.clock += s.sigma;
        s.sigma = std::numeric_limits<double>::infinity();

        std::cout << "End time: " << s.clock << std::endl;
        std::cout << "Jobs generated: " << s.nJobsGenerated << std::endl;
        std::cout << "Jobs processed: " << s.nJobsProcessed << std::endl;
        if (s.nJobsProcessed > 0) {
            std::cout << "Average TA: " << s.totalTA / (double) s.nJobsProcessed << std::endl;
        }
        if (s.clock > 0) {
            std::cout << "Throughput: " << (double) s.nJobsProcessed /  s.clock << std::endl;
        }
    }
    void externalTransition(TransducerState& s, double e, const cadmium::PortSet& x) const override {
        s.clock += e;
        s.sigma -= e;
        for (auto& job: x.getBag<Job>("generated")) {
            s.nJobsGenerated += 1;
            std::cout << "Job " << job->id << " generated at t = " << s.clock << std::endl;
        }
        for (auto& job: x.getBag<Job>("processed")) {
            s.nJobsProcessed += 1;
            s.totalTA += job->timeProcessed - job->timeGenerated;
            std::cout << "Job " << job->id << " processed at t = " << s.clock << std::endl;
        }
    }
    void output(const TransducerState& s, const cadmium::PortSet& y) const override {
        y.addMessage("stop", true);
    }
    [[nodiscard]] double timeAdvance(const TransducerState& s) const override {
        return s.sigma;
    }
};


 class ExperimentalFrameProcessor: public cadmium::Coupled {
  public:
     explicit ExperimentalFrameProcessor(const std::string& id, double jobPeriod, double processingTime, double obsTime):
     cadmium::Coupled(id) {
		 auto generator = Generator("generator", jobPeriod);
		 auto processor = Processor("processor", processingTime);
         addComponent(generator);
		 addComponent(processor);
		 addComponent(Transducer("transducer", obsTime));

		 addCoupling(generator.getOutPort("out"), processor.getInPort("in"));
		 addIC("generator", "out", "processor", "in");
		 addIC("generator", "out", "transducer", "generated");
		 addIC("processor", "out", "transducer", "processed");
		 addIC("transducer", "stop", "generator", "stop");
     }
 };

 int main() {
	 auto model = ExperimentalFrameProcessor("efp", 3, 1, 100);
	 auto rootCoordinator = cadmium::RootCoordinator(model);
	 auto logger = std::make_shared<cadmium::CSVLogger>("log.csv", ";");
	 rootCoordinator.setLogger(logger);
	 rootCoordinator.start();
	 rootCoordinator.simulate(std::numeric_limits<double>::infinity());
	 rootCoordinator.stop();
 }
