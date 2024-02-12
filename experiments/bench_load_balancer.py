
import asyncio 
import argparse

LOAD_BALANCE_STRATEGY = "round_robin"
FORBID_NDP = False
ASYNC_TOGGLED = False
DEFAULT_RUNS = 10

async def dispatch_func_async(user, func, input_data):
    cmd = ["inv", "func.dispatch_function", user, func, input_data, LOAD_BALANCE_STRATEGY, ASYNC_TOGGLED, FORBID_NDP]
    process = await asyncio.create_subprocess_exec(*cmd, stdout=asyncio.subprocess.PIPE, stderr=asyncio.subprocess.PIPE)
    stdout, stderr = await process.communicate()
    print(f'[{cmd!r} exited with {process.returncode}]')
    return stdout, stderr

async def benchmark_async(user, func, input_data, num_runs=DEFAULT_RUNS):
    tasks = []
    for i in range(1, num_runs+1):
        task = asyncio.ensure_future(dispatch_func_async(user, func, input_data))
        tasks.append(task)
        await asyncio.sleep(1/i)
    
    responses = await asyncio.gather(*tasks)
    for response in responses:
        print(response)
        
if __name__ == "__main__":
    
    # Parse args (user function input_data num_runs load_balance_strategy async_toggle forbid_ndp)
    arg_parser = argparse.ArgumentParser(description="Benchmark load balancer")
    
    arg_parser.add_argument("user", help="User to invoke function as")
    arg_parser.add_argument("func", help="Function to invoke")
    arg_parser.add_argument("input_data", help="Input data for function")
    arg_parser.add_argument("num_runs", help="Number of runs to perform", type=int, default=10)
    arg_parser.add_argument("load_balance_strategy", help="Load balance strategy to use")
    arg_parser.add_argument("async_toggle", help="Toggle async behaviour")
    arg_parser.add_argument("forbid_ndp", help="Toggle NDP")
    
    args = arg_parser.parse_args()
    
    LOAD_BALANCE_STRATEGY = args.load_balance_strategy
    ASYNC_TOGGLED = args.async_toggle
    FORBID_NDP = args.forbid_ndp
    
    asyncio.run(benchmark_async(args.user, args.func, args.input_data, args.num_runs))
