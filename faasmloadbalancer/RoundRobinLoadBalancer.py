import itertools
from faasmloadbalancer.ILoadBalanceStrategy import ILoadBalanceStrategy
import threading
class RoundRobinLoadBalancerStrategy(ILoadBalanceStrategy):
    def __init__(self, workers):
        self.workers = workers
        self.worker_iterator = itertools.cycle(workers)
        
    def get_next_host(self, user=None, func_name=None):
        with threading.Lock():
            return next(self.worker_iterator)