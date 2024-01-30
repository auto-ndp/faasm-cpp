from os import makedirs, listdir
from os.path import join, exists, splitext
from shutil import rmtree
from subprocess import run
import time
import requests
from invoke import task

import aiohttp
from aiohttp import ClientSession
import asyncio
from faasmtools.env import PROJ_ROOT
from faasmtools.endpoints import (
    get_faasm_invoke_host_port,
    get_faasm_upload_host_port,
    get_knative_headers,
)

# Load Balancer imports
from faasmtools.compile_util import wasm_cmake, wasm_copy_upload
from faasmloadbalancer.RoundRobinLoadBalancer import RoundRobinLoadBalancerStrategy
from faasmloadbalancer.WorkerHashLoadBalancer import WorkerHashLoadBalancerStrategy

FAABRIC_MSG_TYPE_FLUSH = 3

FUNC_DIR = join(PROJ_ROOT, "func")
FUNC_BUILD_DIR = join(PROJ_ROOT, "build", "func")
NATIVE_FUNC_BUILD_DIR = join(PROJ_ROOT, "build", "native-func")

WORKER_ADDRESSES = ['worker-0', 'worker-1', 'worker-2']

round_robin_balancer = RoundRobinLoadBalancerStrategy(WORKER_ADDRESSES)
worker_hash_balancer = WorkerHashLoadBalancerStrategy(WORKER_ADDRESSES)

def _get_all_user_funcs(user):
    # Work out all the functions for this user (that we assume will have been
    # built)
    funcs = list()
    for func_file in listdir(join(FUNC_BUILD_DIR, user)):
        name, ext = splitext(func_file)
        if ext != ".wasm":
            continue

        funcs.append(name)

    return funcs


def _copy_built_function(user, func):
    src_file = join(FUNC_BUILD_DIR, user, ".".join([func, "wasm"]))
    wasm_copy_upload(user, func, src_file)


@task(default=True, name="compile")
def compile(ctx, user, func, clean=False, debug=False, native=False):
    """
    Compile a function
    """
    if native:
        if exists(NATIVE_FUNC_BUILD_DIR) and clean:
            rmtree(NATIVE_FUNC_BUILD_DIR)

        makedirs(NATIVE_FUNC_BUILD_DIR, exist_ok=True)

        build_cmd = ["cmake", "-GNinja", FUNC_DIR]

        build_cmd = " ".join(build_cmd)
        print(build_cmd)
        run(
            "cmake -GNinja {}".format(FUNC_DIR),
            check=True,
            shell=True,
            cwd=NATIVE_FUNC_BUILD_DIR,
        )

        run(
            "ninja {}".format(func),
            shell=True,
            check=True,
            cwd=NATIVE_FUNC_BUILD_DIR,
        )
    else:
        # Build the function (gets written to the build dir)
        wasm_cmake(FUNC_DIR, FUNC_BUILD_DIR, func, clean, debug)

        # Copy into place
        _copy_built_function(user, func)


@task
def upload(ctx, user, func):
    """
    Upload a compiled function
    """
    host, port = get_faasm_upload_host_port()
    func_file = join(FUNC_BUILD_DIR, user, "{}.wasm".format(func))
    url = "http://{}:{}/f/{}/{}".format(host, port, user, func)
    response = requests.put(url, data=open(func_file, "rb"))

    print("Response {}: {}".format(response.status_code, response.text))


@task
def upload_user(ctx, user):
    """
    Upload all compiled functions for a user
    """
    funcs = _get_all_user_funcs(user)
    for f in funcs:
        upload(ctx, user, f)


@task
def invoke(ctx, user, func, input_data, mpi=None, graph=False):
    """
    Invoke a given function
    """
    # host, port = get_faasm_invoke_host_port()
    host = "worker-0"
    port = 8080
    url = "http://{}:{}/f/".format(host, port)
    data = {
        "function": func,
        "user": user,
    }
    
    print("Invoking function: {}_{}".format(user, func))
    print("Connecting to: {}".format(url))

    if input_data:
        data["input_data"] = input_data

    if mpi is not None:
        data["mpi_world_size"] = int(mpi)

    if graph:
        data["record_exec_graph"] = True
        data["async"] = True

    # headers = get_knative_headers()
    headers =  { "Content-Type" : "application/json" }
    print("Headers: {}".format(headers))
    print("Data: {}".format(data))
    
    response = requests.post(url, json=data, headers=headers)

    if response.status_code != 200:
        print("Error ({}):\n{}".format(response.status_code, response.text))
        exit(1)

    print("Success:\n{}".format(response.text))

@task
async def dispatch_function(ctx, user, func, input_data, load_balance_strategy, session, mpi=None, graph=False):
    print("Running function: {}_{}".format(user, func))
    print("Initialising load balancer")
    
    # Write a function that retrieves the addresses of the workers
    # and writes them to a file
    # Then, run the load balancer
    # Then, invoke the function
    # Then, kill the load balancer
    # Then, read the file and print the results
    # Then, delete the file
    
    host, port = get_faasm_invoke_host_port()
    
    balancer = get_load_balancer("roundrobin")
    worker_to_run_on = balancer.get_next_host()
    worker_to_run_on = "worker-1"
    # print("Running on worker: {}".format(worker_to_run_on))
    
    port = 8080
    url = "http://{}:{}/f/".format(worker_to_run_on, port)
    data = {
        "function": func,
        "user": user,
    }
    
    print("Invoking function: {}_{}".format(user, func))
    print("Connecting to: {}".format(url))

    data["forbid_ndp"] = False
    data["async"] = False
    if input_data:
        data["input_data"] = input_data

    if mpi is not None:
        data["mpi_world_size"] = int(mpi)

    if graph:
        data["record_exec_graph"] = True
        data["async"] = True

    # headers = get_knative_headers()
    headers =  { "Content-Type" : "application/json" }
    print("Headers: {}".format(headers))
    print("Data: {}".format(data))
    
    async with session.post(url, json=data, headers=headers) as response:
        if response.status != 200:
            text = await response.text()
            print(f"Error ({response.status}):\n{text}")
            exit(1)
        else:
            print("Success:\n{}".format(await response.text()))
            return await response.text()

@task
def test_load_balancer_async(ctx, user, func, input_data):
    def run_async_code():
        async def main():
            async with aiohttp.ClientSession() as session:
                tasks = []
                for i in range(0, 10):
                    tasks.append(dispatch_function(ctx, user, func, input_data, "roundrobin", session))
                responses = await asyncio.gather(*tasks)
                return responses
        return asyncio.run(main())
    run_async_code()
    
@task
def test_load_balancer_sync(ctx, user, func, input_data):
    for i in range(0, 10):
        invoke(ctx, user, func, input_data)
        
@task
def update(ctx, user, func, clean=False, debug=False, native=False):
    """
    Combined compile, upload, flush
    """
    compile(ctx, user, func, clean=clean)

    upload(ctx, user, func)

    flush(ctx)


@task
def flush(ctx):
    """
    Flush the Faasm cluster
    """
    # headers = get_knative_headers()
    # host, port = get_faasm_invoke_host_port()
    headers = { "Content-Type" : "application/json" }
    host = "worker-0"
    port = 8080
    url = "http://{}:{}".format(host, port)
    data = {"type": FAABRIC_MSG_TYPE_FLUSH}
    print("Flushing Faasm cluster at: {}".format(url))
    try:
        response = requests.post(url, json=data, headers=headers, timeout=10)
        response.raise_for_status()
    except requests.exceptions.RequestException as e:
        print(f"Request failed: {e}")
        return

    print("Flush response {}: {}".format(response.status_code, response.text))


@task
def user(ctx, user, clean=False, debug=False):
    """
    Compile all functions belonging to the given user
    """
    # Build all funcs for this user (will fail if any builds fail)
    target = "{}_all_funcs".format(user)
    wasm_cmake(FUNC_DIR, FUNC_BUILD_DIR, target, clean, debug)

    funcs = _get_all_user_funcs(user)
    for f in funcs:
        _copy_built_function(user, f)


@task
def local(ctx, clean=False, debug=False):
    """
    Compile all functions used in the tests
    """
    user(ctx, "demo", clean, debug)
    user(ctx, "errors", clean, debug)
    user(ctx, "mpi", clean, debug)
    user(ctx, "omp", clean, debug)


def get_load_balancer(strategy_name: str):
    if (strategy_name.lower() == "roundrobin" or strategy_name.lower() == "round_robin"):
        print("Using round robin load balancer")
        return round_robin_balancer
    elif (strategy_name.lower() == "workerhash" or strategy_name.lower() == "worker_hash"):
        print("Using worker hash load balancer")
        return worker_hash_balancer
    else:
        print("Invalid load balancer strategy name: {}".format(strategy_name))
        print("Returning round robin load balancer by default")
        return round_robin_balancer