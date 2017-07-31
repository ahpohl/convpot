#include "device.h"
#include "util.h"

#include <iostream>
#include <cmath>
using namespace std;

// do calculations for each data point, half and full cycles
void Device::calculateCycles()
{
	half_t* half;
	full_t* full;
	size_t halfCounter = 0;
	double startTime = 0;
	int previous = 0, current = 0;
	double sum1 = 0, sum2 = 0, sum3 = 0, x = 0;
	double func1 = 0, func2 = 0, func3 = 0;
	vector<double> voltageVector, capacityVector;

	// calculations for data points
	for (size_t i = 0; i < recs.size(); ++i) {

		// get current step index, ignore OCV points
		if (recs[i].stepIndex) {
			current = recs[i].stepIndex;
		}

		// detect half cycle
		if ( previous != current ) {

			// create pointer
			half = new half_t({0});

			// end of previous half cycle index
			if (halfCounter > 0) {
				halfCycles[halfCounter-1].end = i - 1;
			}

			// step time of previous half cycle
			if (halfCounter > 0) {
				halfCycles[halfCounter-1].stepTime = recs[i-1].stepTime;
			}

			// step time start, take previous data point
			startTime = (halfCounter == 0) ? 0 : recs[i-1].testTime;

			// begin data point, always begin from zero
			half->begin = (halfCounter == 0) ? 0 : i;

			// half cycle index
			half->halfCycle = halfCounter++;

			// save
			halfCycles.push_back(*half);
			delete half;
		}

		// step time
		recs[i].stepTime = recs[i].testTime - startTime;

		// half cycle index
		recs[i].halfCycle = (halfCounter == 0) ? (halfCounter) : (halfCounter - 1);

		// full cycle index
		recs[i].fullCycle = (halfCounter == 0) ? (halfCounter / 2) : ((halfCounter - 1) / 2);

		// save step
		previous = current;
	}

	// end of last half cycle
	if ( ! halfCycles.empty() ) {
		halfCycles.back().end = recs.back().dataPoint;
	}

	// step time of last half cycle
	if ( ! halfCycles.empty() ) {
		halfCycles.back().stepTime = recs.back().stepTime;
	}

	// calculations for half cycles
	// note: reuse of halfCounter variable
	halfCounter = 0;

	for (vector<half_t>::iterator it = halfCycles.begin(); it != halfCycles.end(); ++it) {

		// get half cycle index
		halfCounter = distance(halfCycles.begin(), it);

		// reset capacity and energy on half cycle change
		sum1 = 0;
		sum2 = 0;
		sum3 = 0;

		// capacity, energy, dQdV
		// integration with trapezoidal rule for non-uniform grid
		// k: data points
		// end: last data point, k < end means skip last data point
		for (size_t k = it->begin; k < (it->end); ++k) {

			// delta time
			x = recs[k+1].stepTime - recs[k].stepTime;

			// capacity, in As (coloumb C)
			// prevent access to [b+1] on last point (array out of bounds)
			func1 = recs[k+1].current + recs[k].current;
			sum1 += (0.5 * x * func1);
			recs[k].capacity = sum1;

			// energy, in VAs (Ws)
			func2 = (recs[k+1].voltage * recs[k+1].current) +
					(recs[k+1].voltage * recs[k].current);
			sum2 += (0.5 * x * func2);
			recs[k].energy = sum2;

			// average voltage, in V
			func3 = recs[k+1].voltage + recs[k].voltage;
			sum3 += (0.5 * x * func3);
		}

		// smooth capacity and current for each half cycle
		if (args.smooth > 0) {
#if(DEBUG == 1)
			cout << "Using a smooth value of " << args.smooth << endl;
#endif
			util::boxFIR box(args.smooth);
			size_t i = 0;

			for (size_t k = it->begin; k < (it->end); ++k) {
				voltageVector.push_back(recs[k].voltage);
				capacityVector.push_back(recs[k].capacity);
			}

			// smooth voltage and current
			box.filter(voltageVector);
			box.filter(capacityVector);

			for (size_t k = it->begin; k < (it->end); ++k, ++i) {
				recs[k].voltage = voltageVector[i];
				recs[k].capacity = capacityVector[i];
			}

			// reset vectors
			voltageVector.clear();
			capacityVector.clear();
			i = 0;
		}

		// dQdV, in As V-1
		// separate loop because we have to calculate capacity[k+1]
		// which is not known in first loop over data points
		// stop two points before last data point because capacity
		// of last point is zero
		for (size_t k = it->begin; k < (it->end-1); ++k) {
			recs[k].dQdV = ((recs[k+1].capacity - recs[k].capacity) /
				(recs[k+1].voltage - recs[k].voltage));
		}

		// save half cycle, take value before last data point
		it->capacity = recs[(it->end)-1].capacity;
		it->energy = recs[(it->end)-1].energy;
		it->averageVoltage = sum3 / recs[(it->end)-1].stepTime;

		// calculations for full cycles
		// create new full cycle only on even half cycles
		if ( (halfCounter % 2) == 0 ) {
			full = new full_t({0});
			full->begin = it->begin;
			fullCycles.push_back(*full);
			delete full;
		}

		// update fullCycles on every half cycle in case the
		// full cycle in incomplete
		fullCycles.back().end = it->end;

		if (it->capacity >= 0) {
			fullCycles.back().chargeTime = it->stepTime;
			fullCycles.back().chargeCapacity = it->capacity;
			fullCycles.back().chargeEnergy = it->energy;
			fullCycles.back().chargeVoltage = it->averageVoltage;
		} else {
			fullCycles.back().dischargeTime = it->stepTime;
			fullCycles.back().dischargeCapacity = it->capacity;
			fullCycles.back().dischargeEnergy = it->energy;
			fullCycles.back().dischargeVoltage = it->averageVoltage;
		}
	}

	// calculation on full cycles
	for (vector<full_t>::iterator it = fullCycles.begin(); it != fullCycles.end(); ++it) {

		// cycle index
		it->fullCycle = distance(fullCycles.begin(), it);

		// voltage hysteresis, in V
		it->hysteresis = (it->chargeVoltage) - (it->dischargeVoltage);

		// coulombic efficiency
		it->efficiency = fabs(it->dischargeCapacity) / (it->chargeCapacity);
	}
}
