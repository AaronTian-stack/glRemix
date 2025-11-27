import matplotlib.pyplot as plt

categories = [
    "Framerate Sync",
    "Double-buffered IPC",
    "O(n) CPU Transfer",
    "Merged IPC + Renderer Optimization"
]

# Data columns
original_app_debug = 1926.2
original_app_release = 1980.5

debug = [486.5, 502.7, 739.6, 2101.9]
release = [774.4, 855.5, 1390.8, 2409.7]

offset = 20

def annotate_line(x_vals, y_vals):
    for x, y in zip(x_vals, y_vals):
        plt.text(x, y + offset, f"{y}", fontsize=14, ha="center", va="bottom")


# Create figure
plt.figure(figsize=(10, 4))

# Three lines:
# 1. Original App (horizontal; value doesn't depend on category)
# plt.plot(categories, [original_app_debug]*len(categories),
#          marker='o', label="Original App (Debug Baseline)")

plt.plot(categories, [original_app_release]*len(categories),
         marker='o', label="Original GLXGears (OpenGL 1.0)")
plt.text(0, original_app_release + offset, f"{original_app_release}", fontsize=12, ha="center", va="bottom")

# 2. Debug
# plt.plot(categories, debug, marker='o', label="Debug")

# 3. Release
plt.plot(categories, release, marker='o', label="glRemix GLXGears (DX12)")
annotate_line(categories, release)

plt.xlabel("Optimization Step")
plt.ylabel("FPS")
plt.title("Performance Improvement from optimizing Interprocess Communication")
plt.legend()
plt.grid(True)
plt.tight_layout()

plt.savefig("docs/assets/performance_milestone2.png")

plt.show()