import json
from django.http import JsonResponse, HttpResponseServerError
from django.db.models import F
from django.db import transaction

from . import models


def step0(request, dummysum):

    with transaction.atomic():
        dummysum.last_client_id = F("last_client_id") + 1
        dummysum.save(update_fields=["last_client_id"])
        dummysum.refresh_from_db()

        client = models.Client()
        client.run_id = dummysum.last_client_id
        client.save()

    dummysum.clients.add(client)

    return JsonResponse({"id": client.run_id, "step_duration": dummysum.step_duration}, status=200)


def step1(request, dummysum):

    client_id = json.loads(request.body)["id"]
    client_vector = json.loads(request.body)["vector"]

    client = dummysum.clients.get(run_id=client_id)
    client.vector = client_vector
    client.save()

    return JsonResponse({}, status=200)


steps_by_id = {
    0: step0,
    1: step1,
}


def dummysum(request, name, step):

    dummysum = models.DummySum.objects.filter(name=name).first()

    if dummysum is None:
        return HttpResponseServerError(f"Something went wrong.")

    if dummysum.error_state:
        return HttpResponseServerError(f"Something went wrong.")

    if step != dummysum.current_step:
        return HttpResponseServerError(f"Invalid step: '{step}'.")

    func_step = steps_by_id[step]

    if request.method != "POST":
        return HttpResponseServerError(f"Invalid request method: '{request.method}'.")

    response = func_step(request, dummysum)

    return response


def ready(request, name):
    dummysum = models.DummySum.objects.filter(name=name).first()
    status = "READY" if dummysum != None and dummysum.ready else "UNREADY"
    return JsonResponse({"status": status}, status=200)


def result(request, name):
    dummysum = models.DummySum.objects.filter(name=name).first()
    if dummysum is not None:
        if dummysum.finished:
            return JsonResponse({"status": "finished", "result": dummysum.final_vector}, status=200)
        else:
            return JsonResponse({"status": "ongoing", "result": []}, status=200)
    return JsonResponse({"status": "error", "result": []}, status=200)
