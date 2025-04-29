import logging

from secsums import models
from secsums.cpp.swig import secsum as protocol

import numpy as np


class ProtocolCoordinator:

    def __init__(self, name, num_clients, vector_length):
        self.step_duration = 10  # in seconds
        self.name = name

        secsum = models.SecSum()

        secsum.name = name
        secsum.step_duration = self.step_duration
        secsum.current_step = -1  # Using -1 for a warm-up to wait until the server is ready.
        secsum.num_clients = num_clients
        secsum.vector_length = vector_length
        secsum.precision = 2

        secsum.gamma = 0.1
        secsum.delta = 0.1
        secsum.sigma = 1
        secsum.eta = 1

        secsum.min_num_clients = int((1 - secsum.delta) * secsum.num_clients)

        secsum.save()

    def advance_step(self):

        secsum = models.SecSum.objects.filter(name=self.name).first()

        if secsum.error_state:
            return

        next_step = secsum.current_step + 1

        logging.getLogger(__name__).error(f"A'1 {secsum.a1_prime.all().count()}")
        logging.getLogger(__name__).error(f"A'2 {secsum.a2_prime.all().count()}")
        logging.getLogger(__name__).error(f"A'3 {secsum.a3_prime.all().count()}")

        if next_step == 1:
            success_flag = self.check_if_enough_clients(secsum)
            if not success_flag:
                secsum.error_state = True
                secsum.save()
                return
            self.pre_step1(secsum)

        elif next_step == 5:  # Check before startin Phase 4
            success_flag = self.check_if_min_required_clients(secsum, 1)
            if not success_flag:
                secsum.error_state = True
                secsum.save()
                return
        elif next_step == 7:  # Check before startin Phase 4
            success_flag = self.check_if_min_required_clients(secsum, 2)
            if not success_flag:
                secsum.error_state = True
                secsum.save()
                return
        elif next_step == 9:
            success_flag = self.check_if_min_required_clients(secsum, 3)
            if not success_flag:
                secsum.error_state = True
                secsum.save()
                return
            self.step8(secsum)

        if next_step >= 9:
            secsum.finished = True
            secsum.save()
            return True

        statistic = models.Statistic()
        statistic.secsum = secsum
        statistic.phase = next_step
        statistic.save()

        logging.getLogger(__name__).error(f"[secsum] Phase {next_step}")

        secsum.current_step += 1

        if next_step == 0:
            secsum.ready = True

        secsum.save()

    def check_if_enough_clients(self, secsum):
        num_onboarded_clients = secsum.clients.all().count()

        if num_onboarded_clients > secsum.num_clients:
            logging.getLogger(__name__).error(
                f"[secsum] Phase 0 - Too much clients: {num_onboarded_clients} / {secsum.num_clients}."
            )
            return False
        elif num_onboarded_clients < secsum.num_clients:
            logging.getLogger(__name__).error(
                f"[secsum] Phase 0 - Too few clients: {num_onboarded_clients} / {secsum.num_clients}."
            )
            return False

        return True

    def check_if_min_required_clients(self, secsum, a_set_id):
        a_sets = {1: secsum.a1_prime, 2: secsum.a2_prime, 3: secsum.a3_prime}
        num_available_clients = a_sets[a_set_id].all().count()
        if num_available_clients < secsum.min_num_clients:
            logging.getLogger(__name__).error(
                f"[secsum] A'{a_set_id} - Too few clients: {num_available_clients} / {secsum.min_num_clients}."
            )
            return False

        return True

    def pre_step1(self, secsum):
        num_neighbours, threshold = protocol.degreeAndThreshold(
            secsum.num_clients, secsum.gamma, secsum.delta, secsum.sigma, secsum.eta
        )

        secsum.num_neighbours = int(num_neighbours)
        secsum.threshold = threshold

        logging.getLogger(__name__).error(f"[secsum] Num neighbours: {num_neighbours}")
        logging.getLogger(__name__).error(f"[secsum] Threshold: {threshold}")

        harary_map = protocol.getHararyNeighborMap(
            secsum.num_clients, secsum.num_neighbours, secsum.gamma, secsum.delta, secsum.sigma, secsum.eta
        )

        for client_id, neighbour_ids in dict(harary_map).items():
            neighbour_ids_incr = [int(_id) for _id in neighbour_ids]
            client = secsum.clients.get(run_id=int(client_id))
            neighbours = secsum.clients.filter(run_id__in=neighbour_ids_incr)
            client.neighbours.add(*neighbours)

    def step8(self, secsum):
        final_vec = np.zeros(secsum.vector_length).astype(float)

        for client in secsum.a2_prime.all():
            hb_shares = [hb_share.share for hb_share in secsum.hb_shares.filter(neighbour=client)]
            if len(hb_shares) < secsum.threshold:
                logging.getLogger(__name__).error(
                    f"[secsum] Client {client.run_id} has too few shares: {len(hb_shares)} / {secsum.threshold}."
                )
                return
            masked_input = np.array([s[:-1] for s in client.masked_input.split(";")[:-1]]).astype(int)
            ri = protocol.reconstructSecSharing(secsum.threshold, secsum.vector_length, hb_shares)
            final_vec += masked_input - ri

        dropout_clients = set(secsum.a1_prime.all()).difference(set(secsum.a2_prime.all()))
        for client in dropout_clients:
            hs_shares = [hs_share.share for hs_share in secsum.hs_shares.filter(neighbour=client)]
            if len(hs_shares) < secsum.threshold:
                logging.getLogger(__name__).error(
                    f"[secsum] Dropout client {client.run_id} has too few shares: {len(hs_shares)} / {secsum.threshold}."
                )
                return
            a1_prime_ids = [str(cl.run_id) for cl in secsum.a1_prime.all()]
            neighbour_ids = [str(cl.run_id) for cl in client.neighbours.all()]
            mijs_sum = protocol.recoverDropoutInputs(
                secsum.threshold,
                secsum.vector_length,
                a1_prime_ids,
                neighbour_ids,
                client.public_keys.key1,
                str(client.run_id),
                hs_shares,
            )

        final_vec *= 10 ** (-secsum.precision)

        secsum.final_vector = final_vec.tolist()

        logging.getLogger(__name__).error(f"[secsum] Result: {final_vec}")
