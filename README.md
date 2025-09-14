# NSGA-II Implementation for Bi-objective Optimization (MOP)

## ( I ) Introduction

- **Programming language:** C++
- **Metaheuristic algorithm:**  
  NSGA-II (Fast Elitist Multiobjective Genetic Algorithm)  
- **Benchmark functions:**  
  Bi-objective combination of Sphere and Shifted Sphere  
  (f1: ∑xᵢ², f2: ∑(xᵢ−2)²)
- **Visualization:** Python (matplotlib)  
- **Configurable parameters:** run, dimension, population size, function ID, crossover rate (CR), mutation rate (MR)

---

## ( II ) Main Functionality

### `NSGAII`
- `void RunALG(int run, int func_id, int mnfes, int dim, int pop_size, double CR, double MR)`  
  *Main entry point to run NSGA-II*

- `void Init()`  
  *Initialize random population within lower/upper bounds*

- `void Evaluation(vector<individual>& population)`  
  *Evaluate objective1 and objective2 via `Problem` interface*

- `void Crossover()`  
  *Simulated Binary Crossover (SBX), with configurable CR*

- `void Mutation()`  
  *Polynomial Mutation, with configurable MR*

- `void FastNonDominatedSort()`  
  *Compute all fronts based on Pareto dominance*

- `void CrowdingDistanceAssignment()`  
  *Assign crowding distance to individuals in each front*

- `void Determination()`  
  *Select new population from combined parent + offspring*

- `void nsgaii_fileoutput()`  
  *Export Pareto front objectives to `.csv` for all runs*

---

## ( III ) Input

### Command-line arguments:
```bash
NSGAII.exe {run} {func_id} {mnfes} {dim} {pop_size} {CR} {MR}
```

- `run`         : Number of independent runs (e.g., 5)  
- `func_id`     : Objective function set (default: 1 for [f1, f2])  
- `mnfes`       : Maximum number of fitness evaluations  
- `dim`         : Number of dimensions  
- `pop_size`    : Population size  
- `CR`          : Crossover rate (e.g., 0.9)  
- `MR`          : Mutation rate (e.g., 0.1)  

---

## ( IV ) Output

- `temp_results/pareto_run{r}.csv`  
  - Each CSV contains one run’s final first-front (f1, f2)

- Example content:
  ```
  f1,f2
  3.981,0.0000001
  0.003,3.999
  ...
  ```

---

## ( V ) How to Compile & Run

### 🛠 Compile
#### 方法 1：MSYS2
```bash
g++ main.cpp -o run.exe -std=c++11
```

#### 方法 2：Visual Studio
按下 `Ctrl + Shift + B`

---

### Run
```bash
./run.exe 1 1 25000 1 100 0.9 0.1
```

這會執行 1 次 NSGA-II，維度為 1，產生 `temp_results/pareto_run{1}.csv`

---

## ( VI ) Visualization

### 畫 Pareto Front (使用 Python)

#### Python 繪圖檔：`plot_pareto_{FunctionName}.py`
```python
import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv("pareto_run1.csv")

plt.figure(figsize=(6, 6))
plt.xlim(0, 4)
plt.ylim(0, 4)
plt.scatter(df['f1'], df['f2'], c='#4682B4', s=20)
plt.xlabel("f₁")
plt.ylabel("f₂")
plt.title("Pareto Front")
plt.grid(True)
plt.savefig("pareto_run1.png", dpi=300)
plt.show()
```

#### ✅ 如何跑
```bash
條件: 已存在 pareto_run1.csv
python plot_pareto_{FunctionName}.py
```

你會看到：
- 結果圖片：`pareto_run1.png`
- 點的分布為 f1-f2 的 Pareto 前沿

---

## ( VII ) File Structure

```
NSGAII_BiObjective/
├── main.cpp                  ← 主函式入口
├── nsgaii.h                  ← NSGA-II 主邏輯
├── nsgaii_fileoutput.h       ← 輸出 Pareto 前沿 CSV
├── problem.h                 ← 多目標函數定義 (f1, f2)
├── nsgaii_plot
│   └── plot_pareto_{FunctionName}.py ← Python 畫圖工具
├── temp_results/            ← 每個 run 的 Pareto Front CSV
│   ├── pareto_run1.csv
│   ├── pareto_run2.csv
│   └── ...
└── README.md                ← 使用說明文件
```

---

## ( VIII ) Sample Result

<p align="center">
  <img src="temp_results/pareto_run1.png" width="400px"/>
</p>

---

## ( IX ) How to Extend

You can extend this implementation by:
- Adding more objective functions in `problem.h`
- Supporting 3 or more objectives (`f3`, `f4`...)
- Adding metrics like IGD, HV, etc.
- Using gnuplot to compare multiple runs

---

## ( X ) References

NSGA-II:   
[K. Deb, A. Pratap, S. Agarwal, and T. Meyarivan, "A fast and elitist multiobjective genetic algorithm: NSGA-II," _IEEE Transactions on Evolutionary Computation_, vol. 6, no. 2, pp. 182-197, 2002.](http://ieeexplore.ieee.org/document/996017/)