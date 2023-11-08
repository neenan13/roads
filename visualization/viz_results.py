import geopandas as gpd
import pandas as pd
import numpy as np
import networkx as nx
from pathlib import Path
import re

import matplotlib.pyplot as plt
import folium


def read_route(route_path):
    route_pattern = re.compile("ID: (\d+) --- (\d+)->(\d+)")
    route_list = []
    for l in Path(route_path).read_text().splitlines():
        m = route_pattern.match(l)
        if m is None:
            continue
        edge_id = int(m.group(1)) - 1
        s = int(m.group(2))
        t = int(m.group(3))
        # if s != edge_gdf.loc[edge_id].s or t != edge_gdf.loc[edge_id].t:
        #   print(f"Warning: edge{edge_id} and route not match: {s}!={edge_gdf.loc[edge_id].s} or {t}!={edge_gdf.loc[edge_id].t}")
        route_list.append(edge_id)
    return route_list


def _main():
    current_dir = Path(__file__).parent

    NODE_GEO_PATH = current_dir / "neena_nodes.geojson"
    EDGE_GEO_PATH = current_dir / "neena_edges.geojson"
    EDGE_ATTR_PATH = current_dir / "neena_linked_table.txt"
    ROUTE_PATH = current_dir / "neena_route.txt"
    BASELINE_PATH = current_dir / "baseline_route.txt"
    for _p in [NODE_GEO_PATH, EDGE_GEO_PATH, EDGE_ATTR_PATH, ROUTE_PATH, BASELINE_PATH]:
        if not _p.is_file():
            print(f"File {_p} not found")
            return -1

    # Read Node List and Edge List
    node_gdf = gpd.read_file(NODE_GEO_PATH)
    node_gdf.set_index("id", inplace=True)
    edge_gdf = gpd.read_file(EDGE_GEO_PATH)
    edge_gdf.id = edge_gdf.id - 1  # transform to starts from 0
    edge_gdf["EID"] = edge_gdf.id  # copy id to field EID
    edge_gdf.set_index("id", inplace=True)

    edge_df = pd.read_csv(EDGE_ATTR_PATH, index_col=None, header=None, delim_whitespace=True)
    edge_df = edge_df.rename(columns={0: "s", 1: "t", 2: "length", 3: "priority", 4: "weight"})
    edge_df.s = edge_df.s.astype(np.int64)
    edge_df.t = edge_df.t.astype(np.int64)
    edge_gdf = edge_gdf.join(edge_df, on="id", how="left", lsuffix="_left", rsuffix="_right")

    neena_route = read_route(ROUTE_PATH)
    baseline_route = read_route(BASELINE_PATH)
    print(f"Load {len(neena_route)} routes (neena)")
    print(f"Load {len(baseline_route)} routes (baseline)")

    # Analysis 1
    # Compare the total traveling distance between baseline and our route.
    baseline_total_distance = np.sum([edge_gdf.loc[eid, "length"] for eid in baseline_route])
    neena_total_distance = np.sum([edge_gdf.loc[eid, "length"] for eid in neena_route])
    print(f"Baseline route total distance: {baseline_total_distance} miles")
    print(f"Our route total distance: {neena_total_distance} miles")
    print("Saved {:.2f}%".format((baseline_total_distance - neena_total_distance) / baseline_total_distance * 100))

    from itertools import repeat

    baseline_visits = dict(zip(edge_gdf.index, repeat(0)))
    neena_visits = dict(zip(edge_gdf.index, repeat(0)))

    baseline_visits.update(dict(zip(*np.unique(baseline_route, return_counts=True))))
    neena_visits.update(dict(zip(*np.unique(neena_route, return_counts=True))))

    edge_gdf["Baseline_visits"] = baseline_visits
    edge_gdf["Our_visits"] = neena_visits

    m = None
    edge_max_count = max(edge_gdf["Baseline_visits"].max(), edge_gdf["Our_visits"].max())
    m = edge_gdf.explore(
        m=m,
        name="Baseline_visits",
        tooltip=["Baseline_visits", "Our_visits"],
        column="Baseline_visits",
        cmap="viridis",
        vmax=edge_max_count,
        legend=True,
        style_kwds={"weight": 10, "alpha": 0.9},
    )
    m = edge_gdf.explore(
        m=m,
        name="Our visits",
        tooltip=["Baseline_visits", "Our_visits"],
        column="Our_visits",
        cmap="viridis",
        vmax=edge_max_count,
        legend=True,
        style_kwds={"weight": 10, "alpha": 0.9},
    )
    folium.LayerControl(collapsed=False).add_to(m)
    m.save(current_dir / "vis_results.html")
    print("Visualization results saved to vis_results.html")
    return 0


if __name__ == "__main__":
    import sys

    sys.exit(_main())
