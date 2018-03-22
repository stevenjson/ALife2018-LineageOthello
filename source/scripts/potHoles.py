import argparse, os, errno

'''
potHoles.py
Given a set of job_not_done logs (example log contents below), rebuild qsub file to submit jobs. 

== Example contents of job_not_done log: ==
P0_G32_M0.0001_S1_E1_LE1e-09_1783/run.log
P0_G32_M0.0001_S1_E1_LE1e-12_1796/run.log
P0_G32_M0.0001_S5_E0_6431/run.log
P0_G32_M0.0001_S5_E0_6433/run.log
P0_G32_M0.0001_S5_E0_6437/run.log

'''

qsub_base = '''#!/bin/bash -login

### Configure job:
#PBS -l walltime=04:00:00:00
#PBS -l feature=intel16
#PBS -l mem=8gb
#PBS -t [[JOB_CONFIG:-t]]
#PBS -N POTHOLES

### load necessary modules, e.g.
module load powertools

# General Parameters.
EXEC=command.sh

'''

def main():
    parser = argparse.ArgumentParser(description="Pot hole filler.")
    parser.add_argument("-d", "--data_directory", type=str, action="append", help="Target experiment directory.")
    parser.add_argument("-l", "--not_done_log", type=str, action="append", help="Location of log of unfinished jobs.")
    parser.add_argument("-c", "--condition", type=str, action="append", help="At least one given condition string must be job name to include in resubs.")

    args = parser.parse_args()

    data_directories = args.data_directory
    not_done_logs = args.not_done_log
    filter_jobs = bool(args.condition)
    include_conditions = args.condition
    
    def JobFilter(job_name):
        if (not filter_jobs): return True
        for condition in include_conditions:
            if (condition in job_name):
                return True
        return False

    print("Data directories: {}".format(data_directories))
    print("Not done logs: {}".format(not_done_logs))

    if (filter_jobs):
        print("Filtering jobs on conditions: {}".format(include_conditions))

    if (not data_directories):
        print("Must provide at least one data directory!")
        exit()
    if (not not_done_logs):
        print("Must provide at least one log of unfinished jobs!")
        exit()
    
    unfinished_jobs = []
    # Aggregate unfinished jobs.
    for log in not_done_logs:
        with open(log, "r") as fp:
            unfinished_jobs += [line.split("/")[0] for line in fp.read().split("\n") if JobFilter(line)]

    print("TOTAL UNFINISHED JOBS (FROM LOGS): {}".format(len(unfinished_jobs)))

    # Find e'rybody. 
    job_info = {}
    for job in unfinished_jobs:
        run_dir = ""
        for data_dir in data_directories:
            candidate_dir = os.path.join(data_dir, job)
            if (os.path.isdir(candidate_dir)):
                # Found it!
                run_dir = candidate_dir
                break
        # Double check that the command.sh exists.
        if (run_dir == ""):
            print("WARNING: could not find run directory for {}".format(job))
            continue
        if (not os.path.exists(os.path.join(run_dir, "command.sh"))):
            print("WARNING: could not find command.sh in found run directory: {}".format(run_dir))
            continue

        job_info[job] = {}
        job_info[job]["run_dir"] = run_dir

    print("TOTAL JOBS ABLE TO RESUBMIT: {}".format(len(job_info)))

    qsub = qsub_base.replace("[[JOB_CONFIG:-t]]", "{}-{}".format(1,len(job_info)))

    # Build up a qsub file.
    # if [ ${PBS_ARRAYID} -eq 1 ]; then 
    array_id = 1
    for job in job_info:
        info = job_info[job]
        qsub += '''
if [ ${PBS_ARRAYID} -eq [[ARRAY_ID]] ]; then 
    RUN_DIR=[[RUN_DIR]]
fi
'''.replace("[[RUN_DIR]]", info["run_dir"]).replace("[[ARRAY_ID]]", str(array_id))
        array_id += 1
    
    qsub += "\n\n"
    qsub += "cd ${RUN_DIR}\n"
    qsub += "mv run.log bak_run.log\n"
    qsub += "./${EXEC}\n"

    with open("fill_jobholes.qsub", "w") as fp:
        fp.write(qsub)

if __name__ == "__main__":
    main()