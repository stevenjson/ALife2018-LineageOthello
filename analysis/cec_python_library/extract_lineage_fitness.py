#!/home/emily/anaconda2/bin/python
###############################################################################
# Version: 1.1
# Last modified on: 3 April, 2016 
# Developers: Michael G. Epitropakis
#      email: m_(DOT)_epitropakis_(AT)_lancaster_(DOT)_ac_(DOT)_uk 
###############################################################################
from cec2013.cec2013 import *
import numpy as np
import sys
import pandas as pd
import glob
import os

def main():

	glob_pattern = sys.argv[1]
	outfilename = sys.argv[2]

	all_data = pd.DataFrame()

	# print(glob_pattern, glob.glob(glob_pattern))

	problem_map = {0:4, 1:5, 2:6, 6:12}

	for dirname in glob.glob(glob_pattern):
		print(dirname)
		run_log = dirname + "/run.log"
		filename = dirname + "/pop_5000/phylogeny_5000.csv"

		if not (os.path.exists(run_log) and os.path.exists(filename)):
			continue

		local_data = {}

		with open(run_log) as run_log_file:
			for line in run_log_file:
				if line.startswith("Doing initial"):
					break
				elif not line.startswith("set"):
					continue
				line = line.split()
				local_data[line[1]] = line[2]

		# Create function
		f = CEC2013(problem_map[int(local_data["PROBLEM"])])
		assert(f.get_dimension() == 2)			

		df = pd.read_csv(filename, index_col="id")

		df.rename(str.strip, axis="columns", inplace=True)
		# print(df.columns.values)
		genotypes = [i.strip("[] ").split() for i in df["info"]]
		genotypes = map(lambda x: [float(x[0]), float(x[1])], genotypes)
		df["fitness"] = [f.evaluate(np.array(x)) for x in genotypes]
		genotypes = zip(*genotypes)

		df["x"] = genotypes[0]
		df["y"] = genotypes[1]

		paths = []
		ids  = []

		next_gen = set([])
		alive = df[df["num_orgs"] > 0].index
		for i in range(1000):
			for curr in alive:
				if (df.loc[curr, "parent_id"]== 1):
					next_gen.add(curr)
				else:
					# print(curr, df.loc[curr, "parent_id"])
					next_gen.add(df.loc[curr, "parent_id"])
			alive = next_gen.copy()
			next_gen = set([])
			if len(alive) == 1:
				break

		for curr_id in alive:
			curr_id = int(curr_id)
			# print(curr_id)
			path = []
			while (curr_id > 1):
				# print(df.loc[curr_id, :])
				path.append(" ".join(["{:.2f}".format(i) for i in [df.loc[curr_id, "x"], df.loc[curr_id, "y"], df.loc[curr_id, "fitness"]]]))
				if (df.loc[curr_id, "parent_id"] == 1):
					break
				curr_id = df.loc[curr_id, "parent_id"]

			paths.append(",".join(path))
			ids.append(curr_id)

		temp = pd.DataFrame({"path": paths, "lin_id": ids})
		for k in local_data:
			temp[k] = local_data[k]

		all_data = pd.concat([all_data, temp])


	all_data.to_csv(outfilename,index=False)

	#print(df)
	# Evaluate :-)
	# x = np.ones(2)
	# value = f.evaluate(x)




if __name__ == "__main__":
	main()
