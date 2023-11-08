#%%
import osmnx as ox
import geopandas as gpd
# minx, miny, maxx, maxy
AOI = [-83.1114568297594, 40.1429648490025, -83.04141757607977, 40.197140237852494] # Powell, OH
road_graph = ox.graph_from_bbox(AOI[3], AOI[1], AOI[2], AOI[0], network_type='drive', simplify=True, retain_all=False, truncate_by_edge=False, custom_filter=None)
ox.plot_graph(road_graph)



# %%
