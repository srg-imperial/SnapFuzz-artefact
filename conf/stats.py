#!/usr/bin/env python3

"""Get stats from AFL runs.
"""

import decimal
import glob
import statistics
import sys

from collections import defaultdict, namedtuple
from datetime import datetime, timedelta
from pathlib import Path
from typing import List

FUZZER_STAT_FILES_GLOB = f"./aflout-*/fuzzer_stats"

SessionStep = namedtuple("SessionStep", "action duration")


ctx = decimal.Context()
ctx.prec = 20


def float_to_str(f):
    """
    Convert the given float to a string,
    without resorting to scientific notation
    """
    d1 = ctx.create_decimal(repr(f))
    return format(d1, "f")


def avg(lst):
    if len(lst) == 0:
        return 0
    return sum(lst) / len(lst)


def do_mma(mma, dur: str):
    duration = float(dur)
    if duration < mma[0]:
        mma[0] = duration
    if duration > mma[1]:
        mma[1] = duration
    mma[2].append(duration)


def finish_mma(mma):
    mma[0] = float_to_str(round(mma[0], 6))
    mma[1] = float_to_str(round(mma[1], 6))
    mma.append(len(mma[2]))
    mma[2] = float_to_str(round(avg(mma[2]), 6))


def remove_rel_path(path: str) -> str:
    if path.startswith("./"):
        return path[2:]


def get_target_name(line):
    line_tokens = line.split(" ")
    idx = line_tokens.index("-o")
    # Bin name should always be after -o <fuzz_dir>
    target_bin = line_tokens[idx + 2]

    target_bin = remove_rel_path(target_bin)

    return target_bin


def checks():
    stat_files = len(glob.glob(FUZZER_STAT_FILES_GLOB))
    output_files = len(glob.glob("./output-*.txt"))
    assert stat_files > 0
    assert output_files > 0
    assert output_files == stat_files
    print("++++++++++++++++++++++++++++ STATS ++++++++++++++++++++++++++++")
    print(f"Total stat files: {stat_files}")
    print(f"Total output files: {stat_files}")
    print("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++")


def gather_print_mode():
    # csv_file = open("print_stats.csv", mode="w")
    # writer = csv.writer(
    #     csv_file,
    #     delimiter=",",
    #     quotechar='"',
    #     quoting=csv.QUOTE_MINIMAL,
    # )
    # writer.writerow(
    #     [
    #         "label",
    #         "num_sessions",
    #         "tot_steps_ps",
    #         "conn_min",
    #         "conn_max",
    #         "conn_avg",
    #         "conn_cnt",
    #         "first_recv_min",
    #         "first_recv_max",
    #         "first_recv_avg",
    #         "first_recv_cnt",
    #         "sends_min",
    #         "sends_max",
    #         "sends_avg",
    #         "sends_cnt",
    #         "recvs_min",
    #         "recvs_max",
    #         "recvs_avg",
    #         "recvs_cnt",
    #         "kill_min",
    #         "kill_max",
    #         "kill_avg",
    #         "kill_cnt",
    #         "tot_min",
    #         "tot_max",
    #         "tot_avg",
    #         "tot_cnt",
    #     ]
    # )

    print_output_fnames = defaultdict(list)
    for fname in glob.glob(FUZZER_STAT_FILES_GLOB):
        with open(fname) as f:
            for line in f.readlines():
                val = line.split(":", 1)[-1][1:].rstrip()

                if line.startswith("command_line"):
                    afl_bin = val.split(" ")[0]
                    if "-print" not in afl_bin:
                        continue

                    afl_bin = remove_rel_path(afl_bin)

                    target_bin = get_target_name(val)

                    id = fname.split("/")[1].split("aflout-")[1]
                    ffname = f"./output-{id}.txt"
                    if not Path(ffname).exists():
                        print(f"Missing output file {ffname}")
                        exit(1)

                    label = afl_bin + " + " + target_bin
                    print_output_fnames[label].append(ffname)

    results = []
    for label, fnames in print_output_fnames.items():
        sessions: List[List[SessionStep]] = []

        for fname in fnames:
            with open(fname, "r", errors="ignore") as f:
                for line in f.readlines():
                    if "TASO" in line:
                        if "rm:" in line:
                            print(f"Warning('rm:'): This will fail: {fname}")

                        perf_nums = line.split(" ")[3:5]
                        if len(perf_nums) == 2:
                            action = perf_nums[0].replace(":", "")
                            duration = perf_nums[1].split("\x1b")[0]

                            if action == "Conn":
                                sessions.append([SessionStep(action, duration)])
                            else:
                                sessions[-1].append(SessionStep(action, duration))

                            # print(f"{label}: {action}, {duration}")

        # mma = Min, Max, Avg
        mma_conn = [float("inf"), 0.0, []]
        mma_first_recv = [float("inf"), 0.0, []]
        mma_recvs = [float("inf"), 0.0, []]
        mma_sends = [float("inf"), 0.0, []]
        mma_kill = [float("inf"), 0.0, []]
        mma_tot = [float("inf"), 0.0, []]

        for session in sessions:
            assert session[0].action == "Conn"
            assert session[-1].action == "Total"
            # assert session[-2].action == "Kill" # No if has crashed

            for i, step in enumerate(session):
                if step.action == "Conn":
                    do_mma(mma_conn, step.duration)
                elif step.action == "Total":
                    do_mma(mma_tot, step.duration)
                elif step.action == "Kill":
                    do_mma(mma_kill, step.duration)
                elif step.action == "Send":
                    do_mma(mma_sends, step.duration)
                elif step.action == "Recv" and i == 1:
                    do_mma(mma_first_recv, step.duration)
                elif step.action == "Recv":
                    do_mma(mma_recvs, step.duration)
                else:
                    assert False

        finish_mma(mma_conn)
        finish_mma(mma_first_recv)
        finish_mma(mma_recvs)
        finish_mma(mma_sends)
        finish_mma(mma_kill)
        finish_mma(mma_tot)

        num_sessions = len(sessions)
        tot_steps_ps = sum([len(s) for s in sessions])
        results.append(
            f"{label:55} {str(num_sessions):5} {str(tot_steps_ps):5} "
            f"{str(mma_conn):40} {str(mma_first_recv):40} {str(mma_sends):40} "
            f"{str(mma_recvs):40} {str(mma_kill):40} {str(mma_tot):40}"
        )
        # writer.writerow(
        #     [
        #         label,
        #         str(num_sessions),
        #         str(tot_steps_ps),
        #         str(mma_conn[0]),
        #         str(mma_conn[1]),
        #         str(mma_conn[2]),
        #         str(mma_conn[3]),
        #         str(mma_first_recv[0]),
        #         str(mma_first_recv[1]),
        #         str(mma_first_recv[2]),
        #         str(mma_first_recv[3]),
        #         str(mma_sends[0]),
        #         str(mma_sends[1]),
        #         str(mma_sends[2]),
        #         str(mma_sends[3]),
        #         str(mma_recvs[0]),
        #         str(mma_recvs[1]),
        #         str(mma_recvs[2]),
        #         str(mma_recvs[3]),
        #         str(mma_kill[0]),
        #         str(mma_kill[1]),
        #         str(mma_kill[2]),
        #         str(mma_kill[3]),
        #         str(mma_tot[0]),
        #         str(mma_tot[1]),
        #         str(mma_tot[2]),
        #         str(mma_tot[3]),
        #     ]
        # )

    results.sort()
    if results:
        print("++++++++++++++++++++++++++ Print Mode +++++++++++++++++++++++++")
        print("\n".join(results))

    # csv_file.close()


def gather_perf_mode():
    # csv_file = open("perf_stats.csv", mode="w")
    # writer = csv.writer(
    #     csv_file,
    #     delimiter=",",
    #     quotechar='"',
    #     quoting=csv.QUOTE_MINIMAL,
    # )
    # writer.writerow(
    #     [
    #         "label",
    #         "run_time",
    #         "execs_per_sec",
    #         "execs_done",
    #         "cycles_done",
    #         "stability",
    #         "unique_hangs",
    #         "unique_crashes",
    #         "paths_total",
    #         "max_depth",
    #         "variable_paths",
    #     ]
    # )

    results = []
    avg_total_time = []
    expected_execs_done = 0
    for fname in glob.glob(FUZZER_STAT_FILES_GLOB):
        with open(fname) as f:
            start_time: datetime = None
            last_update: datetime = None
            run_time: datetime = None
            nominal_execs_per_sec = 0.0
            stability = ""
            label = ""
            unique_crashes = ""
            unique_hangs = ""
            execs_done = ""
            cycles_done = ""

            paths_total = ""
            max_depth = ""
            variable_paths = ""

            for line in f.readlines():
                val: str = line.split(":", 1)[-1][1:].rstrip()

                if line.startswith("start_time"):
                    start_time = datetime.utcfromtimestamp(int(val))
                elif line.startswith("last_update"):
                    last_update = datetime.utcfromtimestamp(int(val))
                elif line.startswith("execs_per_sec"):
                    nominal_execs_per_sec = float(val)
                elif line.startswith("execs_done"):
                    execs_done = int(val)
                elif line.startswith("stability"):
                    stability = val[:-1]
                elif line.startswith("command_line"):
                    afl_bin = val.split(" ")[0]
                    afl_bin = remove_rel_path(afl_bin)

                    target_bin = get_target_name(val)

                    label = afl_bin + " + " + target_bin
                elif line.startswith("unique_crashes"):
                    unique_crashes = val
                elif line.startswith("unique_hangs"):
                    unique_hangs = val
                elif line.startswith("paths_total"):
                    paths_total = val
                elif line.startswith("max_depth"):
                    max_depth = val
                elif line.startswith("variable_paths"):
                    variable_paths = val
                elif line.startswith("cycles_done"):
                    cycles_done = val

            if "-print" in label:
                continue

            if execs_done > expected_execs_done:
                expected_execs_done = execs_done
            run_time = last_update - start_time
            avg_total_time.append(run_time.total_seconds())
            execs_per_sec = round(execs_done / (run_time.total_seconds()), 2)

            # nominal_print = f"({nominal_execs_per_sec})"
            # execs_print = f"{execs_per_sec:10} {nominal_print:10}"
            execs_done_str = f"({execs_done})"
            execs_print = f"{execs_per_sec:10} {execs_done_str:10}"
            hangs_crashes = f"{unique_hangs}|{unique_crashes}"
            aux = f"{paths_total}|{max_depth}|{variable_paths}"
            run_time = (
                str(run_time) if "day" in str(run_time) else "0 day, " + str(run_time)
            )
            results.append(
                f"{label:45} {run_time:15} {str(execs_print):20} {cycles_done:5} {str(stability):10} {hangs_crashes:10} {aux:20} {fname}"
            )
            # writer.writerow(
            #     [
            #         label,
            #         str(run_time),
            #         str(execs_per_sec),
            #         str(execs_done),
            #         str(cycles_done),
            #         str(stability),
            #         str(unique_hangs),
            #         str(unique_crashes),
            #         str(paths_total),
            #         str(max_depth),
            #         str(variable_paths),
            #     ]
            # )

    results.sort()
    if results:
        print("++++++++++++++++++++++++++ Perf Mode +++++++++++++++++++++++++")
        print("\n".join(results))
        mean = statistics.mean(avg_total_time)
        stdev = statistics.stdev(avg_total_time)
        print(
            f"Mean total time: {str(timedelta(seconds=mean)).split('.')[0]} ({mean//60:.0f} min)"
        )
        print(
            f"Stdev total time: {str(timedelta(seconds=stdev)).split('.')[0]} ({stdev//60:.0f} min)"
        )
        print(f"Iterations per second: {execs_done/mean:.2f}")

    # csv_file.close()


def main(argv):
    checks()
    gather_perf_mode()
    gather_print_mode()


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
