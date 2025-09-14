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
		int rank;					   // �����h�e�u
		double crowding_distance;	   // �����Z��
		vector<double> solution;	   // ��
		double objective1, objective2; // �h�ӥؼЭ�
	};

	void RunALG(int run, int func_id, int mnfes, int dim, int pop_size, double CR, double MR);

private:
	random_device rd;  /*�ŧi�üƤ���*/
	mt19937 gen;

	// command line parameters
	int run;		// ���榸��
	int func_id;	// ���ը�ƽs��
	int mnfes;		// �̤j��Ƶ�������
	int dim;		// ���D����
	int pop_size;	// ����j�p
	double CR;		// ��t�v
	double MR;		// �ܲ��v

	// algorithm parameters
	int nfes;					   // ��e��Ƶ�������
	vector<individual> population; // ����
	vector<individual> offspring;  // �l�N

	// problem
	Problem problem1;
	Problem problem2;

	void Init();
	void Crossover(const vector<individual>& population, vector<individual>& offspring);
	void Mutation(vector<individual>& offspring);
	void Determination();

	void FastNonDominatedSort(const vector<individual>& combined_population); // �ֳt�D��t�Ƨ�
	void CrowdingDistanceAssignment(vector<individual>& population);          // �����Z�����t

	void Evaluation(vector<individual>& population); // ��ڵ�����Ʊo��ؼЭ� objective1, objective2, nfes++

	double CalMean(const vector<vector<double>>& single_run_pareto_objectives);
};

NSGAII::NSGAII()
	: gen(rd()) // �ϥ��H���]�ƪ�l�� Mersenne Twister �üƤ���
{
}

double NSGAII::CalMean(const vector<vector<double>>& single_run_pareto_objectives) {
	if (single_run_pareto_objectives.empty()) return -1; // �Y�L��ƫh�^�� -1
	// ���� optimal Pareto front ���ؼЭȲզX
	vector<vector<double>> optimal_objectives;
	double lower = 0.0; // x ��optimal�d��
	double upper = 2.0;

	for (int i = 0; i < pop_size; ++i) {
		double x = lower + (upper - lower) * i / (pop_size - 1);  // ���Z x ��
		double f1 = x * x;
		double f2 = (x - 2) * (x - 2);
		optimal_objectives.push_back({ f1, f2 }); // �x�s optimal f(x), �ؼЭȲզX
	}
	// �p�� mean value
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

	vector<vector<vector<double>>> all_runs_pareto_objectives; // �x�s�Crun���檺 Pareto objective values
	vector<double> all_runs_means;							   // �x�s�Crun���檺 mean value
	for (int r = 0; r < run; ++r) {
		Init();
		
		// �D�j��
		while (nfes < mnfes) {
			// �M�Ťl�N�ѷs�@�N�ϥ�
			offspring.clear();

			// ���N���ƧǻP�����Z���p��
			FastNonDominatedSort(population);
			CrowdingDistanceAssignment(population);

			// ���ͤl�N
			Crossover(population, offspring);
			Mutation(offspring);
			Evaluation(offspring);

			// ���N�l�N�X�֫�n�A�ƧǻP�����Z���p��
			population.insert(population.end(), offspring.begin(), offspring.end());
			FastNonDominatedSort(population);
			CrowdingDistanceAssignment(population);

			// �̲׿�ܤU�@�N����
			Determination();
			// cout << "[Debug] Generation ends, nfes: " << nfes << endl;
		}
		// ��run�����A��X���G
		cout << "Pareto Front (Final Population):" << endl;
		vector<vector<double>> single_run_pareto_objective;
		for (const auto& ind : population) {
			if (ind.rank == 1) { // �u��X�Ĥ@�h Pareto �e�u
				cout << "f1: " << ind.objective1 << ", f2: " << ind.objective2 << "" << endl;
				single_run_pareto_objective.push_back({ ind.objective1, ind.objective2 }); // �x�s��run pareto �e�u�ؼЭ�
			}
		}
		all_runs_pareto_objectives.push_back(single_run_pareto_objective); // �s�J����run�� pareto �e�u�ؼЭ�
		all_runs_means.push_back(CalMean(single_run_pareto_objective));	   // �p����x�s��run�� mean value
	}
	// �p��Ҧ� run ������ mean value
	double avg_mean = accumulate(all_runs_means.begin(), all_runs_means.end(), 0.0) / all_runs_means.size();
	cout << "avg mean of distances to optimal pareto front: " << avg_mean << endl;

	// �Ҧ�runs�����A��X���G���ɮ�
	nsgaii_fileoutput(run, func_id, mnfes, dim, pop_size, all_runs_pareto_objectives, all_runs_means);
}

/* ��l�ƥ��� */
void NSGAII::Init() {
	nfes = 0;
	population.resize(pop_size);
	offspring.resize(pop_size);
	uniform_real_distribution<double> dist(0, 1);

	// �o��W�U��
	double lower_bound1, upper_bound1;
	double lower_bound2, upper_bound2;
	problem1.GetBounds(lower_bound1, upper_bound1);
	problem2.GetBounds(lower_bound2, upper_bound2);

	for (int i = 0; i < pop_size; ++i) {
		population[i].solution.resize(dim);
		population[i].objective1 = 0.0;
		population[i].objective2 = 0.0;
		for (int d = 0; d < dim; ++d) {
			double rand_val = dist(gen); // ���� [0,1) �������ü�
			population[i].solution[d] = lower_bound1 + rand_val * (upper_bound1 - lower_bound1); // ?upper/lower_bound2�O?
		}
		population[i].rank = -1;
		population[i].crowding_distance = 0.0;
	}
	Evaluation(population); // ����ئn��A������l���骺�ؼЭ� objective1, objective2
	// cout << "[Debug] Init OK, nfes:" << nfes << endl;
}

/* �ֳt�D��t�Ƨ� */
void NSGAII::FastNonDominatedSort(const vector<individual>& combined_population) {
	int N = combined_population.size();
	vector<vector<int>> S(N); // S[i] �x�s�Q���� i ��t���������
	vector<int> n(N, 0);      // n[i] �x�s��t���� i ������ƶq
	vector<vector<int>> fronts; // �x�s�U�e�u���������
	for (int p = 0; p < N; ++p) {
		for (int q = 0; q < N; ++q) {
			if (p == q) continue; // ���L�ۤv
			if ((combined_population[p].objective1 < combined_population[q].objective1 && combined_population[p].objective2 <= combined_population[q].objective2) ||
				(combined_population[p].objective1 <= combined_population[q].objective1 && combined_population[p].objective2 < combined_population[q].objective2)) {
				S[p].push_back(q); // p ��t q
			} 
			else if ((combined_population[q].objective1 < combined_population[p].objective1 && combined_population[q].objective2 <= combined_population[p].objective2) ||
					 (combined_population[q].objective1 <= combined_population[p].objective1 && combined_population[q].objective2 < combined_population[p].objective2)) {
				n[p]++; // q ��t p
			}
		}
		if (n[p] == 0) { // p������������t->�ݩ�Ĥ@�e�u
			population[p].rank = 1; // �Ĥ@�e�u
			if (fronts.empty()) fronts.push_back(vector<int>());
			fronts[0].push_back(p); // �[�J�Ĥ@�e�u
		}
	}
	int r = 0; // ��e�e�u����
	while (r < fronts.size() && !fronts[r].empty()) {
		vector<int> next_front;   // �Ȧs�U�@�e�u
		for (int p : fronts[r]) { // ���e�e�u���C�ӭ��� p
			for (int q : S[p]) {  // ��Q p ��t���C�ӭ��� q
				n[q]--;           // ��֤�t q ������ƶq
				if (n[q] == 0) {  // �p�G q ���A�Q��������t
					next_front.push_back(q); // �N q �[�J�U�@�e�u
				}
			}
		}
		if (!next_front.empty()) {
			fronts.push_back(next_front); // �N�U�@�e�u�[�J fronts
		}
		r++; // ���ʨ�U�@�e�u
	}
	// ��s���骺 rank
	for (int f = 0; f < fronts.size(); ++f) {
		for (int idx : fronts[f]) {
			population[idx].rank = f + 1; // rank �q 1 �}�l
		}
	}
}

/* �����Z�����t */
void NSGAII::CrowdingDistanceAssignment(vector<individual>& population) {
	int N = population.size();
	if (N == 0) return; // ����Y�Ū���return
	// ��l�ƾ����Z��
	for (int i = 0; i < N; ++i) {
		population[i].crowding_distance = 0.0;
	}
	// ��C�ӥؼжi��ƧǨíp������Z��
	for (int m = 1; m <= 2; ++m) { // ��ӥؼ�
		// �ھڥؼ� m �Ƨ�
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
		// �]�m����I�������Z�����L���j
		population[0].crowding_distance = numeric_limits<double>::infinity();
		population[N - 1].crowding_distance = numeric_limits<double>::infinity();
		// �p���L���骺�����Z��
		double f_min, f_max; // �ؼ� m ���̤p�ȩM�̤j��
		if (m == 1) {
			f_min = population[0].objective1;
			f_max = population[N - 1].objective1;
		}
		else {
			f_min = population[0].objective2;
			f_max = population[N - 1].objective2;
		}
		// �֭p�������骺�����Z��
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
				population[i].crowding_distance += 0.0; // �קK���H�s
			}
			else {
				population[i].crowding_distance += (next_obj - prev_obj) / (f_max - f_min);
			}
		}
	}
}

/* �����G�i���e�]SBX�ASimulated Binary Crossover�^ */
void NSGAII::Crossover(const vector<individual>& population, vector<individual>& offspring) {
	int N = population.size();
	double eta_c = 20.0; // SBX ���G���� (�q�`�]�� 10~30)

	uniform_int_distribution<int> dist_parent(0, N - 1);
	uniform_real_distribution<double> dist_rate(0, 1);

	for (int i = 0; i < N/2; ++i) {
		// �H����ܨ�Ӥ��N
		int idx_p1 = dist_parent(gen); // ���S���ݭn�קK���ƿ��?
		int idx_p2 = dist_parent(gen);
		const individual& parent1 = population[idx_p1];
		const individual& parent2 = population[idx_p2];
		// ��l�ƨ�Ӥl�N
		individual child1, child2;
		child1.solution.resize(dim);
		child2.solution.resize(dim);
		
		// SBX ��t�v�P�_
		if (dist_rate(gen) < CR) { // �i���t
			for (int d = 0; d < dim; ++d) {
				double x1 = parent1.solution[d];
				double x2 = parent2.solution[d];

				if (fabs(x1 - x2) < 1e-14) {
					// �Y��Ӥ��N�b�Ӻ��פW�۵��A�h�����ƻs
					child1.solution[d] = x1;
					child2.solution[d] = x2;
				}
				else {
					double y1 = min(x1, x2);
					double y2 = max(x1, x2);

					double rand_beta = dist_rate(gen);
					double beta; // ���G�Y��
					if (rand_beta <= 0.5) {
						beta = pow(2.0 * rand_beta, 1.0 / (eta_c + 1.0));				  // beta �|�p�� 1�]���V���ˤ����^
					}
					else {
						beta = pow(1.0 / (2.0 * (1.0 - rand_beta)), 1.0 / (eta_c + 1.0)); // beta �|�j�� 1�]���V���˥~���^
					}
					// ���ͨ�Ӥl�N
					child1.solution[d] = 0.5 * ((1 + beta) * x1 + (1 - beta) * x2);
					child2.solution[d] = 0.5 * ((1 - beta) * x1 + (1 + beta) * x2);

					// ��ɳB�z�A�Hproblem1���D
					double lower_bound1, upper_bound1;
					problem1.GetBounds(lower_bound1, upper_bound1);
					child1.solution[d] = max(lower_bound1, min(upper_bound1, child1.solution[d]));
					child2.solution[d] = max(lower_bound1, min(upper_bound1, child2.solution[d]));
				}
			}
		
		}
		else { // ���i���t�A�����ƻs
			child1.solution = parent1.solution;
			child2.solution = parent2.solution;
		}

		// ��l�Ƥl�N��L�ݩ�
		child1.rank = -1;
		child1.crowding_distance = 0.0;
		child1.objective1 = 0.0;
		child1.objective2 = 0.0;

		child2.rank = -1;
		child2.crowding_distance = 0.0;
		child2.objective1 = 0.0;
		child2.objective2 = 0.0;

		// �N�l�N�[�J offspring
		offspring.push_back(child1);
		offspring.push_back(child2);
	}
}

/* �ܲ� */
void NSGAII::Mutation(vector<individual>& offspring) {
	int N = offspring.size();
	double eta_m = 20.0; // �h�I�ܲ����G���� (�q�`�]�� 10~50)
	uniform_real_distribution<double> dist_rate(0, 1);
	// �o��W�U��
	double lower_bound1, upper_bound1;
	problem1.GetBounds(lower_bound1, upper_bound1);
	for (int i = 0; i < N; ++i) {
		for (int d = 0; d < dim; ++d) {
			if (dist_rate(gen) < MR) { // �i���ܲ�
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
				// ��s�ܲ��᪺�Ȩöi����ɳB�z
				y = y + deltaq * (upper_bound1 - lower_bound1);
				y = max(lower_bound1, min(upper_bound1, y));
				offspring[i].solution[d] = y;
			}
		}
	}
}

/* �����ؼЭ� objective1, objective2 */
void NSGAII::Evaluation(vector<individual>& population) {
	for (auto& ind : population) {
		ind.objective1 = problem1.Evaluate(ind.solution);
		ind.objective2 = problem2.Evaluate(ind.solution);
		nfes += 2; // �C�����@�ӭ���Anfes �W�[ 2�]�]������ӥؼШ�ơ^
	}
	// cout << "[Debug] Evaluation OK, nfes: " << nfes << endl;
}

/* ��ܤU�@�N���� */
void NSGAII::Determination() {
	// �إ� fronts, �x�s�U�e�u������, fronts[0] = �Ĥ@�e�u, fronts[1] = �ĤG�e�u, ...
	vector<vector<individual>> fronts; 
	for (const auto& ind : population) {
		if (ind.rank > fronts.size()) {
			fronts.resize(ind.rank); // �ʺA�վ� fronts �j�p
		}
		fronts[ind.rank - 1].push_back(ind); // ������J������ rank
	}

	// �M�w�s������
	vector<individual> new_population; // �s������
	int new_size = 0;				   // �s����p��, �ΨӽT�O�s���餣�W�L pop_size

	// �M��fronts���U�e�u
	for (auto& front : fronts) {
		if (new_size + front.size() <= pop_size) {
			// �p�G��ӫe�u�i�H�[�J�s����
			new_population.insert(new_population.end(), front.begin(), front.end());
			new_size += front.size();
		}
		else {
			// �p�G�e�u�L�k�����[�J�s����, �ݭn�ھھ����Z�����
			sort(front.begin(), front.end(), [](const individual& a, const individual& b) {
				return a.crowding_distance > b.crowding_distance; // �����Z���Ѥj��p�Ƨ�
				});
			int remain = pop_size - new_size; // �Ѿl�i�[�J�s���骺�W�B
			new_population.insert(new_population.end(), front.begin(), front.begin() + remain);
			new_size += remain;
			break; // �s����w��, �h�X
		}
	}
	population = new_population; // ��s����
}

#endif
