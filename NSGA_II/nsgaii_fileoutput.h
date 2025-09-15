#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <iomanip>     // for setprecision

using namespace std;

void nsgaii_fileoutput(
	const int _run,
	const int _func_id,
	const int _mnfes,
	const int _dim,
	const int _pop_size,
	const vector<vector<vector<double>>>& all_runs_pareto_objectives,
	const vector<double> all_runs_mean
) {
	// ====== 1. 輸出每個 run 的 Pareto Front objectives ======
	for (int run_id = 0; run_id < _run; ++run_id) {
		string filename = "pareto_run" + to_string(run_id + 1) + ".csv";
		ofstream file(filename);

		if (!file.is_open()) {
			cerr << "❌ 無法開啟檔案: " << filename << endl;
			continue;
		}

		file << "f1,f2\n";
		for (const auto& obj : all_runs_pareto_objectives[run_id]) {
			file << fixed << setprecision(10) << obj[0] << "," << obj[1] << "\n";
		}
		file.close();
		cout << "✅ 輸出 Pareto Front: " << filename << endl;
	}
}