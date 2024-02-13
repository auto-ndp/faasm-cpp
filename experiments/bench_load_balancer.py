
import asyncio 
import argparse
import time

LOAD_BALANCE_STRATEGY = "round_robin"
FORBID_NDP = "False"
ASYNC_TOGGLED = "False"
DEFAULT_RUNS = 10

async def dispatch_func_async(user, func, input_data):
    cmd = ["inv", "func.dispatch-function", user, func, input_data, LOAD_BALANCE_STRATEGY, ASYNC_TOGGLED, FORBID_NDP]
    start_time = time.perf_counter()
    process = await asyncio.create_subprocess_exec(*cmd, stdout=asyncio.subprocess.PIPE, stderr=asyncio.subprocess.PIPE)
    await process.communicate()
    end_time = time.perf_counter()
    return float(end_time - start_time)

async def benchmark_async(user, func, input_data, num_runs=DEFAULT_RUNS):
    
    latencies = []
    
    INCREMENTS = 10
    for batch_size in range(1, num_runs+1, INCREMENTS):
        
        batch_tasks = []
        
        batch_start_time = time.perf_counter()
        
        # Create batch of tasks
        for i in range(batch_size):
            task = asyncio.ensure_future(dispatch_func_async(user, func, input_data))
            batch_tasks.append(task)
            
            sleep_time = max(0, 1/i - (time.perf_counter() - batch_start_time))
            await asyncio.sleep(sleep_time)
            
        # Await batch
        responses = await asyncio.gather(*batch_tasks)

        for response in responses:
            latencies.append((batch_size, response))       

    # Save results to timestamped file
    timestamp = time.strftime("%Y%m%d-%H%M%S")
    with open(f"./results/{timestamp}_{func}_{LOAD_BALANCE_STRATEGY}_{FORBID_NDP}", "a") as f:
        
        # Write header containing all metadata
        f.write("User,{user},\n")
        f.write("Function,{func},\n")
        f.write("Input Data,{input_data},\n")
        f.write("Num Runs,{num_runs},\n")
        f.write("Load Balance Strategy,{LOAD_BALANCE_STRATEGY},\n")
        f.write("Async Toggled,{ASYNC_TOGGLED},\n")
        f.write("Forbid NDP,{FORBID_NDP},\n")
        
        
        f.write("Batch Size,Latency\n")        
        for batch_size, latency in latencies:
            f.write(f"{batch_size},{latency},\n")
            

        
if __name__ == "__main__":
    
    # Parse args (user function input_data num_runs load_balance_strategy async_toggle forbid_ndp)
    arg_parser = argparse.ArgumentParser(description="Benchmark load balancer")
    
    arg_parser.add_argument("user", help="User to invoke function as")
    arg_parser.add_argument("func", help="Function to invoke")
    arg_parser.add_argument("input_data", help="Input data for function")
    arg_parser.add_argument("num_runs", help="Number of runs to perform", type=int, default=10)
    arg_parser.add_argument("load_balance_strategy", help="Load balance strategy to use")
    #arg_parser.add_argument("async_toggle", help="Toggle async behaviour", type=bool, default=False)
    #arg_parser.add_argument("forbid_ndp", help="Toggle NDP", type=bool, default=False)
    
    args = arg_parser.parse_args()
    
    LOAD_BALANCE_STRATEGY = args.load_balance_strategy
    
    # if args.async_toggle:
    #     ASYNC_TOGGLED = True
    # 
    # if args.forbid_ndp:
    #     FORBID_NDP = True
    
    asyncio.run(benchmark_async(args.user, args.func, args.input_data, args.num_runs))
