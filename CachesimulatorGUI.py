import streamlit as st
import pandas as pd
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D  # noqa: F401 unused import
import re

st.set_page_config(layout="wide")
st.title("📊 Cache Simulation Visualizer with 3D Comparison")

def parse_log_file(lines):
    grouped = []
    config = None
    current_block = []
    reading_config = False
    temp_config = {}

    for line in lines:
        line = line.strip()
        if line == "# Config:":
            if config and current_block:
                grouped.append((config, list(current_block)))
                current_block.clear()
            reading_config = True
            temp_config = {}
            continue
        if reading_config:
            if line.startswith("# Cache Size:"):
                temp_config["Cache Size"] = int(line.split(":")[1].strip())
            elif line.startswith("# Block Size:"):
                temp_config["Block Size"] = int(line.split(":")[1].strip())
            elif line.startswith("# Associativity:"):
                temp_config["Associativity"] = int(line.split(":")[1].strip())
            elif line.startswith("# Replacement Policy:"):
                temp_config["Replacement Policy"] = line.split(":")[1].strip()
            elif line.startswith("# Write Policy:"):
                temp_config["Write Policy"] = line.split(":")[1].strip()
            elif line == "" or not line.startswith("#"):
                config = temp_config
                reading_config = False
                if line != "":
                    current_block.append(line)
            continue
        if line != "" and not line.startswith("#"):
            current_block.append(line)
    if config and current_block:
        grouped.append((config, current_block))
    return grouped

file = st.file_uploader("Upload a cache simulation log file", type="txt")

if file:
    lines = file.readlines()
    lines = [line.decode("utf-8") if isinstance(line, bytes) else line for line in lines]
    grouped_logs = parse_log_file(lines)

    if len(grouped_logs) == 0:
        st.error("No configurations found in the uploaded file. Please check the file format.")
    else:
        configs_labels = [
            f"Assoc={cfg['Associativity']}, Repl={cfg['Replacement Policy']}, Write={cfg['Write Policy']}"
            for cfg, _ in grouped_logs
        ]
        selected_index = st.selectbox(
            "Select Configuration to View", range(len(grouped_logs)), format_func=lambda i: configs_labels[i]
        )

        config, entries = grouped_logs[selected_index]
        st.subheader("Selected Configuration")
        st.json(config)

        hit_count = sum(1 for e in entries if "Hit" in e)
        miss_count = len(entries) - hit_count

        st.metric("Cache Hits", hit_count)
        st.metric("Cache Misses", miss_count)

        st.text("\n".join(entries))

        # Create DataFrame for 3D visualization
        summary_data = []
        for cfg, block in grouped_logs:
            hits = sum(1 for e in block if "Hit" in e)
            total = len(block)
            summary_data.append({
                "Associativity": cfg["Associativity"],
                "Replacement Policy": cfg["Replacement Policy"],
                "Write Policy": cfg["Write Policy"],
                "Hit Rate": hits / total if total > 0 else 0
            })

        df_summary = pd.DataFrame(summary_data)

        # 🟢 Write Policy filter
        write_policy_options = ["All", "WT", "WB"]
        selected_write_policy = st.selectbox("Filter by Write Policy", write_policy_options)

        if selected_write_policy != "All":
            df_summary = df_summary[df_summary["Write Policy"] == selected_write_policy]

        if df_summary.empty:
            st.warning(f"No data available for Write Policy = '{selected_write_policy}'")
        else:
            st.subheader("🧠 3D Chart: Avg Hit Rate vs Associativity by Replacement Policy")
            fig = plt.figure(figsize=(4, 3))
            ax = fig.add_subplot(111, projection='3d')

            repl_policies = list(df_summary["Replacement Policy"].unique())
            y_pos_map = {name: i for i, name in enumerate(repl_policies)}

            for repl in repl_policies:
                subset = df_summary[df_summary["Replacement Policy"] == repl]
                xs = subset["Associativity"].values
                ys = [y_pos_map[repl]] * len(subset)
                zs = [0] * len(subset)
                dx = 0.4
                dy = 0.4
                dz = subset["Hit Rate"].values

                ax.bar3d(xs, ys, zs, dx, dy, dz, label=repl)

            ax.set_xlabel("Associativity")
            ax.set_ylabel("Replacement Policy")
            ax.set_yticks(list(y_pos_map.values()))
            ax.set_yticklabels(list(y_pos_map.keys()))
            ax.set_zlabel("Hit Rate")
            ax.set_title(f"3D Bar Chart: Hit Rate by Assoc and Repl Policy\n(Write Policy = {selected_write_policy})")
            ax.legend()
            st.pyplot(fig)
else:
    st.info("Please upload a cache simulation `.txt` file to begin.")
