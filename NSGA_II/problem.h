#ifndef PROBLEM_H
#define PROBLEM_H

#include <vector>
#include <cmath>

using namespace std;

class Problem {
public:
	Problem() : objective_id(1) {} // Default constructor
	Problem(int objective_id)
		: objective_id(objective_id)
	{
	}

	double Evaluate(const vector<double>& x) const { // Evaluate the objective function based on the objective ID 1~13
		const int D = x.size();
		double result = 0.0;
		double temp;
		switch (objective_id) {
		case 1: // objective function 1
			for (int i = 0; i < D; ++i) {
				result += x[i] * x[i];
			}
			return result;
			break;

		case 2: // objective function 2
			for (int i = 0; i < D; ++i) {
				result += (x[i] - 2) * (x[i] - 2);
			}
			return result;
			break;

		}
	}

	void GetBounds(double& lower_bound, double& upper_bound) const { // Get the bounds for the objective function based on the objective ID
		switch (objective_id) {
		case 1:
			lower_bound = -1000.0;
			upper_bound = 1000.0;
			break;
		case 2:
			lower_bound = -1000.0;
			upper_bound = 1000.0;
			break;
		}
	}

private:
	int objective_id; // Objective function ID
};

#endif