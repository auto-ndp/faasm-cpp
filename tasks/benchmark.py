from invoke import task

import asyncio
import time
import aiohttp

from faasmloadbalancer.RoundRobinLoadBalancer import RoundRobinLoadBalancerStrategy
from faasmloadbalancer.WorkerHashLoadBalancer import WorkerHashLoadBalancerStrategy
from faasmloadbalancer.MetricsLoadBalancer import MetricsLoadBalancer

from . import func
import threading

ROUND_ROBIN_BALANCER = RoundRobinLoadBalancerStrategy(['worker-0', 'worker-1', 'worker-2'])
WORKER_HASH_BALANCER = WorkerHashLoadBalancerStrategy(['worker-0', 'worker-1', 'worker-2'])
METRICS_LOAD_BALANCER = MetricsLoadBalancer(['worker-0', 'worker-1', 'worker-2'])


selected_balancer = ROUND_ROBIN_BALANCER

@task
def test_load_balancer(ctx, user, rados_func, input_data, load_balance_strategy, n, async_toggle, forbid_ndp):
    
    number_iterations = int(n)
    
    # create file to store results
    fp = "./experiments/results/" + time.strftime("%Y%m%d-%H%M%S") + "_" + load_balance_strategy + "_results.csv"
    results_file = open(fp, "a")
    
    with open(fp, "a") as results_file:
        results_file.write("Input data" + "," + input_data + "\n")
        results_file.write("user" + "," + user + "\n")
        results_file.write("function" + "," + rados_func + "\n")
        results_file.write("async toggle" + "," + str(async_toggle) + "\n")
        results_file.write("forbid ndp" + "," + str(forbid_ndp) + "\n")
        results_file.write("load balance strategy" + "," + load_balance_strategy + "\n")
        results_file.write("iteration, latency\n")
        for i in range(0, number_iterations):
            print("Iteration: {}/{}".format(i, number_iterations))
            latency = func.dispatch_function(ctx, user, rados_func, input_data, load_balance_strategy, async_toggle, forbid_ndp)
            results_file.write(str(i) + "," + str(latency) + "\n")

async def dispatch_func_async(session, url, data, headers):
    start_time = time.perf_counter()
    async with session.post(url, json=data, headers=headers) as response:
        end_time = time.perf_counter()
        await response.text()
        return end_time - start_time
   
async def batch_send(data, headers, batch_size, load_balancer):
    async with aiohttp.ClientSession() as session:
        tasks = []
        for _ in range(batch_size):
            with threading.Lock():
                worker_id = load_balancer.get_next_host(data["user"], data["function"])
            url = "http://{}:{}/f/".format(worker_id, 8080)
            tasks.append(dispatch_func_async(session, url, data, headers))
            # await asyncio.sleep(1/batch_size)

        responses = await asyncio.gather(*tasks)
        return responses

@task
def throughput_test(ctx, user, rados_func, input_data, load_balance_strategy, n, async_toggle, forbid_ndp):
    
    results = []
    
    if load_balance_strategy.lower() == "roundrobin":
        selected_balancer = ROUND_ROBIN_BALANCER
    elif load_balance_strategy.lower() == "workerhash":
        selected_balancer = WORKER_HASH_BALANCER
    elif load_balance_strategy.lower() == "metrics":
        selected_balancer = METRICS_LOAD_BALANCER
    else:
        print("Invalid load balancer strategy: ", load_balance_strategy)
        exit(1)
        
    print("Getting load balancer: ", selected_balancer)
    data = {
        "function": rados_func,
        "user": user,
        "input_data": input_data
    }
    
    headers = { "Content-Type" : "application/json" }
    
    if async_toggle.lower() == "true":
        data["async"] = True
    
    if forbid_ndp.lower() == "true":
        print("Forbidding NDP:")
        data["forbid_ndp"] = True
    
    ITERATIONS = int(n)
    for i in range(1, ITERATIONS):
        latencies = []
        start_time = time.perf_counter()
        print("Running a batch of size: ", i)
        latencies = asyncio.run(batch_send(data, headers, i, selected_balancer))
        end_time = time.perf_counter()
        print("Time taken to run batch: ", end_time - start_time)
        
        result_dict = {
            "batch_size": i,
            "mean_latency" : sum(latencies)/len(latencies),
            "median_latency": sorted(latencies)[len(latencies)//2],
            "time_taken": end_time - start_time            
        }
        
        print("Result: ", result_dict)
        results.append(result_dict)
        
    # Save results to timestamped file
    timestamp = time.strftime("%Y%m%d-%H%M%S")
    with open(f"./experiments/results/{timestamp}_{rados_func}_{load_balance_strategy}_{forbid_ndp}", "a") as f:
        
        # Write header containing all metadata
        f.write("User,{user}\n")
        f.write("Function,{rados_func}\n")
        f.write("Input Data,{input_data}\n")
        f.write("Num Runs,{num_runs}\n")
        f.write("Load Balance Strategy,{load_balance_strategy}\n")
        f.write("Async Toggled,{async_toggle}\n")
        f.write("Forbid NDP,{forbid_ndp}\n")
        
        
        f.write("Batch Size,Mean Latency,Median Latency,Time Taken\n")        
        for result in results:
            f.write(f"{result['batch_size']},{result['mean_latency']},{result['median_latency']},{result['time_taken']},\n")
        
@task
def gather_loads(ctx, user, rados_func, input_data, n):
    load_balancer = METRICS_LOAD_BALANCER
    
    pass