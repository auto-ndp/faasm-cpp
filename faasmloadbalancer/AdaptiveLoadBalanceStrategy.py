
from faasmloadbalancer.ILoadBalanceStrategy import ILoadBalanceStrategy
import requests

class LeastLoadStrategy(ILoadBalanceStrategy):
    def __init__(self, workers):
        self.workers = workers
        
    def get_next_host(self, task_id) -> str:
        # Get server loads and sort them
        loads = []
        for worker in self.workers:
            
            # Send request to worker
            url = "http://{}:8080/f/".format(worker)
            data = {
                "function": "get_load",
                "user": "admin",
            }
            
            data["is_loadrequest"] = True
            
            headers =  { "Content-Type" : "application/json" }
            response = requests.post(url, json=data, headers=headers)
            
        
            # Add to list
            loads.append(float(worker, response.text))
            
        # Sort by load
        loads.sort(key=lambda tup: tup[1])
        
        # Return the worker ID
        return loads[0][0]
