import sys

new_data = []

with open(sys.argv[1]) as infile:
    data = infile.readlines()
    assert(len(data)==1)

    data = data[0].split(",")
    for i in range(0, len(data), 3):
        new_data.append(",".join(data[i:i+3]))

print("\n".join(new_data))