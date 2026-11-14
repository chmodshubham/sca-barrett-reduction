# Side Channel Attack on Single Word Barrett Reduction

Power side-channel analysis of single-word Barrett reduction, as used in [HQC](https://gitlab.com/pqc-hqc/hqc), on a ChipWhisperer Husky with the CW312T-SAM4S target.

## Prerequisites

Set up ChipWhisperer `v6.1.0` with the [chipwhisperer-setup](https://github.com/chmodshubham/chipwhisperer-setup) guide.

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
# Open the notebook in the ChipWhisperer environment
conda activate cw
cd ~/chipwhisperer-setup/sca-barrett-reduction/simpleserial-barrett
jupyter lab flash_and_test.ipynb
```

Run all cells. The test cell must print two equal numbers, `10726 10726` with the default `N`.

| Command | Data                                                 |
| ------- | ---------------------------------------------------- |
| `b`     | Computer to board: `x`, 3 bytes, little-endian       |
| `r`     | Board to computer: `x mod N`, 4 bytes, little-endian |

## Configuration

| Setting        | Where                      | Default                            |
| -------------- | -------------------------- | ---------------------------------- |
| `N` (modulus)  | `simpleserial-barrett.c`   | `17669` (HQC-1)                    |
| `FIRMWAREPATH` | `make FIRMWAREPATH=<path>` | `../../chipwhisperer/firmware/mcu` |

After changing `N` in `simpleserial-barrett.c`, set the same `N` in the notebook and rebuild:

```bash
# Rebuild the firmware
cd ~/chipwhisperer-setup/sca-barrett-reduction/simpleserial-barrett
make
```

Then run the notebook again.

## Cleanup

```bash
# Remove build files
cd ~/chipwhisperer-setup/sca-barrett-reduction/simpleserial-barrett
make clean
```
