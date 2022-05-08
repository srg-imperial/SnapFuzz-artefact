# SnapFuzz Artefact

## IMPORTANT

- All commands should be run from inside the root directory that this `README.md` file exists!
- We highly suggest using `screen` for all experiment runs.
- Don't run scripts in parallel. This artifact is explicitly serial.
- We expect some variations on performance numbers and we remind the reader that our performance numbers were obtained on a bare-metal machine with a `3.0 GHz AMD EPYC 7302P 16-Core CPU` and `128 GB RAM`, running `64-bit Ubuntu 18.04 LTS (kernel version 4.15.0-162)` with an `SSD disk`. If the experiments are to be conducted inside a container or a virtual machine, performance variations are expected.

<!-- - Benchmark data from our own runs are under `gold-{orig,snapfuzz-1m,snapfuzz-24h}` directories. -->

## Setup

1. Run `./conf/config.sh`
1. Run `./conf/build.sh`

## Run AFLNet baseline experiments

To generate the baseline data we need to run each experiment seperately:

1. `./conf/run-orig.sh dicom`
1. `./conf/run-orig.sh dns`
1. `./conf/run-orig.sh dtls`
1. `./conf/run-orig.sh ftp`
1. `./conf/run-orig.sh rtsp`

After the above runs are done, you should be able to see new directores under the name schema of `results-orig-{project name}-{date}`.

You can `cd` in each of the directories and see the stats with:

1. `../conf/stats.py`

An example output is provided in the following picture. The only important information is the `Avg total time` line which provides explicit information on the average time required for one fuzzing campaign to execute. By collecting each average total time for each project and compairing them then with SnapFuzz, we can conclude on the total speedups reported in our paper.

![Example output of stats.py](./imgs/orig-out.png)

## Run 1 milion iterations SnapFuzz experiments

To generate the SnapFuzz data we follow a similar strategy as above:

1. `./conf/run-snapfuzz.sh dicom`
1. `./conf/run-snapfuzz.sh dns`
1. `./conf/run-snapfuzz.sh dtls`
1. `./conf/run-snapfuzz.sh ftp`
1. `./conf/run-snapfuzz.sh rtsp`

After the above runs are done, you should be able to see new directores under the name schema of `results-snapfuzz-{project_name}-{date}`. You can `cd` in each of the directories and see the stats with:

1. `../conf/stats.py`

## Run 24h SnapFuzz experiments

To generate the SnapFuzz data we follow a similar strategy as above:

1. `./conf/run-snapfuzz.sh -l dicom`
1. `./conf/run-snapfuzz.sh -l dns`
1. `./conf/run-snapfuzz.sh -l dtls`
1. `./conf/run-snapfuzz.sh -l ftp`
1. `./conf/run-snapfuzz.sh -l rtsp`

After the above runs are done, you should be able to see new directores under the name schema of `results-snapfuzz-{project_name}-{date}`. You can `cd` in each of the directories and see the stats with:

1. `../conf/stats.py`

**NOTE: The 24h results don't have any special directory indicator from the standard SnapFuzz results.**

<!-- ## Find new bugs

Group the 24h experiments together under a directory called `snapfuzz-24h`.

Run the following inside the AFLNet baseline experiment directories and the SnapFuzz 24h experiment directories:

1. `mkdir -p all_bugs && find . -name "id*" | grep "/replayable-crashes" | xargs -I{} cp --backup=numbered {} ./all_bugs && ls -1 ./all_bugs | wc -l`
    1. e.g. `cd ./gold-orig/dicom` and then the above command.
1. You should now have an `all_bugs` directory in each AFLNet experiment result and in each SnapFuzz experiment results.
1. Return back to the main `artefact` directory.
1. For each `all_bugs` directory you have, run: `./conf/autofind_bugs.sh -t <orig or snapfuzz> -e <any of the experiments above> <path_to_all_bugs_dir>`.
    1. In this step, our script will go through all buggy inputs and will record crashes under ASan for each benchmark suite. -->
