import time

from secsums.protocol import ProtocolCoordinator
from secsums.models import SecSum


class SecSumInvoker:
    def __init__(self, name, num_clients, vector_length):
        self.coordinator = ProtocolCoordinator(name, num_clients, vector_length)
        self.name = name

    def run_protocol(self):
        while True:
            finished = self.coordinator.advance_step()
            time.sleep(self.coordinator.step_duration)
            if finished:
                break

        secsum = SecSum.objects.filter(name=self.name).first()
        return secsum.final_vector
