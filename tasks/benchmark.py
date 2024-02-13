from invoke import task

import asyncio
import time
import aiohttp

from faasmloadbalancer.RoundRobinLoadBalancer import RoundRobinLoadBalancerStrategy
from faasmloadbalancer.WorkerHashLoadBalancer import WorkerHashLoadBalancerStrategy

from func import get_load_balancer, dispatch_function

ROUND_ROBIN_BALANCER = RoundRobinLoadBalancerStrategy(['worker-0', 'worker-1', 'worker-2'])
WORKER_HASH_BALANCER = WorkerHashLoadBalancerStrategy(['worker-0', 'worker-1', 'worker-2'])

@task
def test_load_balancer(ctx, user, func, input_data, load_balance_strategy, n, async_toggle, forbid_ndp):
    
    number_iterations = int(n)
    
    if async_toggle.lower() == "true":
        async_toggle = True
    else:
        async_toggle = False
        
    if forbid_ndp.lower() == "true":
        forbid_ndp = True
    else:
        forbid_ndp = False
        
    # create file to store results
    fp = "./experiments/results/" + time.strftime("%Y%m%d-%H%M%S") + "_" + load_balance_strategy + "_results.csv"
    results_file = open(fp, "a")
    
    with open(fp, "a") as results_file:
        results_file.write("Input data" + "," + input_data + "\n")
        results_file.write("user" + "," + user + "\n")
        results_file.write("function" + "," + func + "\n")
        results_file.write("async toggle" + "," + str(async_toggle) + "\n")
        results_file.write("forbid ndp" + "," + str(forbid_ndp) + "\n")
        results_file.write("load balance strategy" + "," + load_balance_strategy + "\n")
        results_file.write("iteration, latency\n")
        for i in range(0, number_iterations):
            print("Iteration: {}/{}".format(i, number_iterations))
            latency = dispatch_function(ctx, user, func, input_data, load_balance_strategy, async_toggle, forbid_ndp)
            results_file.write(str(i) + "," + str(latency) + "\n")

async def dispatch_func_async(session, url, data, headers):
    async with session.post(url, json=data, headers=headers) as response:
        return await response.text()
   
async def batch_send(url, data, headers, batch_size, latencies):
    async with aiohttp.ClientSession() as session:
        tasks = []
        for _ in range(batch_size):
            tasks.append(dispatch_func_async(session, url, data, headers))
            await asyncio.sleep(1/batch_size)

        responses = await asyncio.gather(*tasks)
        return responses

@task
def throughput_test(ctx, user, func, input_data, load_balance_strategy, n, async_toggle, forbid_ndp):
    
    results = []
    
    data = {
        "function": func,
        "user": user,
        "input_data": input_data
    }
    
    headers = { "Content-Type" : "application/json" }
    
    if async_toggle.lower() == "true":
        async_toggle = True
    
    if forbid_ndp.lower() == "true":
        forbid_ndp = True
        
    for i in range(0, n):
        latencies = []
        start_time = time.perf_counter()
        asyncio.run(batch_send(data, headers, i, latencies))
        end_time = time.perf_counter()
        print("Time taken to run batch: ", end_time - start_time)
        
        result_dict = {
            "batch_size": i,
            "mean_latency" : sum(latencies)/len(latencies),
            "median_latency": latencies[len(latencies)//2],
            "time_taken": end_time - start_time            
        }
        
        print("Result: ", result_dict)
        results.append(result_dict)
        
    # Save results to timestamped file
    timestamp = time.strftime("%Y%m%d-%H%M%S")
    with open(f"./experiments/results/{timestamp}_{func}_{load_balance_strategy}_{forbid_ndp}", "a") as f:
        
        # Write header containing all metadata
        f.write("User,{user},\n")
        f.write("Function,{func},\n")
        f.write("Input Data,{input_data},\n")
        f.write("Num Runs,{num_runs},\n")
        f.write("Load Balance Strategy,{load_balance_strategy},\n")
        f.write("Async Toggled,{async_toggle},\n")
        f.write("Forbid NDP,{forbid_ndp},\n")
        
        
        f.write("Batch Size,Mean Latency,Median Latency,Time Taken\n")        
        for result in results:
            f.write(f"{result['batch_size']},{result['mean_latency']},{result['median_latency']},{result['time_taken']},\n")
        
    
    