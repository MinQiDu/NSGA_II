import pandas as pd
import matplotlib.pyplot as plt

# 讀取 CSV
df = pd.read_csv("pareto_run1.csv")

# 計算個體數量
num_points = len(df)

# 畫 Pareto Front
plt.figure(figsize=(6, 6))
plt.xlim(0, 4)
plt.ylim(0, 4)
plt.scatter(df['f1'], df['f2'], c='#C2185B', marker='o', s=20, label=f'NSGA-II ({num_points} solutions)')
plt.xlabel("f₁", fontsize=16)
plt.ylabel("f₂", fontsize=16)
plt.title("SCH Pareto Front by NSGA-II", fontsize=15)
plt.grid(True)
plt.legend()
plt.tight_layout()
plt.savefig("pareto_run1.png", dpi=300)
plt.show()
