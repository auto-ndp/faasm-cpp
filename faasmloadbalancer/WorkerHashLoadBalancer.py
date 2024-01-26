from faasmloadbalancer.ILoadBalanceStrategy import ILoadBalanceStrategy

class WorkerHashLoadBalancerStrategy(ILoadBalanceStrategy):
    def __init__(self, workers):
        self.workers = workers

    def get_next_host(self, task_id) -> str:
        # Calculate the hash of the task ID
        hash_value = hash(task_id)

        # Get the index of the worker based on the hash value
        worker_index = hash_value % len(self.workers)

        # Return the worker ID
        return self.workers[worker_index]
