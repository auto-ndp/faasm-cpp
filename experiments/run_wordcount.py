import time
import os
import subprocess
import asyncio

text_sources_dir = "/users/DonaldJ/faasm/cpp/experiments/text_sources/"
def run_benchmark(gutenberg_title, user, func, num_runs=10):

    for i in range(num_runs):
        cmd = "inv func.invoke " + user + " " + func + " \'" + gutenberg_title + "\'"
        print("Command: ", cmd)
        start = time.time()
        subprocess.run(cmd, shell=True)
        print("Time taken to run wordcount: ", time.time() - start)

async def invoke_func_async(user, func, gutenberg_title):
    cmd = ["inv", "func.invoke", user, func, gutenberg_title]
    process = await asyncio.create_subprocess_exec(*cmd, stdout=asyncio.subprocess.PIPE, stderr=asyncio.subprocess.PIPE)
    stdout, stderr = await process.communicate()
    print(f'[{cmd!r} exited with {process.returncode}]')
    return stdout, stderr

async def run_benchmark_async(gutenberg_title, user, func, num_runs=10):
    tasks = []
    for i in range(num_runs):
        task = asyncio.ensure_future(invoke_func_async(user, func, gutenberg_title))
        tasks.append(task)
        await asyncio.sleep(1/i)
    
    responses = await asyncio.gather(*tasks)
    for response in responses:
        print(response)
if __name__ == "__main__":
    # Run the benchmark
    run_benchmark("pride_and_prejudice", "ndp", "wordcount")