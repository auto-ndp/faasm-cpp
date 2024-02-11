import time
import os

def run_benchmark(gutenberg_title, user, func, num_runs=10):

    # Get the text
    with open("./text_sources/{}.txt".format(gutenberg_title), "r") as f:
        text = f.read()
        
        # Upload the txt to the user's storage
        start = time.time()
        print("Calling inv func.invoke to upload the text to the user's storage")
        print("Key: ", gutenberg_title)
        cmd = "inv func.invoke " + user + " put " + "\'" + gutenberg_title + " " + text + "\'"
        os.system(cmd)
        print("Time taken to upload the text to the user's storage: ", time.time() - start)
        end = time.time()
        
        
        print("Calling inv func.invoke to run the wordcount on the text")
        cmd = "inv func.invoke " + user + " " + func + " \'" + gutenberg_title + "\'"
        print("Command: ", cmd)
        wordcount_time = time.time()
        os.system(cmd)
        print("Time taken to run the wordcount on the text: ", time.time() - wordcount_time)
        
if __name__ == "__main__":
    # Run the benchmark
    run_benchmark("kjv_bible", "ndp", "wordcount")