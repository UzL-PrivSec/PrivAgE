import json

from django.shortcuts import render
from django.http import JsonResponse, HttpResponseServerError
from django.db.models import F
from django.db import transaction

from . import models


def step0(request, secsum):

    with transaction.atomic():
        secsum.last_client_id = F("last_client_id") + 1
        secsum.save(update_fields=["last_client_id"])
        secsum.refresh_from_db()

        client = models.Client()
        client.run_id = secsum.last_client_id
        client.save()

    secsum.clients.add(client)

    return JsonResponse(
        {"id": client.run_id, "step_duration": secsum.step_duration, "precision": secsum.precision}, status=200
    )


def step1(request, secsum):

    client_id = json.loads(request.body)["id"]

    client = secsum.clients.get(run_id=client_id)
    neighbour_ids = [neighbour.run_id for neighbour in client.neighbours.all()]

    return JsonResponse({"t": secsum.threshold, "l": secsum.vector_length, "neighbours": neighbour_ids}, status=200)


def step21(request, secsum):

    client_message: dict = json.loads(request.body)

    client_id = client_message["id"]
    client = secsum.clients.get(run_id=client_id)

    public_keys = models.PublicKeys()
    public_keys.key1 = client_message["pubKey1"]
    public_keys.key2 = client_message["pubKey2"]
    public_keys.save()

    client.public_keys = public_keys
    client.save()

    return JsonResponse({}, status=200)


def step22(request, secsum):

    client_id = json.loads(request.body)["id"]
    client = secsum.clients.get(run_id=client_id)

    neighbour_public_keys_list = []
    for neighbour in client.neighbours.all():
        neighbour_id = neighbour.run_id
        neighbour_public_keys = neighbour.public_keys
        concat_msg = ";".join([str(neighbour_id), neighbour_public_keys.key1, neighbour_public_keys.key2])
        neighbour_public_keys_list.append(concat_msg)

    return JsonResponse({"key_dict": neighbour_public_keys_list}, status=200)


def step3(request, secsum):

    client_message = json.loads(request.body)

    client_id = client_message["id"]
    client = secsum.clients.get(run_id=client_id)

    for neighbour_id, cij in client_message["cijMap"].items():
        cij_msg = models.CijMsg()
        cij_msg.neighbour = client
        cij_msg.msg = cij
        cij_msg.save()

        neighbour = secsum.clients.get(run_id=int(neighbour_id))
        neighbour.cij_msgs.add(cij_msg)

    secsum.a1_prime.add(client)

    return JsonResponse({}, status=200)


def step4(request, secsum):

    client_id = json.loads(request.body)["id"]
    client = secsum.clients.get(run_id=client_id)

    cij_msgs = [(cij_msg.neighbour.run_id, cij_msg.msg) for cij_msg in client.cij_msgs.all()]

    return JsonResponse({"cij_list": cij_msgs}, status=200)


def step5(request, secsum):

    client_message = json.loads(request.body)

    client_id = client_message["id"]
    client = secsum.clients.get(run_id=client_id)
    client.masked_input = client_message["masked_input"]
    client.save()

    secsum.a2_prime.add(client)

    return JsonResponse({}, status=200)


def step6(request, secsum):

    client_id = json.loads(request.body)["id"]
    client = secsum.clients.get(run_id=client_id)

    neighbour_ids = set([cl.run_id for cl in client.neighbours.all()])
    a2_prime_clients = set([cl.run_id for cl in secsum.a2_prime.all()])
    r1 = a2_prime_clients.intersection(neighbour_ids)

    a1_prime_clients = set([cl.run_id for cl in secsum.a1_prime.all()])
    r2 = a1_prime_clients.difference(a2_prime_clients).intersection(neighbour_ids)

    return JsonResponse({"R1": list(r1), "R2": list(r2)}, status=200)


def step7(request, secsum):

    client_message = json.loads(request.body)

    for neighbour_id, share in client_message["hbs"].items():
        hb_share = models.HBShare()
        hb_share.neighbour = secsum.clients.get(run_id=int(neighbour_id))
        hb_share.share = share
        hb_share.save()
        secsum.hb_shares.add(hb_share)

    for neighbour_id, share in client_message["hss"].items():
        hs_share = models.HSShare()
        hs_share.neighbour = secsum.clients.get(run_id=int(neighbour_id))
        hs_share.share = share
        hs_share.save()
        secsum.hs_shares.add(hs_share)

    client_id = client_message["id"]
    client = secsum.clients.get(run_id=client_id)

    secsum.a3_prime.add(client)

    return JsonResponse({}, status=200)


steps_by_id = {
    0: step0,
    1: step1,
    2: step21,
    3: step22,
    4: step3,
    5: step4,
    6: step5,
    7: step6,
    8: step7,
}


def secsum(request, name, step):

    if step not in steps_by_id:
        return HttpResponseServerError(f"Unknown secsum step: : '{step}'.")

    secsum = models.SecSum.objects.filter(name=name).first()

    if secsum is None:
        return HttpResponseServerError(f"Something went wrong.")

    if step != secsum.current_step:
        return HttpResponseServerError(f"Invalid step: '{step}'.")

    if secsum.error_state:
        return HttpResponseServerError(f"Something went wrong.")

    func_step = steps_by_id[step]

    if request.method != "POST":
        return HttpResponseServerError(f"Invalid request method: '{request.method}'.")

    response = func_step(request, secsum)

    statistic = secsum.statistics.get(phase=step)
    statistic.clients_at_step = F("clients_at_step") + 1
    statistic.save()

    return response


def ready(request, name):
    secsum = models.SecSum.objects.filter(name=name).first()
    status = "READY" if secsum != None and secsum.ready else "UNREADY"
    return JsonResponse({"status": status}, status=200)


def fetch_statistics(name):
    """
    Will return the number of clients per protocol phase as dictionary, since it is similar to json.
    :return: dict - number of clients per protocol phase as well as the result of the protocol calculation.
    """
    data = {0: 0, 1: 0, 2: 0, 3: 0, 4: 0, 5: 0, 6: 0, 7: 0, 8: 0}
    secsum = models.SecSum.objects.filter(name=name).first()
    if secsum is None:
        return {"stats": data, "result": []}
    for phase in range(0, 9):
        statistic = secsum.statistics.filter(phase=phase).first()
        if statistic is not None:
            data[phase] = statistic.clients_at_step
    result = secsum.final_vector
    if result is None:
        package = {"stats": data, "result": []}
    else:
        package = {"stats": data, "result": result}
    return package


def statistic_website(request, name):
    return render(request, "statistics.html", {"name": name})


def statistic_json(request, name):
    data_dict = fetch_statistics(name)
    return JsonResponse(data_dict, status=200)


def result(request, name):

    secsum = models.SecSum.objects.filter(name=name).first()

    if secsum is not None:
        if secsum.finished:
            return JsonResponse({"status": "finished", "result": secsum.final_vector}, status=200)
        else:
            return JsonResponse({"status": "ongoing", "result": []}, status=200)

    return JsonResponse({"status": "error", "result": []}, status=200)
