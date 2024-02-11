import time
import os

def run_benchmark(gutenberg_title, user, func, num_runs=10):

    # Get the text
    with open("./text_sources/{}.txt".format(gutenberg_title), "r") as f:
        text = f.read()
        
        # Upload the txt to the user's storage
        start = time.time()
        cmd = "inv func.invoke " + user + " put " + "'" + gutenberg_title + " " + text + "'"
        print(cmd)
        os.system(cmd)
        end = time.time()
        
if __name__ == "__main__":
    # Run the benchmark
    run_benchmark("kjv_bible", "ndp", "wordcount")