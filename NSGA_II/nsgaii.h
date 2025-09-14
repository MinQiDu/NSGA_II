#ifndef NSGAII_H
#define NSGAII_H

#include <vector>
#include <algorithm>
#include <random>
#include <iostream>
#include <numeric>
#include "problem.h"
#include "nsgaii_fileoutput.h"

using namespace std;

class NSGAII {
public:
	NSGAII();
	struct individual {
		int rank;					   // 位於哪層前沿
		double crowding_distance;	   // 擁擠距離
		vector<double> solution;	   // 解
		double objective1, objective2; // 多個目標值
	};

	void RunALG(int run, int func_id, int mnfes, int dim, int pop_size, double CR, double MR);

private:
	random_device rd;  /*宣告亂數引擎*/
	mt19937 gen;

	// command line parameters
	int run;		// 執行次數
	int func_id;	// 測試函數編號
	int mnfes;		// 最大函數評估次數
	int dim;		// 問題維度
	int pop_size;	// 母體大小
	double CR;		// 交配率
	double MR;		// 變異率

	// algorithm parameters
	int nfes;					   // 當前函數評估次數
	vector<individual> population; // 母體
	vector<individual> offspring;  // 子代

	// problem
	Problem problem1;
	Problem problem2;

	void Init();
	void Crossover(const vector<individual>& population, vector<individual>& offspring);
	void Mutation(vector<individual>& offspring);
	void Determination();

	void FastNonDominatedSort(const vector<individual>& combined_population); // 快速非支配排序
	void CrowdingDistanceAssignment(vector<individual>& population);          // 擁擠距離分配

	void Evaluation(vector<individual>& population); // 實際評估函數得到目標值 objective1, objective2, nfes++

	double CalMean(const vector<vector<double>>& single_run_pareto_objectives);
};

NSGAII::NSGAII()
	: gen(rd()) // 使用隨機設備初始化 Mersenne Twister 亂數引擎
{
}

double NSGAII::CalMean(const vector<vector<double>>& single_run_pareto_objectives) {
	if (single_run_pareto_objectives.empty()) return -1; // 若無資料則回傳 -1
	// 產生 optimal Pareto front 的目標值組合
	vector<vector<double>> optimal_objectives;
	double lower = 0.0; // x 的optimal範圍
	double upper = 2.0;

	for (int i = 0; i < pop_size; ++i) {
		double x = lower + (upper - lower) * i / (pop_size - 1);  // 等距 x 值
		double f1 = x * x;
		double f2 = (x - 2) * (x - 2);
		optimal_objectives.push_back({ f1, f2 }); // 儲存 optimal f(x), 目標值組合
	}
	// 計算 mean value
	double total_dist = 0.0;
	for (const auto& p : single_run_pareto_objectives) {
		double min_dist = 1e9;
		for (const auto& opt : optimal_objectives) {
			double d = sqrt(pow(p[0] - opt[0], 2) + pow(p[1] - opt[1], 2));
			if (d < min_dist)
				min_dist = d;
		}
		total_dist += min_dist;
	}
	double mean_distance = total_dist / single_run_pareto_objectives.size();
	return mean_distance;
}

void NSGAII::RunALG(int _run, int _func_id, int _mnfes, int _dim, int _pop_size, double _CR, double _MR) {
	run = _run;
	func_id = _func_id;
	mnfes = _mnfes;
	dim = _dim;
	pop_size = _pop_size;
	CR = _CR;
	MR = _MR;

	// Initialize problem instances
	if (func_id == 1) {
		problem1 = Problem(1); // Objective function 1
		problem2 = Problem(2); // Objective function 2
	}
	else {
		cout << "Invalid func_id! Only func_id=1 supported." << endl;
		return;
	}

	vector<vector<vector<double>>> all_runs_pareto_objectives; // 儲存每run執行的 Pareto objective values
	vector<double> all_runs_means;							   // 儲存每run執行的 mean value
	for (int r = 0; r < run; ++r) {
		Init();
		
		// 主迴圈
		while (nfes < mnfes) {
			// 清空子代供新一代使用
			offspring.clear();

			// 母代的排序與擁擠距離計算
			FastNonDominatedSort(population);
			CrowdingDistanceAssignment(population);

			// 產生子代
			Crossover(population, offspring);
			Mutation(offspring);
			Evaluation(offspring);

			// 母代子代合併後要再排序與擁擠距離計算
			population.insert(population.end(), offspring.begin(), offspring.end());
			FastNonDominatedSort(population);
			CrowdingDistanceAssignment(population);

			// 最終選擇下一代母體
			Determination();
			// cout << "[Debug] Generation ends, nfes: " << nfes << endl;
		}
		// 此run結束，輸出結果
		cout << "Pareto Front (Final Population):" << endl;
		vector<vector<double>> single_run_pareto_objective;
		for (const auto& ind : population) {
			if (ind.rank == 1) { // 只輸出第一層 Pareto 前沿
				cout << "f1: " << ind.objective1 << ", f2: " << ind.objective2 << "" << endl;
				single_run_pareto_objective.push_back({ ind.objective1, ind.objective2 }); // 儲存此run pareto 前沿目標值
			}
		}
		all_runs_pareto_objectives.push_back(single_run_pareto_objective); // 存入整體run的 pareto 前沿目標值
		all_runs_means.push_back(CalMean(single_run_pareto_objective));	   // 計算並儲存此run的 mean value
	}
	// 計算所有 run 的平均 mean value
	double avg_mean = accumulate(all_runs_means.begin(), all_runs_means.end(), 0.0) / all_runs_means.size();
	cout << "avg mean of distances to optimal pareto front: " << avg_mean << endl;

	// 所有runs結束，輸出結果到檔案
	nsgaii_fileoutput(run, func_id, mnfes, dim, pop_size, all_runs_pareto_objectives, all_runs_means);
}

/* 初始化母體 */
void NSGAII::Init() {
	nfes = 0;
	population.resize(pop_size);
	offspring.resize(pop_size);
	uniform_real_distribution<double> dist(0, 1);

	// 得到上下限
	double lower_bound1, upper_bound1;
	double lower_bound2, upper_bound2;
	problem1.GetBounds(lower_bound1, upper_bound1);
	problem2.GetBounds(lower_bound2, upper_bound2);

	for (int i = 0; i < pop_size; ++i) {
		population[i].solution.resize(dim);
		population[i].objective1 = 0.0;
		population[i].objective2 = 0.0;
		for (int d = 0; d < dim; ++d) {
			double rand_val = dist(gen); // 產生 [0,1) 之間的亂數
			population[i].solution[d] = lower_bound1 + rand_val * (upper_bound1 - lower_bound1); // ?upper/lower_bound2呢?
		}
		population[i].rank = -1;
		population[i].crowding_distance = 0.0;
	}
	Evaluation(population); // 母體建好後，評估初始母體的目標值 objective1, objective2
	// cout << "[Debug] Init OK, nfes:" << nfes << endl;
}

/* 快速非支配排序 */
void NSGAII::FastNonDominatedSort(const vector<individual>& combined_population) {
	int N = combined_population.size();
	vector<vector<int>> S(N); // S[i] 儲存被個體 i 支配的個體索引
	vector<int> n(N, 0);      // n[i] 儲存支配個體 i 的個體數量
	vector<vector<int>> fronts; // 儲存各前沿的個體索引
	for (int p = 0; p < N; ++p) {
		for (int q = 0; q < N; ++q) {
			if (p == q) continue; // 跳過自己
			if ((combined_population[p].objective1 < combined_population[q].objective1 && combined_population[p].objective2 <= combined_population[q].objective2) ||
				(combined_population[p].objective1 <= combined_population[q].objective1 && combined_population[p].objective2 < combined_population[q].objective2)) {
				S[p].push_back(q); // p 支配 q
			} 
			else if ((combined_population[q].objective1 < combined_population[p].objective1 && combined_population[q].objective2 <= combined_population[p].objective2) ||
					 (combined_population[q].objective1 <= combined_population[p].objective1 && combined_population[q].objective2 < combined_population[p].objective2)) {
				n[p]++; // q 支配 p
			}
		}
		if (n[p] == 0) { // p不受任何個體支配->屬於第一前沿
			population[p].rank = 1; // 第一前沿
			if (fronts.empty()) fronts.push_back(vector<int>());
			fronts[0].push_back(p); // 加入第一前沿
		}
	}
	int r = 0; // 當前前沿索引
	while (r < fronts.size() && !fronts[r].empty()) {
		vector<int> next_front;   // 暫存下一前沿
		for (int p : fronts[r]) { // 對當前前沿的每個個體 p
			for (int q : S[p]) {  // 對被 p 支配的每個個體 q
				n[q]--;           // 減少支配 q 的個體數量
				if (n[q] == 0) {  // 如果 q 不再被任何個體支配
					next_front.push_back(q); // 將 q 加入下一前沿
				}
			}
		}
		if (!next_front.empty()) {
			fronts.push_back(next_front); // 將下一前沿加入 fronts
		}
		r++; // 移動到下一前沿
	}
	// 更新個體的 rank
	for (int f = 0; f < fronts.size(); ++f) {
		for (int idx : fronts[f]) {
			population[idx].rank = f + 1; // rank 從 1 開始
		}
	}
}

/* 擁擠距離分配 */
void NSGAII::CrowdingDistanceAssignment(vector<individual>& population) {
	int N = population.size();
	if (N == 0) return; // 母體若空直接return
	// 初始化擁擠距離
	for (int i = 0; i < N; ++i) {
		population[i].crowding_distance = 0.0;
	}
	// 對每個目標進行排序並計算擁擠距離
	for (int m = 1; m <= 2; ++m) { // 兩個目標
		// 根據目標 m 排序
		if (m == 1) {
			sort(population.begin(), population.end(), [](const individual& a, const individual& b) {
				return a.objective1 < b.objective1;
				});
		}
		else { // if (m == 2)
			sort(population.begin(), population.end(), [](const individual& a, const individual& b) {
				return a.objective2 < b.objective2;
				});
		}
		// 設置邊界點的擁擠距離為無限大
		population[0].crowding_distance = numeric_limits<double>::infinity();
		population[N - 1].crowding_distance = numeric_limits<double>::infinity();
		// 計算其他個體的擁擠距離
		double f_min, f_max; // 目標 m 的最小值和最大值
		if (m == 1) {
			f_min = population[0].objective1;
			f_max = population[N - 1].objective1;
		}
		else {
			f_min = population[0].objective2;
			f_max = population[N - 1].objective2;
		}
		// 累計中間個體的擁擠距離
		for (int i = 1; i < N - 1; ++i) {
			double prev_obj, next_obj;
			if (m == 1) {
				prev_obj = population[i - 1].objective1;
				next_obj = population[i + 1].objective1;
			}
			else {
				prev_obj = population[i - 1].objective2;
				next_obj = population[i + 1].objective2;
			}
			if (f_max - f_min == 0) {
				population[i].crowding_distance += 0.0; // 避免除以零
			}
			else {
				population[i].crowding_distance += (next_obj - prev_obj) / (f_max - f_min);
			}
		}
	}
}

/* 模擬二進制交叉（SBX，Simulated Binary Crossover） */
void NSGAII::Crossover(const vector<individual>& population, vector<individual>& offspring) {
	int N = population.size();
	double eta_c = 20.0; // SBX 分佈指數 (通常設為 10~30)

	uniform_int_distribution<int> dist_parent(0, N - 1);
	uniform_real_distribution<double> dist_rate(0, 1);

	for (int i = 0; i < N/2; ++i) {
		// 隨機選擇兩個父代
		int idx_p1 = dist_parent(gen); // 有沒有需要避免重複選擇?
		int idx_p2 = dist_parent(gen);
		const individual& parent1 = population[idx_p1];
		const individual& parent2 = population[idx_p2];
		// 初始化兩個子代
		individual child1, child2;
		child1.solution.resize(dim);
		child2.solution.resize(dim);
		
		// SBX 交配率判斷
		if (dist_rate(gen) < CR) { // 進行交配
			for (int d = 0; d < dim; ++d) {
				double x1 = parent1.solution[d];
				double x2 = parent2.solution[d];

				if (fabs(x1 - x2) < 1e-14) {
					// 若兩個父代在該維度上相等，則直接複製
					child1.solution[d] = x1;
					child2.solution[d] = x2;
				}
				else {
					double y1 = min(x1, x2);
					double y2 = max(x1, x2);

					double rand_beta = dist_rate(gen);
					double beta; // 分佈係數
					if (rand_beta <= 0.5) {
						beta = pow(2.0 * rand_beta, 1.0 / (eta_c + 1.0));				  // beta 會小於 1（偏向雙親中間）
					}
					else {
						beta = pow(1.0 / (2.0 * (1.0 - rand_beta)), 1.0 / (eta_c + 1.0)); // beta 會大於 1（偏向雙親外側）
					}
					// 產生兩個子代
					child1.solution[d] = 0.5 * ((1 + beta) * x1 + (1 - beta) * x2);
					child2.solution[d] = 0.5 * ((1 - beta) * x1 + (1 + beta) * x2);

					// 邊界處理，以problem1為主
					double lower_bound1, upper_bound1;
					problem1.GetBounds(lower_bound1, upper_bound1);
					child1.solution[d] = max(lower_bound1, min(upper_bound1, child1.solution[d]));
					child2.solution[d] = max(lower_bound1, min(upper_bound1, child2.solution[d]));
				}
			}
		
		}
		else { // 不進行交配，直接複製
			child1.solution = parent1.solution;
			child2.solution = parent2.solution;
		}

		// 初始化子代其他屬性
		child1.rank = -1;
		child1.crowding_distance = 0.0;
		child1.objective1 = 0.0;
		child1.objective2 = 0.0;

		child2.rank = -1;
		child2.crowding_distance = 0.0;
		child2.objective1 = 0.0;
		child2.objective2 = 0.0;

		// 將子代加入 offspring
		offspring.push_back(child1);
		offspring.push_back(child2);
	}
}

/* 變異 */
void NSGAII::Mutation(vector<individual>& offspring) {
	int N = offspring.size();
	double eta_m = 20.0; // 多點變異分佈指數 (通常設為 10~50)
	uniform_real_distribution<double> dist_rate(0, 1);
	// 得到上下限
	double lower_bound1, upper_bound1;
	problem1.GetBounds(lower_bound1, upper_bound1);
	for (int i = 0; i < N; ++i) {
		for (int d = 0; d < dim; ++d) {
			if (dist_rate(gen) < MR) { // 進行變異
				double y = offspring[i].solution[d];
				double delta1 = (y - lower_bound1) / (upper_bound1 - lower_bound1);
				double delta2 = (upper_bound1 - y) / (upper_bound1 - lower_bound1);
				double rand_delta = dist_rate(gen);
				double mut_pow = 1.0 / (eta_m + 1.0);
				double deltaq;
				if (rand_delta <= 0.5) {
					double xy = 1.0 - delta1;
					double val = 2.0 * rand_delta + (1.0 - 2.0 * rand_delta) * pow(xy, eta_m + 1.0);
					deltaq = pow(val, mut_pow) - 1.0;
				}
				else {
					double xy = 1.0 - delta2;
					double val = 2.0 * (1.0 - rand_delta) + 2.0 * (rand_delta - 0.5) * pow(xy, eta_m + 1.0);
					deltaq = 1.0 - pow(val, mut_pow);
				}
				// 更新變異後的值並進行邊界處理
				y = y + deltaq * (upper_bound1 - lower_bound1);
				y = max(lower_bound1, min(upper_bound1, y));
				offspring[i].solution[d] = y;
			}
		}
	}
}

/* 評估目標值 objective1, objective2 */
void NSGAII::Evaluation(vector<individual>& population) {
	for (auto& ind : population) {
		ind.objective1 = problem1.Evaluate(ind.solution);
		ind.objective2 = problem2.Evaluate(ind.solution);
		nfes += 2; // 每評估一個個體，nfes 增加 2（因為有兩個目標函數）
	}
	// cout << "[Debug] Evaluation OK, nfes: " << nfes << endl;
}

/* 選擇下一代母體 */
void NSGAII::Determination() {
	// 建立 fronts, 儲存各前沿的個體, fronts[0] = 第一前沿, fronts[1] = 第二前沿, ...
	vector<vector<individual>> fronts; 
	for (const auto& ind : population) {
		if (ind.rank > fronts.size()) {
			fronts.resize(ind.rank); // 動態調整 fronts 大小
		}
		fronts[ind.rank - 1].push_back(ind); // 把個體放入對應的 rank
	}

	// 決定新的母體
	vector<individual> new_population; // 新的母體
	int new_size = 0;				   // 新母體計數, 用來確保新母體不超過 pop_size

	// 遍歷fronts中各前沿
	for (auto& front : fronts) {
		if (new_size + front.size() <= pop_size) {
			// 如果整個前沿可以加入新母體
			new_population.insert(new_population.end(), front.begin(), front.end());
			new_size += front.size();
		}
		else {
			// 如果前沿無法完全加入新母體, 需要根據擁擠距離選擇
			sort(front.begin(), front.end(), [](const individual& a, const individual& b) {
				return a.crowding_distance > b.crowding_distance; // 擁擠距離由大到小排序
				});
			int remain = pop_size - new_size; // 剩餘可加入新母體的名額
			new_population.insert(new_population.end(), front.begin(), front.begin() + remain);
			new_size += remain;
			break; // 新母體已滿, 退出
		}
	}
	population = new_population; // 更新母體
}

#endif
