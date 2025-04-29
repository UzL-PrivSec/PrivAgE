import time

from dummysums.protocol import ProtocolCoordinator
from dummysums.models import DummySum


class DummySumInvoker:
    def __init__(self, name, num_clients, vector_length):
        self.coordinator = ProtocolCoordinator(name, num_clients, vector_length)
        self.name = name

    def run_protocol(self):
        while True:
            finished = self.coordinator.advance_step()
            time.sleep(self.coordinator.step_duration)
            if finished:
                break

        dummysum = DummySum.objects.filter(name=self.name).first()
        return dummysum.final_vector
