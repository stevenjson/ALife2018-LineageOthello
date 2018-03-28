import argparse, os, errno, copy

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
#PBS -l mem=8gb
#PBS -t [[JOB_CONFIG:-t]]
#PBS -N POTHOLES

### load necessary modules, e.g.
module load powertools

# General Parameters.
EXEC=command.sh

'''

def main():
    parser = argparse.ArgumentParser(description="Pot hole filler. MUST have at least 1 not_done log or run_list.")
    parser.add_argument("-d", "--data_directory", type=str, action="append", help="Target experiment directory.")
    parser.add_argument("-c", "--condition", type=str, action="append", help="At least one given condition string must be job name to include in resubs.")
    parser.add_argument("-r", "--run_list", type=str, action="append", help="Location of run_list that describes original experiment conditions.")
    parser.add_argument("-dest", "--destination", type=str, help="Where to put missing runs")
    parser.add_argument("-config", "--configs", type=str, help="Where to find config directory")
    parser.add_argument("-u", "--update", type=str, help="Update that indicates run finished (final update)")

    args = parser.parse_args()

    data_directories = args.data_directory
    filter_jobs = bool(args.condition)
    include_conditions = args.condition
    run_lists = args.run_list
    config_dir = args.configs
    dest_dir = args.destination
    final_update = args.update
    
    def JobFilter(job_name):
        if (not filter_jobs): return True
        for condition in include_conditions:
            if (condition in job_name):
                return True
        return False

    print("Data directories: {}".format(data_directories))
    # print("Not done logs: {}".format(not_done_logs))

    if (filter_jobs):
        print("Filtering jobs on conditions: {}".format(include_conditions))
    if (not data_directories):
        print("Must provide at least one data directory!")
        exit()
    if (not run_lists):
        print("Must provide at least one run_list file!")
        exit()

    if (run_lists):
        if (not config_dir or not dest_dir):
            print("Require a config and destination location!")
            exit()
        print("Taking a peek at those run lists.")
        # 1) Extract listing of all runs. 
        all_runs = []
        for run_list in run_lists:
            rl_content = None
            with open(run_list, "r") as fp:
                rl_content = fp.read()
            # Collect all lines that describe treatments.
            run_lines = [line for line in rl_content.split("\n") if "./toy_problems" in line]
            # Expand the run lines into individual replicates.
            for run_line in run_lines:
                [start,finish] = map(int, run_line.split(" ")[0].split(".."))
                run_desc = " ".join(run_line.split(" ")[1:])
                all_runs += [run_desc.replace("$seed", str(i)) for i in range(start, finish+1)]
        print("Total number of runs: " + str(len(all_runs)))
        # 2) Try to find run
        missing_runs = {}
        old_run_log = []
        for run in all_runs:
            run_name = run.split(" ")[0]
            run_dir = "{}_{}".format(run_name, run.split(" ")[3])
            if not JobFilter(run_dir): continue
            found = False
            data_directory = None
            for data_dir in data_directories:
                found = os.path.isdir(os.path.join(data_dir, run_dir))
                if (found):
                    data_directory = data_dir
                    break
            print("{}: {}".format(run_dir, found))
            # If we can find it, did it finish?
            if (found):
                contents = None
                with open(os.path.join(data_directory, run_dir, "run.log"), "r") as fp:
                    contents = fp.read()
                if not "Update: {} Max score:".format(final_update) in contents:
                    found = False
                    print("  --> Unfinished!")
            # If we can't find it, add to missing runs dictionary.
            if (not found):
                missing_runs[run_dir] = {}
                missing_runs[run_dir]["command"] = " ".join(run.split(" ")[1:])
                missing_runs[run_dir]["run_dir"] = os.path.join("${DEST_DIR}", run_dir)
                old_run_log.append(os.path.join(data_directory, run_dir))
        
        print("Total number of missing runs: " + str(len(missing_runs)))

        with open("missing_runs.log", "w") as fp:
            fp.write("\n".join(old_run_log))
        
        # 3) Build Qsub file. 
        qsub = qsub_base.replace("[[JOB_CONFIG:-t]]", "{}-{}".format(1,len(missing_runs)))
        qsub += "\n"
        qsub += "DEST_DIR={}\n".format(dest_dir.rstrip("/"))
        qsub += "CONFIG_DIR={}\n".format(config_dir.rstrip("/"))
        array_id = 1
        for run in missing_runs:
            info = missing_runs[run]
            qsub += '''
if [ ${PBS_ARRAYID} -eq [[ARRAY_ID]] ]; then 
    RUN_DIR=[[RUN_DIR]]
'''.replace("[[RUN_DIR]]", info["run_dir"]).replace("[[ARRAY_ID]]", str(array_id))
            
            qsub += "    mkdir -p ${RUN_DIR}\n"
            qsub += "    cd ${RUN_DIR}\n"
            qsub += "    cp -R ${CONFIG_DIR}/* .\n"
            qsub += "    {} > run.log\n".format(info["command"])
            qsub += "fi\n\n"
            array_id += 1

        with open("missing_runs.qsub", "w") as fp:
            fp.write(qsub)
            

if __name__ == "__main__":
    main()