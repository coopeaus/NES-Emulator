import os
import filecmp

# Paths to the two directories
smaller_dir = "/Users/bgevko/Projects/nes-emu2/lib/sdl2/mac/SDL2.framework/Headers/SDL2"
larger_dir = "/Users/bgevko/Projects/nes-emu2/lib/sdl2/include/SDL2"

# Get all files in the smaller directory
smaller_files = [
    f for f in os.listdir(smaller_dir) if os.path.isfile(os.path.join(smaller_dir, f))
]

# Compare each file in the smaller directory with the corresponding file in the larger directory
differences = []
for file_name in smaller_files:
    file_path_smaller = os.path.join(smaller_dir, file_name)
    file_path_larger = os.path.join(larger_dir, file_name)

    if os.path.exists(file_path_larger):
        # Compare files
        if not filecmp.cmp(file_path_smaller, file_path_larger, shallow=False):
            differences.append(file_name)
    else:
        differences.append(f"{file_name} is missing in {larger_dir}")

# Print the results
if differences:
    print("Differences found:")
    for diff in differences:
        print(diff)
else:
    print("All files are identical between the two directories.")
