from django.http import JsonResponse
from apscheduler.schedulers.background import BackgroundScheduler

from .services.mult_secsum import do_secsums
from .services.mult_dummysums import do_dummysums


def start_secsums(request, num_clients):

    scheduler = BackgroundScheduler()
    job = scheduler.add_job(lambda: do_secsums(num_clients), "date", id="TEST")
    scheduler.start()

    return JsonResponse({"Status": "STARTED"}, status=200)


def start_dummysums(request, num_clients):

    scheduler = BackgroundScheduler()
    job = scheduler.add_job(lambda: do_dummysums(num_clients), "date", id="TEST")
    scheduler.start()

    return JsonResponse({"Status": "STARTED"}, status=200)
