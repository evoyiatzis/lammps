#!/usr/bin/env python
# fep.py - calculate free energy from compute fep results

from argparse import ArgumentParser
import math

def compute_fep():

    parser = ArgumentParser(description='A python script to calculate free energy from compute fep results')

    parser.add_argument("Temperature", type=float, help="The temperature of the system in Kelvin")
    parser.add_argument("InputFile", help="The name of the file with the output from compute fep.")

    args = parser.parse_args()

    rt = 0.008314 / 4.184 * args.Temperature # in kcal/mol

    v = 1.0
    sum = 0.0

    with open(args.InputFile, "r") as input_file:
        for line in input_file:
            if line.startswith("#"):
                continue
            tok = line.split()
            if len(tok) == 4:
                v = float(tok[3])
            sum += math.log(float(tok[2]) / v)

    print(-rt * sum)

if __name__ == "__main__":
    compute_fep()

