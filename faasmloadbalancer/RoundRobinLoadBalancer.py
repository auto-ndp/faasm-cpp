import itertools
from faasmloadbalancer.ILoadBalanceStrategy import ILoadBalanceStrategy

class RoundRobinLoadBalancerStrategy(ILoadBalanceStrategy):
    def __init__(self, workers):
        self.workers = workers
        self.worker_iterator = itertools.cycle(workers)
        
    def get_next_host(self):
        return next(self.worker_iterator)