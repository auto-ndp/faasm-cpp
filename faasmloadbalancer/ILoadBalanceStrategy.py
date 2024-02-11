from abc import ABC, abstractmethod

class ILoadBalanceStrategy(ABC):
    
    @abstractmethod
    def get_next_host(self, user=None, func_name=None):
        pass
    
    