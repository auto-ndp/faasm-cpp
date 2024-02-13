from faasmloadbalancer.ILoadBalanceStrategy import ILoadBalanceStrategy

class MetricsLoadBalancer(ILoadBalanceStrategy):
    def __init__(self, workers):
        self.workers = workers

    def _request_load_average(self, worker):
        pass
        
    def get_next_host(self, user=None, func_name=None):
        
        # Get the UNIX Load Average for each worker
        # load_avgs = {worker: self._request_load_average(worker) for worker in self.workers}
        
        # On the Faabric side - we need to handle this message that is passed to the worker and return based
        # on getUtilisationStats() in the worker
        
        # Select the worker with the lowest load average
        # selected_worker = min(load_avgs, key=load_avgs.get)
        
        # return selected_worker
        pass