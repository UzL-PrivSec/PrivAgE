from dummysums.services.invoker import DummySumInvoker
import numpy as np
import logging


def do_dummysums(num_clients):

    vector_length = 16

    result_vec = np.zeros(vector_length)

    for i in range(2):

        logging.getLogger(__name__).error(f"[dummysum] Mult. {i} Start")

        dummysum_invoker = DummySumInvoker(f"Iteration-{i}", num_clients, vector_length)
        aggregated_vectors = dummysum_invoker.run_protocol()

        result_vec += aggregated_vectors

        logging.getLogger(__name__).error(f"[dummysum] Mult. {i} Result: {result_vec}")
