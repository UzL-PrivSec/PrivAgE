from django.db import models
from django.contrib.postgres.fields import ArrayField


class DummySum(models.Model):
    name = models.CharField(null=True)
    step_duration = models.IntegerField(null=True)
    current_step = models.IntegerField(null=True)
    num_clients = models.IntegerField(null=True)
    vector_length = models.IntegerField(null=True)
    finished = models.BooleanField(default=False)
    final_vector = ArrayField(models.FloatField(), null=True)
    last_client_id = models.IntegerField(default=-1)
    error_state = models.BooleanField(default=False)
    ready = models.BooleanField(default=False)


class Client(models.Model):
    run_id = models.IntegerField(null=True)
    dummysums = models.ManyToManyField("DummySum", related_name="clients")
    vector = models.CharField(null=True)
