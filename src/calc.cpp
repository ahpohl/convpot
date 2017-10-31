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
	double x = 0;
	vector<double> capacityVector;
	bool isFullCell = 0;
	double sumVoltage2 = 0;
	double currentSum = 0; int c = 0;
	
	// working electrode
	double capacitySum = 0, energySum = 0, voltageSum = 0;
	double capacityFunc = 0, energyFunc = 0, voltageFunc = 0;
	vector<double> voltageVector;
	
	// counter electrode
	double capacitySum2 = 0, energySum2 = 0, voltageSum2 = 0;
	double capacityFunc2 = 0, energyFunc2 = 0, voltageFunc2 = 0;
	vector<double> voltageVector2;

	// calculations for data points
	for (size_t i = 0; i < recs.size(); ++i) {
		
		// check if full cell (sum voltage2 should be greater than zero)
		sumVoltage2 += recs[i].voltage2;

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
	
	// test if full cell
	isFullCell = (sumVoltage2 > 0) ? 1 : 0;

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

		// get step index
		it->stepIndex = recs[it->end].stepIndex;

		// reset capacity, energy, voltage and current on half cycle change
		capacitySum = 0;
		energySum = 0; energySum2 = 0;
		voltageSum = 0; voltageSum2 = 0;
		currentSum = 0; c = 0;

		// capacity, energy, dQdV
		// integration with trapezoidal rule for non-uniform grid
		// k: data points
		// end: last data point, k < end means skip last data point
		for (size_t k = it->begin; k < (it->end); ++k) {

			// delta time
			x = recs[k+1].stepTime - recs[k].stepTime;

			// average current ignoring rest time
			if (recs[k].current != 0) {
				currentSum += recs[k].current;
				c++;
			}

			// capacity, in As (coloumb C)
			// prevent access to [b+1] on last point (array out of bounds)
			capacityFunc = recs[k+1].current + recs[k].current;
			capacitySum += (0.5 * x * capacityFunc);
			recs[k].capacity = capacitySum;

			// energy, in VAs (Ws), working electrode
			energyFunc = (recs[k+1].voltage * recs[k+1].current) +
					(recs[k+1].voltage * recs[k].current);
			energySum += (0.5 * x * energyFunc);
			recs[k].energy = energySum;
			
			// energy, in VAs (Ws), counter electrode
			energyFunc2 = (recs[k+1].voltage2 * recs[k+1].current) +
					(recs[k+1].voltage2 * recs[k].current);
			energySum2 += (0.5 * x * energyFunc2);
			recs[k].energy2 = energySum2;
			
			// average voltage, in V, working electrode
			voltageFunc = recs[k+1].voltage + recs[k].voltage;
			voltageSum += (0.5 * x * voltageFunc);
			
			// average voltage, in V, counter electrode
			voltageFunc2 = recs[k+1].voltage2 + recs[k].voltage2;
			voltageSum2 += (0.5 * x * voltageFunc2);
		}

		// smooth capacity and current for each half cycle
		if (args.smooth > 0) {
#if(DEBUG == 1)
			cout << "Using a smooth value of " << args.smooth << endl;
#endif
			util::boxFIR box(args.smooth);
			size_t i = 0;

			for (size_t k = it->begin; k < (it->end); ++k) {
				capacityVector.push_back(recs[k].capacity);
				// working electrode
				voltageVector.push_back(recs[k].voltage);
				// counter electrode
				voltageVector2.push_back(recs[k].voltage2);
			}

			// smooth current
			box.filter(capacityVector);
			// smooth working electrode voltage
			box.filter(voltageVector);
			// smooth counter electrode voltage
			box.filter(voltageVector2);;

			for (size_t k = it->begin; k < (it->end); ++k, ++i) {
				recs[k].capacity = capacityVector[i];
				// working electrode
				recs[k].voltage = voltageVector[i];
				// counter electrode
				recs[k].voltage2 = voltageVector2[i];
			}

			// reset vectors
			capacityVector.clear();
			voltageVector.clear();
			voltageVector2.clear();
			i = 0;
		}

		// dQdV, in As V-1
		// separate loop because we have to calculate capacity[k+1]
		// which is not known in first loop over data points
		// stop two points before last data point because capacity
		// of last point is zero. Check for rest time and only calculate 
		// counter electorde if full cell
		for (size_t k = it->begin; k < (it->end-1); ++k) {
			// working electrode
			recs[k].dQdV = (recs[k].stepIndex != 0) ? ((recs[k+1].capacity - recs[k].capacity) / 
				(recs[k+1].voltage - recs[k].voltage)) : 0;
			// counter electrode
			recs[k].dQdV2 = ((recs[k].stepIndex != 0) && (isFullCell > 0)) ? 
				((recs[k+1].capacity - recs[k].capacity) / (recs[k+1].voltage2 - recs[k].voltage2)) : 0;
		}

		// save half cycle, take value before last data point
		it->capacity = recs[(it->end)-1].capacity;
		
		// working electrode
		it->energy = recs[(it->end)-1].energy;
		it->averageVoltage = voltageSum / recs[(it->end)-1].stepTime;
		
		// counter electrode
		it->energy2 = recs[(it->end)-1].energy2;
		it->averageVoltage2 = voltageSum2 / recs[(it->end)-1].stepTime;

		// average current
		it->averageCurrent = currentSum / c;

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
			fullCycles.back().chargeCurrent = it->averageCurrent;
			fullCycles.back().chargeCapacity = it->capacity;
			// working electrode
			fullCycles.back().chargeEnergy = it->energy;
			fullCycles.back().chargeVoltage = it->averageVoltage;
			// counter electrode
			fullCycles.back().chargeEnergy2 = it->energy2;
			fullCycles.back().chargeVoltage2 = it->averageVoltage2;
		} else {
			fullCycles.back().dischargeTime = it->stepTime;
			fullCycles.back().dischargeCurrent = it->averageCurrent;
			fullCycles.back().dischargeCapacity = it->capacity;
			// working electrode
			fullCycles.back().dischargeEnergy = it->energy;
			fullCycles.back().dischargeVoltage = it->averageVoltage;
			// counter electrode
			fullCycles.back().dischargeEnergy2 = it->energy2;
			fullCycles.back().dischargeVoltage2 = it->averageVoltage2;
		}
	}

	// calculation on full cycles
	for (vector<full_t>::iterator it = fullCycles.begin(); it != fullCycles.end(); ++it) {

		// cycle index
		it->fullCycle = distance(fullCycles.begin(), it);

		// voltage hysteresis, in V
		it->hysteresis = (it->chargeVoltage) - (it->dischargeVoltage);
		it->hysteresis2 = (it->chargeVoltage2) - (it->dischargeVoltage2);

		// coulombic efficiency
		it->efficiency = fabs(it->dischargeCapacity) / (it->chargeCapacity);
	}
}
