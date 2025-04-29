import logging
import numpy as np

from dummysums import models


class ProtocolCoordinator:

    def __init__(self, name, num_clients, vector_length):
        self.step_duration = 10  # in seconds
        self.name = name

        dummysum = models.DummySum()

        dummysum.name = name
        dummysum.step_duration = self.step_duration
        dummysum.current_step = -1  # Using -1 for a warm-up to wait until the server is ready.
        dummysum.num_clients = num_clients
        dummysum.vector_length = vector_length

        dummysum.save()

    def advance_step(self):

        dummysum = models.DummySum.objects.filter(name=self.name).first()

        if dummysum.error_state:
            return

        next_step = dummysum.current_step + 1

        if next_step == 1:
            success_flag = self.check_if_enough_clients(dummysum)
            if not success_flag:
                dummysum.error_state = True
                dummysum.save()
                return
        elif next_step == 2:
            self.step1(dummysum)

        if next_step >= 2:
            dummysum.finished = True
            dummysum.save()
            return True

        logging.getLogger(__name__).error(f"[dummysum] Phase {next_step}")

        dummysum.current_step += 1

        if next_step == 0:
            dummysum.ready = True

        dummysum.save()

    def check_if_enough_clients(self, dummysum):
        num_onboarded_clients = dummysum.clients.all().count()

        if num_onboarded_clients > dummysum.num_clients:
            logging.getLogger(__name__).error(
                f"[dummysum] Phase 0 - Too much clients: {num_onboarded_clients} / {dummysum.num_clients}."
            )
            return False
        elif num_onboarded_clients < dummysum.num_clients:
            logging.getLogger(__name__).error(
                f"[dummysum] Phase 0 - Too few clients: {num_onboarded_clients} / {dummysum.num_clients}."
            )
            return False

        return True

    def step1(self, dummysum):
        final_vec = np.zeros(dummysum.vector_length).astype(float)

        for client in dummysum.clients.all():
            client_vector = np.array(client.vector.split(";")).astype(float)
            final_vec = final_vec + client_vector

        dummysum.final_vector = final_vec.tolist()

        logging.getLogger(__name__).error(f"[dummysum] Result: {final_vec}")
