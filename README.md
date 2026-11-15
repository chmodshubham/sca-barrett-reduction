# Side Channel Attack on Single Word Barrett Reduction

Power side-channel analysis of single-word Barrett reduction, as used in [HQC](https://gitlab.com/pqc-hqc/hqc), on a ChipWhisperer Husky with the CW312T-SAM4S target.

## Prerequisites

Set up ChipWhisperer `v6.1.0` with the [chipwhisperer-setup](https://github.com/chmodshubham/chipwhisperer-setup) guide.

```bash
# Install scipy for the TVLA t-test
conda activate cw
python -m pip install scipy
```

## Deployment

The commands use `~/chipwhisperer-setup`. Replace it with the path of your chipwhisperer-setup clone.

### 1. Clone

```bash
# Clone next to the ChipWhisperer clone
cd ~/chipwhisperer-setup
git clone https://github.com/chmodshubham/sca-barrett-reduction.git
```

### 2. Build

```bash
# Build simpleserial-barrett-CWHUSKY.hex
cd ~/chipwhisperer-setup/sca-barrett-reduction/simpleserial-barrett
make
```

### 3. Flash and test

```bash
conda activate cw
cd ~/chipwhisperer-setup/sca-barrett-reduction/simpleserial-barrett
jupyter notebook flash_and_test.ipynb
```

Run all cells. The test cell must print two equal numbers, `10726 10726` with the default `N`.

| Command | Data                                                 |
| ------- | ---------------------------------------------------- |
| `b`     | Computer to board: `x`, 3 bytes, little-endian       |
| `r`     | Board to computer: `x mod N`, 4 bytes, little-endian |

### 4. Run the TVLA

Stop the Jupyter server from step 3 with Ctrl+C, or open a new terminal.

```bash
# Open the TVLA notebook in the ChipWhisperer environment
conda activate cw
cd ~/chipwhisperer-setup/sca-barrett-reduction
jupyter notebook tvla_barrett.ipynb
```

Run all cells. The notebook captures 1000 warm-up traces, then 10000 test traces, which takes about 4 minutes. It then saves the traces. See step 5.

The plots show:

- the warm-up power
- the mean of each group and the difference of the means
- the Welch t-test on all traces and on their two halves, with the 4.5 bounds
- the t-test against clock cycles, for the whole recording or the range set by `ZOOM_START` and `ZOOM_END`

A sample leaks when |t| is above 4.5 in both halves, with the same sign. The notebook also prints the board instructions. [docs/barrett-instructions.md](docs/barrett-instructions.md) explains what each one does.

### 5. Read the saved files

Each notebook run saves the same data in two files in `~/chipwhisperer-setup/sca-barrett-reduction`. The notebook prints both names.

| File | Use |
| --- | --- |
| `tvla_<date>_<time>_traces.npz` | for Python, loads fast |
| `tvla_<date>_<time>_traces.csv` | a table for a spreadsheet, a text editor or pandas |

The `.npz` file holds three arrays:

| Key | Content |
| --- | --- |
| `traces` | one row per trace, with `SAMPLES` power readings each |
| `inputs` | the input `x` of each trace, in capture order |
| `groups` | the group of each trace: 0 is fixed, 1 is random |

Row `i` of `traces` is the trace recorded for `inputs[i]`, which is in group `groups[i]`.

The `.csv` file has one row per trace, in capture order:

| Column | Content |
| --- | --- |
| `input` | the input `x` of the trace |
| `group` | the group of the trace: 0 is fixed, 1 is random |
| `sample_0`, `sample_1`, ... | the power readings of the trace, one column per sample |

```bash
# Open Python in the ChipWhisperer environment
conda activate cw
cd ~/chipwhisperer-setup/sca-barrett-reduction
python
```

```python
# Load the .npz file, using the file name the notebook printed
import numpy as np
data = np.load("tvla_20261115_170000_traces.npz")
traces = data["traces"]
inputs = data["inputs"]
groups = data["groups"]

# Print the first trace with its input and group
print(traces[0], inputs[0], groups[0])

# Load the same data as a table from the .csv file
import pandas as pd
table = pd.read_csv("tvla_20261115_170000_traces.csv")
print(table.head())
```

## Configuration

| Setting        | Where                                         | Default                                                                |
| -------------- | --------------------------------------------- | ---------------------------------------------------------------------- |
| `PARAM_N`      | `simpleserial-barrett/simpleserial-barrett.c` | `17669` (HQC-1)                                                        |
| `FIRMWAREPATH` | `make FIRMWAREPATH=<path>`                    | `../../chipwhisperer/firmware/mcu`, relative to `simpleserial-barrett` |
| `FIXED_X`      | `tvla_barrett.ipynb`                          | `1212218` (input of the fixed group)                                   |
| `N_WARMUP`     | `tvla_barrett.ipynb`                          | `1000` (traces thrown away)                                            |
| `N_TRACES`     | `tvla_barrett.ipynb`                          | `10000` (test traces, fixed and random together)                       |
| `SAMPLES`      | `tvla_barrett.ipynb`                          | `1000` (samples per trace)                                             |
| `SEED`         | `tvla_barrett.ipynb`                          | `1` (repeatable inputs)                                                |
| `ZOOM_START`   | `tvla_barrett.ipynb`                          | `None` (whole recording)                                               |
| `ZOOM_END`     | `tvla_barrett.ipynb`                          | `None` (whole recording)                                               |

`ZOOM_START` and `ZOOM_END` are the first and last clock cycle in the cycle plot. Cycles count from trigger low, which is cycle 0, so earlier cycles are negative, for example `-60` and `20`.

To change the modulus:

1. In `simpleserial-barrett/simpleserial-barrett.c`, set `PARAM_N`, and set `PARAM_N_MU` to floor(2^32 / `PARAM_N`).
2. Set `N` to the same value in `simpleserial-barrett/flash_and_test.ipynb` and `tvla_barrett.ipynb`.
3. Rebuild the firmware:

```bash
# Rebuild the firmware
cd ~/chipwhisperer-setup/sca-barrett-reduction/simpleserial-barrett
make
```

Then run the notebooks again.

## Cleanup

```bash
# Remove build files
cd ~/chipwhisperer-setup/sca-barrett-reduction/simpleserial-barrett
make clean

# Remove saved traces
cd ~/chipwhisperer-setup/sca-barrett-reduction
rm -f tvla_*_traces.npz tvla_*_traces.csv

# Remove Jupyter cache folders
rm -rf .ipynb_checkpoints simpleserial-barrett/.ipynb_checkpoints
```
