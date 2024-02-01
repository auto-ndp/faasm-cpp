
from faasmloadbalancer.ILoadBalanceStrategy import ILoadBalanceStrategy
import requests
import time


class LeastLoadStrategy(ILoadBalanceStrategy):
    def __init__(self, workers):
        self.workers = workers
        self.loads = { worker: 0 for worker in workers }
        self.last_load_time = time.time()
        
        
    def get_next_host(self, task_id) -> str:        
        if time.time() - self.last_load_time > 10:
            self.last_load_time = time.time()
            for worker in self.workers:
                # Send request to worker
                url = "http://{}:8080/f/".format(worker)
                data = {
                    "function": "get_load",
                    "user": "admin",
                }
                
                data["load_request"] = True
                
                headers =  { "Content-Type" : "application/json" }
                response = requests.post(url, json=data, headers=headers)
                
                self.loads[worker] = float(response.text)

        loads = sorted(self.loads.items(), key=lambda x: x[1])        
        return loads[0][0]
