import time
import os
import subprocess

def run_benchmark(gutenberg_title, user, func, num_runs=10):

    # Get the text
    with open("./text_sources/{}.txt".format(gutenberg_title), "r") as f:
        text = f.read().replace("\n", " ")
        
        # Upload the txt to the user's storage
        start = time.time()
        print("Calling inv func.invoke to upload the text to the user's storage")
        print("Key: ", gutenberg_title)
        put_cmd = "inv func.invoke " + user + " put " + "\'" + gutenberg_title + " " + text + "\'"
        print("Command: ", put_cmd[:100])
        
        process = subprocess.Popen(put_cmd, shell=True, stdout=subprocess.PIPE)
        process.communicate(input=text.encode())
        print("Time taken to upload the text to the user's storage: ", time.time() - start)
        
        
        print("Calling inv func.invoke to run the wordcount on the text")
        function_cmd = "inv func.invoke " + user + " " + func + " \'" + gutenberg_title + "\'"
        print("Command: ", function_cmd)
        wordcount_time = time.time()
        subprocess.run(function_cmd, shell=True)
        print("Time taken to run the wordcount on the text: ", time.time() - wordcount_time)
        
if __name__ == "__main__":
    # Run the benchmark
    run_benchmark("kjv_bible", "ndp", "wordcount")