import time
import matplotlib.pyplot as plt


def run_benchmark(gutenberg_title, user, func, num_runs=10):

    # Get the text
    with open("./text_sources/{}.txt".format(gutenberg_title), "r") as f:
        text = f.read()
        print("Read {} bytes from {}".format(len(text), gutenberg_title))
        
if __name__ == "__main__":
    # Run the benchmark
    run_benchmark("kjv_bible", "ndp", "wordcount")