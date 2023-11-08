from PIL import Image
from pathlib import Path
import os
from tqdm import trange
os.chdir(Path(__file__).parent)

baseline_path = Path("baseline_route.gif")
our_path = Path("neena_route.gif")

baseline = Image.open(baseline_path)
our = Image.open(our_path)

print(baseline.size, baseline.n_frames)
print(our.size, our.n_frames)

combined_num_frames = max(baseline.n_frames, our.n_frames)
combined_size = (baseline.size[0] * 2, baseline.size[1])
combined_frames = []
for i in trange(combined_num_frames, desc="Combining"):
    frame = Image.new("RGB", combined_size)
    baseline.seek(min(i, baseline.n_frames - 1))
    our.seek(min(i, our.n_frames - 1))
    frame.paste(baseline, (0, 0))
    frame.paste(our, (baseline.size[0], 0))
    combined_frames.append(frame)

combined_frames[0].save("combined_route.gif", save_all=True, append_images=combined_frames[1:], duration=100, loop=0)
