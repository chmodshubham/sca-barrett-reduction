# Side Channel Attack on Single Word Barrett Reduction

Power side-channel analysis of single-word Barrett reduction, as used in [HQC](https://gitlab.com/pqc-hqc/hqc), on a ChipWhisperer Husky with the CW312T-SAM4S target. The board runs HQC-1's Barrett reduction, and the notebook `tvla_barrett.ipynb` runs a fixed-vs-random TVLA on it.

## Prerequisites

Set up ChipWhisperer `v6.1.0` with the [chipwhisperer-setup](https://github.com/chmodshubham/chipwhisperer-setup) guide.

```bash
# Install SciPy for the t-test and matplotlib for the plot images
conda activate cw
python -m pip install scipy matplotlib
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
# Open the flash and test notebook in the ChipWhisperer environment
conda activate cw
cd ~/chipwhisperer-setup/sca-barrett-reduction/simpleserial-barrett
jupyter notebook flash_and_test.ipynb
```

Run all cells. The test cell must print two equal numbers, `10726 10726` with the default `N`.

### 4. Run the TVLA

Stop the Jupyter server from step 3 with Ctrl+C, or open a new terminal.

```bash
# Open the TVLA notebook in the ChipWhisperer environment
conda activate cw
cd ~/chipwhisperer-setup/sca-barrett-reduction
jupyter notebook tvla_barrett.ipynb
```

Run all cells. A run takes about 5 minutes. The notebook saves its plot images in `figures/random`. The notebook explains each plot, and its "Saved files" section lists the files it writes and how to read them. [docs/barrett-instructions.md](docs/barrett-instructions.md) explains each board instruction.

`tvla_barrett_alternate.ipynb` runs the same test with fixed and random inputs alternating (fixed, random, fixed, ...) instead of in a random order. It saves only its plot images, in `figures/alternate`. The lines that save its traces are commented out in its Capture cell. Stop the Jupyter server above with Ctrl+C, or open a new terminal, then:

```bash
# Open the alternating-input TVLA notebook in the ChipWhisperer environment
conda activate cw
cd ~/chipwhisperer-setup/sca-barrett-reduction
jupyter notebook tvla_barrett_alternate.ipynb
```

## Configuration

The TVLA settings, such as `FIXED_X`, `N_TRACES` and `SAMPLES`, are in the first code cell of each TVLA notebook, each with a comment. The firmware settings are:

| Setting        | Where                                         | Default                                                                |
| -------------- | --------------------------------------------- | ---------------------------------------------------------------------- |
| `PARAM_N`      | `simpleserial-barrett/simpleserial-barrett.c` | `17669` (HQC-1)                                                        |
| `FIRMWAREPATH` | `make FIRMWAREPATH=<path>`                    | `../../chipwhisperer/firmware/mcu`, relative to `simpleserial-barrett` |

To change the modulus:

1. In `simpleserial-barrett/simpleserial-barrett.c`, set `PARAM_N`, and set `PARAM_N_MU` to `floor(2^32 / PARAM_N)`.
2. Set `N` to the same value in `simpleserial-barrett/flash_and_test.ipynb`, `tvla_barrett.ipynb` and `tvla_barrett_alternate.ipynb`.
3. Rebuild the firmware, then run the notebooks again:

   ```bash
   # Build the firmware with the new modulus
   cd ~/chipwhisperer-setup/sca-barrett-reduction/simpleserial-barrett
   make
   ```

## Cleanup

```bash
# Remove build files
cd ~/chipwhisperer-setup/sca-barrett-reduction/simpleserial-barrett
make clean

# Remove saved traces and Jupyter cache folders
cd ~/chipwhisperer-setup/sca-barrett-reduction
rm -f tvla_*_traces.npz tvla_*_traces.csv
rm -rf .ipynb_checkpoints simpleserial-barrett/.ipynb_checkpoints
```
