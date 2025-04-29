from django.db import models
from django.contrib.postgres.fields import ArrayField


class SecSum(models.Model):
    name = models.CharField(null=True)

    step_duration = models.IntegerField(null=True)
    num_clients = models.IntegerField(null=True)
    min_num_clients = models.IntegerField(null=True)
    vector_length = models.IntegerField(null=True)
    precision = models.IntegerField(null=True)
    gamma = models.FloatField(null=True)
    delta = models.FloatField(null=True)
    sigma = models.IntegerField(null=True)
    eta = models.IntegerField(null=True)
    current_step = models.IntegerField(null=True)
    error_state = models.BooleanField(default=False)

    num_neighbours = models.IntegerField(null=True)
    threshold = models.IntegerField(null=True)
    finished = models.BooleanField(default=False)

    final_vector = ArrayField(models.FloatField(), null=True)
    last_client_id = models.IntegerField(default=-1)
    ready = models.BooleanField(default=False)


class Client(models.Model):
    run_id = models.IntegerField(null=True)
    secsums = models.ManyToManyField("SecSum", related_name="clients")
    a1_prime = models.ForeignKey("SecSum", on_delete=models.CASCADE, null=True, related_name="a1_prime")
    a2_prime = models.ForeignKey("SecSum", on_delete=models.CASCADE, null=True, related_name="a2_prime")
    a3_prime = models.ForeignKey("SecSum", on_delete=models.CASCADE, null=True, related_name="a3_prime")
    neighbours = models.ManyToManyField("self")
    public_keys = models.OneToOneField("PublicKeys", on_delete=models.CASCADE, null=True)
    masked_input = models.CharField(null=True)


class PublicKeys(models.Model):
    key1 = models.CharField(null=True)
    key2 = models.CharField(null=True)


class CijMsg(models.Model):
    client = models.ForeignKey("Client", on_delete=models.CASCADE, null=True, related_name="cij_msgs")
    neighbour = models.ForeignKey("Client", on_delete=models.CASCADE, null=True, related_name="cij_neighbour")
    msg = models.CharField()


class HBShare(models.Model):
    secsum = models.ForeignKey("SecSum", on_delete=models.CASCADE, null=True, related_name="hb_shares")
    neighbour = models.ForeignKey("Client", on_delete=models.CASCADE, null=True, related_name="hb_clients")
    share = models.CharField()


class HSShare(models.Model):
    secsum = models.ForeignKey("SecSum", on_delete=models.CASCADE, null=True, related_name="hs_shares")
    neighbour = models.ForeignKey("Client", on_delete=models.CASCADE, null=True, related_name="hs_clients")
    share = models.CharField()


class Statistic(models.Model):
    clients_at_step = models.IntegerField(default=0)
    secsum = models.ForeignKey("SecSum", on_delete=models.CASCADE, null=True, related_name="statistics")
    phase = models.IntegerField(null=True)
