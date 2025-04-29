from secsums.services.invoker import SecSumInvoker
import numpy as np
import logging


def do_secsums(num_clients):

    vector_length = 16

    result_vec = np.zeros(vector_length)

    for i in range(2):

        logging.getLogger(__name__).error(f"[secsum] Mult. {i} Start")

        secsum_invoker = SecSumInvoker(f"Iteration-{i}", num_clients, vector_length)
        aggregated_vectors = secsum_invoker.run_protocol()

        result_vec += aggregated_vectors

        logging.getLogger(__name__).error(f"[secsum] Mult. {i} Result: {result_vec}")
